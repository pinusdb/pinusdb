/*
* Copyright (c) 2019 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; If not, see <http://www.gnu.org/licenses>
*/

#include "internal.h"
#include "server/server_pdb.h"
#include "server/event_handle.h"
#include "util/log_util.h"
#include "expr/tokenize.h"
#include "db/page_pool.h"
#include "db/db_impl.h"
#include "util/date_time.h"
#include "global_variable.h"
#include "util/dbg.h"
#include "port/env.h"

#ifndef _WIN32

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif

ServerPDB::ServerPDB()
{
  this->serIP_ = "0.0.0.0";
  this->serPort_ = 8105;

#ifdef _WIN32

  this->iocpHandle_ = INVALID_HANDLE_VALUE;
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

#else
  epollFd_ = -1;
  licenseFd_ = -1;
  for (int i = 0; i < kMaxWorkThreadCnt; i++)
  {
    workThreads_[i] = nullptr;
  }

#endif

}

ServerPDB::~ServerPDB()
{
#ifdef _WIN32
  if (pContextPool_ != nullptr)
    delete pContextPool_;
#else

#endif
}

bool ServerPDB::Start()
{
  try {

    //初始化SQL语句分词
    Tokenize::InitTokenize();
    DateTime::InitTimeZone();

    if (!pGlbSysCfg->LoadConfig())
      return false;

    if (!InitLog())
      return false;

    if (!pGlbPagePool->InitPool())
      return false;

    if (!InitUser())
      return false;

    if (!InitTable())
      return false;

    if (!InitCommitLog())
      return false;

    PdbErr_t retVal = PdbE_OK;

    DBImpl* pDB = DBImpl::GetInstance();
    retVal = pDB->Start();
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("start database instance error {}", retVal);
      return false;
    }

#ifdef _WIN32
    if (!StartIocp(pGlbSysCfg->GetAddress().c_str(), pGlbSysCfg->GetPort()))
    {
      LOG_ERROR("failed to start IOCP");
      return false;
    }
#else
    if (!StartEpoll(pGlbSysCfg->GetAddress().c_str(), pGlbSysCfg->GetPort()))
    {
      LOG_ERROR("failed to start epoll");
      return false;
    }
#endif
  }
  catch (std::exception ex)
  {
    return false;
  }

  return true;
}

void ServerPDB::Stop()
{
  LOG_INFO("stoping database service ...");
  DBImpl* pDB = DBImpl::GetInstance();

#ifdef _WIN32
  StopIocp();
#else
  StopEpoll();
#endif

  LOG_INFO("beging flush page cache to disk ...");
  pDB->Stop();
  LOG_INFO("flush page cache to disk finished");
  LOG_INFO("service was stopped gracefully");
}

EventHandle* ServerPDB::NewEvent(int socket, const char* pRemoteIp, int remotePort)
{
  EventHandle* pNewEvent = new EventHandle(socket, pRemoteIp, remotePort);
  if (pNewEvent == nullptr)
    return nullptr;

#ifndef _WIN32
  pNewEvent->AddRef();
#endif

  if (!pGlbServerConnction->AddConnection((uint64_t)pNewEvent, pRemoteIp, remotePort))
  {
    delete pNewEvent;
    return nullptr;
  }

  LOG_INFO("create connection, remote:({}:{})", pRemoteIp, remotePort);
  return pNewEvent;
}

void ServerPDB::RemoveEvent(EventHandle* pEvent)
{
  pGlbServerConnction->DelConnection((uint64_t)pEvent);
#ifndef _WIN32
  if (pEvent->MinusRef())
  {
    delete pEvent;
  }
#endif
}

bool ServerPDB::InitLog()
{
  Env* pEnv = Env::Default();
  std::string logPath = pGlbSysCfg->GetSysLogPath();
  if (!pEnv->PathExists(logPath.c_str()))
  {
    if (pEnv->CreateDir(logPath.c_str()) != PdbE_OK)
      return false;
  }

  int logLevel = pGlbSysCfg->GetLogLevel();
  size_t logSize = PDB_MB_BYTES(5);
  int maxFile = 100;
  std::string filePath = logPath + "/pdb.log";

  std::string logName = "pdb";
  spd::rotating_logger_mt(logName, filePath, logSize, maxFile);
  spd::get(logName)->set_level(static_cast<spdlog::level::level_enum>(logLevel));

  LOG_INFO("init system log successful");
  return true;
}

bool ServerPDB::InitUser()
{
  PdbErr_t retVal = pGlbUser->Load();
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("failed to init user info, ret:{}", retVal);
    return false;
  }

  return true;
}

bool ServerPDB::InitTable()
{
  if (!pGlbTabCfg->LoadTableConfig())
  {
    LOG_ERROR("failed to load table config");
    return false;
  }

  return true;
}

bool ServerPDB::InitCommitLog()
{
  std::string commitLogPath = pGlbSysCfg->GetCommitLogPath();
  if (pGlbCommitLog->Init(commitLogPath.c_str()) != PdbE_OK)
  {
    LOG_ERROR("failed to load datalog");
    return false;
  }

  return true;
}

#ifdef _WIN32

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

DWORD WINAPI ServerPDB::WorkerThread(LPVOID lpParam)
{
  PDB_SEH_BEGIN(false);
  return WorkerMain(lpParam);
  PDB_SEH_END("workthread", return 0);
}

DWORD ServerPDB::WorkerMain(LPVOID lpParam)
{
  ServerPDB* pServer = (ServerPDB*)lpParam;

  OVERLAPPED* pOverlapped = NULL;
  DWORD       dwBytesTransfered = 0;
  ULONG_PTR   completionKey = 0;

  while (glbRunning)
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
          case EventHandle::EventState::kRecv:
          {
            if (!pServer->PostRecv(pContext))
            {
              pServer->FreeSocketContext(pContext);
            }
            break;
          }
          case EventHandle::EventState::kExec:
          {
            //这里执行完成后直接发送
            pContext->pEventHandle_->ExecTask();
          }
          case EventHandle::EventState::kSend:
          {
            if (!pServer->PostSend(pContext))
            {
              pServer->FreeSocketContext(pContext);
            }
            break;
          }
          case EventHandle::EventState::kEnd:
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

bool ServerPDB::LoadSocketLib()
{
  WSADATA wsaData;
  return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void ServerPDB::UnloadSocketLib()
{
  WSACleanup();
}

bool ServerPDB::InitIOCP()
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
    workThreads_[i] = ::CreateThread(0, 0, ServerPDB::WorkerThread, (void*)this, 0, &threadId);
  }

  return true;
}

bool ServerPDB::InitListenSocket()
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

SOCKET_CONTEXT* ServerPDB::DoAccept(SOCKET_CONTEXT* pContext, size_t bytesTransfered)
{
  SOCKET_CONTEXT* pNewClient = nullptr;

  if (bytesTransfered > 0)
    pNewClient = DoFirstRecvWithData(pContext, bytesTransfered);
  else
    pNewClient = DoFirstRecvWithoutData(pContext);

  if (pNewClient == nullptr)
  {
    shutdown(pContext->socket_, SD_BOTH);
    closesocket(pContext->socket_);
    LOG_ERROR("failed to create connection object");
  }

  this->PostAccept(pContext);
  return pNewClient;
}

SOCKET_CONTEXT* ServerPDB::DoFirstRecvWithData(SOCKET_CONTEXT* pContext, size_t bytesTransfered)
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
    LOG_ERROR("MallocSocketContext failed in ServerPDB::DoFirstRecvWithData");
    return nullptr;
  }

  pNewContext->socket_ = pContext->socket_;

  pNewContext->pEventHandle_ = NewEvent(
    0, inet_ntoa(remoteAddr->sin_addr), remoteAddr->sin_port);

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

SOCKET_CONTEXT* ServerPDB::DoFirstRecvWithoutData(SOCKET_CONTEXT* pContext)
{
  SOCKADDR_IN remoteAddr;
  int len = sizeof(SOCKADDR_IN);

  getpeername(pContext->socket_, (sockaddr*)&remoteAddr, &len);

  SOCKET_CONTEXT* pNewContext = MallocSocketContext();
  if (pNewContext == nullptr)
  {
    LOG_ERROR("MallocSocketContext failed in ServerPDB::DoFirstRecvWithoutData");
    return pNewContext;
  }

  pNewContext->socket_ = pContext->socket_;

  pNewContext->pEventHandle_ = NewEvent(
    0, inet_ntoa(remoteAddr.sin_addr), remoteAddr.sin_port);

  if (CreateIoCompletionPort((HANDLE)pNewContext->socket_,
    iocpHandle_, (ULONG_PTR)pNewContext, 0) == NULL)
  {
    LOG_ERROR("failed to CreateIoCompletionPort, err: {}", GetLastError());
    FreeSocketContext(pNewContext);
    return nullptr;
  }

  return pNewContext;
}

bool ServerPDB::PostAccept(SOCKET_CONTEXT* pContext)
{
  DWORD dwBytes = 0;

  pContext->socket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
  pContext->opType_ = OPERATION_TYPE::ACCEPT_POSTED;

  if (pContext->socket_ == INVALID_SOCKET)
  {
    LOG_ERROR("failed to WSASocket in ServerPDB::PostAccept, err: {}", GetLastError());
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
    LOG_ERROR("failed to AcceptEx in ServerPDB::PostAccept, err: {}", lastError);
    return false;
  }

  return true;
}

bool ServerPDB::PostRecv(SOCKET_CONTEXT* pContext)
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
      LOG_ERROR("failed to WSARecv, ret: {}", lastError);
      return false;
    }

    return true;
  }

  LOG_ERROR("failed to GetRecvBuf in ServerPDB::PostRecv");
  return false;
}

bool ServerPDB::PostSend(SOCKET_CONTEXT* pContext)
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
  LOG_ERROR("failed to GetSendBuf in ServerPDB::PostSend");
  return false;
}

bool ServerPDB::StartIocp(const char* pSerIp, int serPort)
{
  if (strlen(pSerIp) > 0)
    this->serIP_ = pSerIp;

  if (serPort > 0 && serPort < 65536)
    this->serPort_ = serPort;

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

void ServerPDB::StopIocp()
{
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

int ServerPDB::GetProcessorsCnt()
{
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  return sysInfo.dwNumberOfProcessors;
}

SOCKET_CONTEXT* ServerPDB::MallocSocketContext()
{
  std::unique_lock<std::mutex> contextLock(iocpMutex_);
  uint8_t* pNewContext = pContextPool_->MallocObject();
  if (pNewContext != nullptr)
  {
    InitSocketContext((SOCKET_CONTEXT*)pNewContext);
  }
  return (SOCKET_CONTEXT*)pNewContext;
}

void ServerPDB::FreeSocketContext(SOCKET_CONTEXT* pContext)
{
  RemoveEvent(pContext->pEventHandle_);
  ResetSocketContext(pContext);
  std::unique_lock<std::mutex> contextLock(iocpMutex_);
  pContextPool_->FreeObject((uint8_t*)pContext);
}

#else

#define MAX_EVENTS     32

void SetSocket(int sockfd)
{
  int opts = fcntl(sockfd, F_GETFL);
  if (opts < 0)
  {
    return;
  }

  //nonblock
  opts = (opts | O_NONBLOCK);
  fcntl(sockfd, F_SETFL, opts);

  int optVal = 1;
  socklen_t optlen = sizeof(optVal);
  setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optVal, optlen);
}

void SetSocketReuseAddr(int sockfd)
{
  int optVal = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
}

bool ServerPDB::StartEpoll(const char* pSerIp, int serPort)
{
  struct epoll_event ev, events[MAX_EVENTS];
  struct sockaddr_in local, remote;
  socklen_t addrlen = sizeof(remote);

  this->serIP_ = pSerIp;
  this->serPort_ = serPort;

  if ((licenseFd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    return false;
  }

  SetSocket(licenseFd_);
  SetSocketReuseAddr(licenseFd_);
  memset(&local, 0, sizeof(local));

  local.sin_family = AF_INET;
  local.sin_addr.s_addr = inet_addr(this->serIP_.c_str());
  local.sin_port = htons(serPort_);
  if (bind(licenseFd_, (struct sockaddr*)&local, sizeof(local)) < 0)
  {
    return false;
  }

  listen(licenseFd_, MAX_EVENTS);

  epollFd_ = epoll_create(MAX_EVENTS);
  if (epollFd_ == -1)
    return false;

  ev.events = EPOLLIN;
  ev.data.fd = licenseFd_;
  if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, licenseFd_, &ev) == -1)
  {
    return false;
  }

  for (int i = 0; i < kMaxWorkThreadCnt; i++)
  {
    workThreads_[i] = new std::thread(&ServerPDB::WorkerThread, this);
  }
  
  while (glbRunning)
  {
    int nfds = epoll_wait(epollFd_, events, MAX_EVENTS, 1000);
    if (nfds == -1 && errno != EINTR) {
      LOG_DEBUG("epoll_wait failed, errno: {}", errno);
      return false;
    }

    for (int i = 0; i < nfds; i++)
    {
      int fd = events[i].data.fd;
      if (fd == licenseFd_)
      {
        addrlen = sizeof(remote);
        int newFd = -1;
        while ((newFd = accept(licenseFd_, (struct sockaddr *)&remote, &addrlen)) > 0)
        {
          SetSocket(newFd);
          ev.events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR | EPOLLRDHUP | EPOLLET;

          EventHandle* pEvent = NewEvent(newFd,
            inet_ntoa(remote.sin_addr), remote.sin_port);

          ev.data.ptr = pEvent;

          if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, newFd, &ev) < 0)
          {
            LOG_DEBUG("epoll_ctl add failed, errno: {}", errno);
            return false;
          }
        }

        if (newFd == -1)
        {
          //LOG_INFO("accept failed, errno: {}", errno);
          if (errno != EAGAIN && errno != ECONNABORTED
            && errno != EPROTO && errno != EINTR && errno != EMFILE)
          {
            LOG_INFO("stop service, {}", errno);
            return false;
          }
        }

        continue;
      }
      else if ((events[i].events & EPOLLERR) ||
        (events[i].events & EPOLLHUP) || (events[i].events & EPOLLRDHUP))
      {
        EventHandle* pEvent = (EventHandle*)events[i].data.ptr;

        epoll_ctl(epollFd_, EPOLL_CTL_DEL, pEvent->GetSocket(), nullptr);
        RemoveEvent(pEvent);
        LOG_DEBUG("connection break, handle:{}, ev:{}", reinterpret_cast<uintptr_t>(pEvent), events[i].events);
        continue;
      }
      else if (events[i].events & EPOLLIN)
      {
        EventHandle* pEvent = (EventHandle*)events[i].data.ptr;
        if (!pEvent->RecvData())
        {
          epoll_ctl(epollFd_, EPOLL_CTL_DEL, pEvent->GetSocket(), nullptr);
          RemoveEvent(pEvent);
          LOG_DEBUG("recv data error, close connection");
        }
        else if (pEvent->GetState() == EventHandle::EventState::kExec)
        {
          pEvent->AddRef();
          do {
            std::unique_lock<std::mutex> taskLock(taskMutex_);
            taskList_.push_back(pEvent);
          } while (false);
          taskVariable_.notify_one();
        }
      }
      else if (events[i].events & EPOLLOUT)
      {
        EventHandle* pEvent = (EventHandle*)events[i].data.ptr;
        if (!pEvent->SendData())
        {
          epoll_ctl(epollFd_, EPOLL_CTL_DEL, pEvent->GetSocket(), nullptr);
          RemoveEvent(pEvent);
          LOG_DEBUG("send response data error, close connection");
        }
      }
      else
      {
        EventHandle* pEvent = (EventHandle*)events[i].data.ptr;
        epoll_ctl(epollFd_, EPOLL_CTL_DEL, pEvent->GetSocket(), nullptr);
        RemoveEvent(pEvent);
        LOG_DEBUG("close connection");
      }
    }
  }

  LOG_DEBUG("epoll loop end");
  return true;
}

void ServerPDB::StopEpoll()
{
  this->taskVariable_.notify_all();
  
  for (int i = 0; i < kMaxWorkThreadCnt; i++)
  {
    if (workThreads_[i] != nullptr)
      workThreads_[i]->join();

    workThreads_[i] = nullptr;
  }
}

void ServerPDB::WorkerThread()
{
  while (glbRunning)
  {
    EventHandle* pEvent = nullptr;

    do {
      std::unique_lock<std::mutex> taskLock(taskMutex_);
      if (taskList_.size() > 0)
      {
        auto firstIt = taskList_.begin();
        pEvent = *firstIt;
        taskList_.erase(firstIt);
      }
      else
      {
        taskVariable_.wait(taskLock);
        if (taskList_.size() > 0)
        {
          auto firstIt = taskList_.begin();
          pEvent = *firstIt;
          taskList_.erase(firstIt);
        }
      }
    } while (false);

    if (glbRunning && pEvent != nullptr)
    {
      pEvent->ExecTask();
      if (pEvent->MinusRef())
      {
        delete pEvent;
      }
    }
  }
}

#endif
