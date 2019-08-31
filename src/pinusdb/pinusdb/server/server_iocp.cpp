#include "server/server_iocp.h"
#include "util/object_pool.h"
#include "server/iocp_event.h"
#include "util/log_util.h"
#include "util/dbg.h"

#define COMPLETION_KEY_QUIT        0x00

inline void InitSocketContext(SOCKET_CONTEXT* pContext)
{
  pContext->socket_ = INVALID_SOCKET;
  memset(&(pContext->overlapped_), 0, sizeof(OVERLAPPED));
  pContext->opType_ = OPERATION_TYPE::NULL_POSTED;
  pContext->wsaBuf_.buf = pContext->dataBuf_;
  pContext->wsaBuf_.len = SOCKET_CONTEXT_BUF_SIZE;
  pContext->pEventHandle_ = nullptr;
}

inline void ResetSocketContext(SOCKET_CONTEXT* pContext)
{
  if (pContext->socket_ != INVALID_SOCKET)
    closesocket(pContext->socket_);

  if (pContext->pEventHandle_ != nullptr)
    delete pContext->pEventHandle_;

  pContext->socket_ = INVALID_SOCKET;
  memset(&(pContext->overlapped_), 0, sizeof(OVERLAPPED));
  pContext->opType_ = OPERATION_TYPE::NULL_POSTED;
  pContext->wsaBuf_.buf = pContext->dataBuf_;
  pContext->wsaBuf_.len = SOCKET_CONTEXT_BUF_SIZE;
  pContext->pEventHandle_ = nullptr;
}

inline void ResetSocketContextBuf(SOCKET_CONTEXT* pContext)
{
  memset(&(pContext->overlapped_), 0, sizeof(OVERLAPPED));
  pContext->wsaBuf_.buf = pContext->dataBuf_;
  pContext->wsaBuf_.len = SOCKET_CONTEXT_BUF_SIZE;
}

DWORD WINAPI ServerIOCP::WorkerThread(LPVOID lpParam)
{
  PDB_SEH_BEGIN(false);
  return WorkerMain(lpParam);
  PDB_SEH_END("workthread", return 0);
}

DWORD ServerIOCP::WorkerMain(LPVOID lpParam)
{
  ServerIOCP* pServer = (ServerIOCP*)lpParam;

  OVERLAPPED* pOverlapped = NULL;
  DWORD       dwBytesTransfered = 0;
  ULONG_PTR   completionKey = 0;


  while (pServer->running_)
  {
    BOOL bRet = GetQueuedCompletionStatus(
      pServer->iocpHandle_,
      &dwBytesTransfered,
      &completionKey,
      &pOverlapped,
      INFINITE);

    if (!bRet)
    {
      int lastError = GetLastError();
      LOG_WARNING("failed to GetQueuedCompletionStatus, err: {}", lastError);
      if (lastError == ERROR_NETNAME_DELETED)
      {
        //客户端异常关闭
        SOCKET_CONTEXT* pContext = CONTAINING_RECORD(pOverlapped, SOCKET_CONTEXT, overlapped_);
        pServer->FreeSocketContext(pContext);
      }
      continue;
    }
    else
    {
      if (OPERATION_TYPE::NULL_POSTED == completionKey) //退出
        break;

      SOCKET_CONTEXT* pContext = CONTAINING_RECORD(pOverlapped, SOCKET_CONTEXT, overlapped_);

      if (0 == dwBytesTransfered
        && (pContext->opType_ == OPERATION_TYPE::RECV_POSTED
          || pContext->opType_ == OPERATION_TYPE::SEND_POSTED))
      {
        //停止任务，断开连接
        pServer->FreeSocketContext(pContext);
      }
      else
      {
        switch (pContext->opType_)
        {
        case OPERATION_TYPE::ACCEPT_POSTED:
          //返回值为新创建的连接对象旧的pEvent对象继续监听
          pContext = pServer->DoAccept(pContext, dwBytesTransfered);
          break;
        case OPERATION_TYPE::RECV_POSTED:
          pContext->pEventHandle_->RecvPostedEvent(
            (const uint8_t*)pContext->wsaBuf_.buf, dwBytesTransfered);
          break;
        case OPERATION_TYPE::SEND_POSTED:
          pContext->pEventHandle_->SendPostedEvent(dwBytesTransfered);
          break;
        default:
          break;
        }

        //有可能执行 DoAccept 时由于连接太多导致返回的pContext为nullptr
        if (pContext != nullptr)
        {
          switch (pContext->pEventHandle_->GetState())
          {
          case IOCPEvent::EventState::kRecv:
          {
            if (!pServer->PostRecv(pContext))
            {
              pServer->FreeSocketContext(pContext);
            }
            break;
          }
          case IOCPEvent::EventState::kExec:
          {
            //这里执行完成后直接发送
            pContext->pEventHandle_->ExecTask();
          }
          case IOCPEvent::EventState::kSend:
          {
            if (!pServer->PostSend(pContext))
            {
              pServer->FreeSocketContext(pContext);
            }
            break;
          }
          case IOCPEvent::EventState::kEnd:
          {
            pServer->FreeSocketContext(pContext);
            break;
          }
          }
        }
      }

    }
  }


  return 0;

}

///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////

ServerIOCP::ServerIOCP()
{
  this->running_ = true;
  this->iocpHandle_ = INVALID_HANDLE_VALUE;

  this->serIP_ = "0.0.0.0";
  this->serPort_ = 8105;

  this->pListenContext_ = nullptr;

  this->lpfnAcceptEx_ = nullptr;
  this->lpfnGetAcceptExSockAddrs_ = nullptr;

  this->workThreadCnt_ = kMaxWorkThreadCnt;
  for (int i = 0; i < kMaxWorkThreadCnt; i++)
  {
    workThreads_[i] = INVALID_HANDLE_VALUE;
  }

  pContextPool_ = new ObjectPool(sizeof(SOCKET_CONTEXT), 
    (kSocketListenCnt + kSocketAcceptCnt + kSocketWorkerCnt), kContextCntPerBlk);

}
ServerIOCP::~ServerIOCP()
{
  if (pContextPool_ != nullptr)
    delete pContextPool_;
}

bool ServerIOCP::LoadSocketLib()
{
  WSADATA wsaData;
  return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void ServerIOCP::UnloadSocketLib()
{
  WSACleanup();
}

bool ServerIOCP::InitIOCP()
{
  //获取CPU的核数
  int processorCnt = GetProcessorsCnt();

  iocpHandle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 
    (processorCnt * kConcurrentPerProcessor));

  if (NULL == iocpHandle_)
    return false;

  this->workThreadCnt_ = kMaxWorkThreadCnt;
  if ((processorCnt * kThreadsPerProcessor) < kMaxWorkThreadCnt)
  {
    this->workThreadCnt_ = (processorCnt * kThreadsPerProcessor);
  }

  DWORD threadId = 0;
  for (int i = 0; i < workThreadCnt_; i++)
  {
    workThreads_[i] = ::CreateThread(0, 0, ServerIOCP::WorkerThread, (void*)this, 0, &threadId);
  }

  return true;
}

bool ServerIOCP::InitListenSocket()
{
  GUID GuidAcceptEx = WSAID_ACCEPTEX;
  GUID GuidGetAcceptExSockAddr = WSAID_GETACCEPTEXSOCKADDRS;

  struct sockaddr_in serAddress;

  if (pListenContext_ != nullptr)
  {
    free(pListenContext_);
  }

  SOCKET lisSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
  if (INVALID_SOCKET == lisSock)
  {
    LOG_ERROR("failed to WSASocket, err: {}", GetLastError());
    return false;
  }

  pListenContext_ = MallocSocketContext();
  pListenContext_->socket_ = lisSock;

  if (NULL == CreateIoCompletionPort((HANDLE)lisSock,
    iocpHandle_, (ULONG_PTR)pListenContext_, 0))
  {
    LOG_ERROR("failed to CreateIoCompletionPort, err: {}", GetLastError());
    return false;
  }

  memset(&serAddress, 0, sizeof(serAddress));
  serAddress.sin_family = AF_INET;
  serAddress.sin_addr.s_addr = inet_addr(serIP_.c_str());
  serAddress.sin_port = htons((u_short)serPort_);

  if (SOCKET_ERROR == bind(lisSock, (struct sockaddr*)&serAddress, sizeof(serAddress)))
  {
    LOG_ERROR("failed to bind, err: {}", GetLastError());
    return false;
  }

  if (SOCKET_ERROR == listen(lisSock, SOMAXCONN))
  {
    LOG_ERROR("failed to listen, err: {}", GetLastError());
    return false;
  }

  DWORD dwBytes = 0;
  if (SOCKET_ERROR == WSAIoctl(
    lisSock,
    SIO_GET_EXTENSION_FUNCTION_POINTER,
    &GuidAcceptEx,
    sizeof(GuidAcceptEx),
    &lpfnAcceptEx_,
    sizeof(lpfnAcceptEx_),
    &dwBytes,
    NULL,
    NULL))
  {
    LOG_ERROR("failed to Get AcceptEx pointer, err: {}", GetLastError());
    return false;
  }

  if (SOCKET_ERROR == WSAIoctl(
    lisSock,
    SIO_GET_EXTENSION_FUNCTION_POINTER,
    &GuidGetAcceptExSockAddr,
    sizeof(GuidGetAcceptExSockAddr),
    &lpfnGetAcceptExSockAddrs_,
    sizeof(lpfnGetAcceptExSockAddrs_),
    &dwBytes,
    NULL,
    NULL))
  {
    LOG_ERROR("failed to get GetAcceptExSockAddr pointer:({})", GetLastError());
    return false;
  }

  for (int i = 0; i < kSocketAcceptCnt; i++)
  {
    SOCKET_CONTEXT* pTmpContext = MallocSocketContext();
    if (pTmpContext == nullptr)
    {
      LOG_ERROR("failed to MallocSocketContext");
      return false;
    }

    if (!this->PostAccept(pTmpContext))
    {
      //FreeSocketContext(pTmpContext);
      LOG_ERROR("failed to PostAccept");
      return false;
    }
  }

  return true;
}

SOCKET_CONTEXT* ServerIOCP::DoAccept(SOCKET_CONTEXT* pAcceptEvent, size_t bytesTransfered)
{
  SOCKET_CONTEXT* pNewClient = nullptr;

  if (bytesTransfered > 0)
    pNewClient = DoFirstRecvWithData(pAcceptEvent, bytesTransfered);
  else
    pNewClient = DoFirstRecvWithoutData(pAcceptEvent);

  this->PostAccept(pAcceptEvent);

  if (pNewClient == nullptr)
  {
    LOG_ERROR("failed to create connection object");
  }

  return pNewClient;
}

SOCKET_CONTEXT* ServerIOCP::DoFirstRecvWithData(SOCKET_CONTEXT* pContext, size_t bytesTransfered)
{
  SOCKADDR_IN* remoteAddr = NULL;
  SOCKADDR_IN* localAddr = NULL;

  int remoteLen = sizeof(SOCKADDR_IN);
  int localLen = sizeof(SOCKADDR_IN);

  this->lpfnGetAcceptExSockAddrs_(pContext->wsaBuf_.buf,
    (pContext->wsaBuf_.len - ((sizeof(SOCKADDR_IN) + 16) * 2)),
    (sizeof(SOCKADDR_IN) + 16), (sizeof(SOCKADDR_IN) + 16),
    (LPSOCKADDR*)&localAddr, &localLen,
    (LPSOCKADDR*)&remoteAddr, &remoteLen);

  SOCKET_CONTEXT* pNewContext = MallocSocketContext();
  if (pNewContext == nullptr)
  {
    LOG_ERROR("MallocSocketContext failed in ServerIOCP::DoFirstRecvWithData");
    return nullptr;
  }

  pNewContext->socket_ = pContext->socket_;

  pNewContext->pEventHandle_ = NewEvent(
    inet_ntoa(remoteAddr->sin_addr), remoteAddr->sin_port);

  if (CreateIoCompletionPort((HANDLE)pNewContext->socket_,
    iocpHandle_, (ULONG_PTR)pNewContext, 0) == NULL)
  {
    LOG_ERROR("failed to CreateIoCompletionPort, err: {}", GetLastError());
    FreeSocketContext(pNewContext);
    return nullptr;
  }

  pNewContext->pEventHandle_->RecvPostedEvent((const uint8_t*)pContext->wsaBuf_.buf, bytesTransfered);

  return pNewContext;
}

SOCKET_CONTEXT* ServerIOCP::DoFirstRecvWithoutData(SOCKET_CONTEXT* pContext)
{
  SOCKADDR_IN remoteAddr;
  int len = sizeof(SOCKADDR_IN);

  getpeername(pContext->socket_, (sockaddr*)&remoteAddr, &len);

  SOCKET_CONTEXT* pNewContext = MallocSocketContext();
  if (pNewContext == nullptr)
  {
    LOG_ERROR("MallocSocketContext failed in ServerIOCP::DoFirstRecvWithoutData");
    return pNewContext;
  }

  pNewContext->socket_ = pContext->socket_;

  pNewContext->pEventHandle_ = NewEvent(
    inet_ntoa(remoteAddr.sin_addr), remoteAddr.sin_port);

  if (CreateIoCompletionPort((HANDLE)pNewContext->socket_,
    iocpHandle_, (ULONG_PTR)pNewContext, 0) == NULL)
  {
    LOG_ERROR("failed to CreateIoCompletionPort, err: {}", GetLastError());
    FreeSocketContext(pNewContext);
    return nullptr;
  }

  return pNewContext;
}


bool ServerIOCP::PostAccept(SOCKET_CONTEXT* pContext)
{
  DWORD dwBytes = 0;

  pContext->socket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
  pContext->opType_ = OPERATION_TYPE::ACCEPT_POSTED;

  if (pContext->socket_ == INVALID_SOCKET)
  {
    LOG_ERROR("failed to WSASocket in ServerIOCP::PostAccept, err: {}", GetLastError());
    return false;
  }

  int on = 1;
  setsockopt(pContext->socket_, SOL_SOCKET, SO_KEEPALIVE, (const char*)&on, sizeof(on));
  setsockopt(pContext->socket_, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on));

  BOOL ret = lpfnAcceptEx_(pListenContext_->socket_, pContext->socket_,
    pContext->wsaBuf_.buf, (pContext->wsaBuf_.len - ((sizeof(SOCKADDR_IN) + 16) * 2)),
    (sizeof(SOCKADDR_IN) + 16), (sizeof(SOCKADDR_IN) + 16), &dwBytes, &(pContext->overlapped_));

  int lastError = GetLastError();
  if (!ret && lastError != WSA_IO_PENDING)
  {
    LOG_ERROR("failed to AcceptEx in ServerIOCP::PostAccept, err: {}", lastError);
    return false;
  }

  return true;
}


bool ServerIOCP::PostRecv(SOCKET_CONTEXT* pContext)
{
  DWORD dwFlags = 0;
  DWORD dwSendBytes = 0;

  ResetSocketContextBuf(pContext);

  if (pContext->pEventHandle_->GetRecvBuf(&(pContext->wsaBuf_)))
  {
    pContext->opType_ = OPERATION_TYPE::RECV_POSTED;

    int ret = WSARecv(pContext->socket_, &(pContext->wsaBuf_), 1,
      &dwSendBytes, &dwFlags, &(pContext->overlapped_), NULL);

    int lastError = WSAGetLastError();
    if ((SOCKET_ERROR == ret) && (WSA_IO_PENDING != lastError))
    {
      //pContext->pEventHandle_->SetEnd();
      LOG_ERROR("failed to WSARecv, ret: {}", lastError);
      return false;
    }

    return true;
  }

  //pContext->pEventHandle_->SetEnd();
  LOG_ERROR("failed to GetRecvBuf in ServerIOCP::PostRecv");
  return false;
}

bool ServerIOCP::PostSend(SOCKET_CONTEXT* pContext)
{
  DWORD dwFlags = 0;
  DWORD dwSendBytes = 0;

  ResetSocketContextBuf(pContext);

  if (pContext->pEventHandle_->GetSendBuf(&(pContext->wsaBuf_)))
  {
    pContext->opType_ = OPERATION_TYPE::SEND_POSTED;

    int ret = WSASend(pContext->socket_, &(pContext->wsaBuf_), 1,
      &dwSendBytes, dwFlags, &(pContext->overlapped_), NULL);

    int lastError = WSAGetLastError();
    if ((SOCKET_ERROR == ret) && (WSA_IO_PENDING != lastError))
    {
      pContext->pEventHandle_->SetEnd();
      LOG_ERROR("failed to WSASend, ret: lastError");
      return false;
    }

    return true;
  }

  pContext->pEventHandle_->SetEnd();
  LOG_ERROR("failed to GetSendBuf in ServerIOCP::PostSend");
  return false;
}

bool ServerIOCP::StartIocp(const char* pSerIp, int serPort)
{
  if (strlen(pSerIp) > 0)
    this->serIP_ = pSerIp;

  if (serPort > 0 && serPort < 65536)
    this->serPort_ = serPort;

  this->running_ = true;

  if (!InitIOCP())
  {
    LOG_ERROR("failed to InitIOCP");
    return false;
  }

  if (!InitListenSocket())
  {
    LOG_ERROR("failed to InitListenSocket");
    return false;
  }

  return true;
}

void ServerIOCP::StopIocp()
{
  this->running_ = false;

  if (pListenContext_ != nullptr)
  {
    for (int i = 0; i < workThreadCnt_; i++)
    {
      PostQueuedCompletionStatus(iocpHandle_, 0,
        (DWORD)OPERATION_TYPE::NULL_POSTED, NULL);
    }

    WaitForMultipleObjects(workThreadCnt_, workThreads_, TRUE, 5000);
  }

}

int ServerIOCP::GetProcessorsCnt()
{
  SYSTEM_INFO sysInfo;

  GetSystemInfo(&sysInfo);

  return sysInfo.dwNumberOfProcessors;
}


SOCKET_CONTEXT* ServerIOCP::MallocSocketContext()
{
  std::unique_lock<std::mutex> contextLock(iocpMutex_);
  uint8_t* pNewContext = pContextPool_->MallocObject();
  if (pNewContext != nullptr)
  {
    InitSocketContext((SOCKET_CONTEXT*)pNewContext);
  }
  return (SOCKET_CONTEXT*)pNewContext;
}

void ServerIOCP::FreeSocketContext(SOCKET_CONTEXT* pContext)
{
  RemoveEvent(pContext->pEventHandle_);
  ResetSocketContext(pContext);
  std::unique_lock<std::mutex> contextLock(iocpMutex_);
  pContextPool_->FreeObject((uint8_t*)pContext);
}


