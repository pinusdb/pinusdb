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

#pragma once

#include "internal.h"
#include "util/object_pool.h"
#include <thread>
#include <atomic>
#include <condition_variable>

class EventHandle;

#ifdef _WIN32

#define SOCKET_CONTEXT_BUF_SIZE   (8 * 1024)

typedef enum _OPERATION_TYPE
{
  ACCEPT_POSTED = 0,      // 投递Accept操作
  RECV_POSTED = 1,        // 投递接收操作
  SEND_POSTED = 2,        // 投递发送操作
  NULL_POSTED = 3,        // 用于初始化
}OPERATION_TYPE;

typedef struct _SOCKET_CONTEXT
{
  SOCKET         socket_;         //套接字
  OVERLAPPED     overlapped_;     //重叠操作对象
  OPERATION_TYPE opType_;         //操作类型
  WSABUF         wsaBuf_;         //缓存对象
  EventHandle*   pEventHandle_;   //事件对象
  char           dataBuf_[SOCKET_CONTEXT_BUF_SIZE]; //接收缓冲区
}SOCKET_CONTEXT;

#endif

class ServerPDB
{
public:
  ServerPDB();
  ~ServerPDB();

public:
  bool Start();
  void Stop();

#ifdef _WIN32

public:
  bool LoadSocketLib();
  void UnloadSocketLib();

private:
  bool InitIOCP();
  bool InitListenSocket();
  SOCKET_CONTEXT* DoAccept(SOCKET_CONTEXT* pContext, size_t bytesTransfered);
  SOCKET_CONTEXT* DoFirstRecvWithData(SOCKET_CONTEXT* pContext, size_t bytesTransfered);

  SOCKET_CONTEXT* DoFirstRecvWithoutData(SOCKET_CONTEXT* pContext);

  bool PostAccept(SOCKET_CONTEXT* pContext);

  bool PostRecv(SOCKET_CONTEXT* pContext);
  bool PostSend(SOCKET_CONTEXT* pContext);

  bool StartIocp(const char* pSerIp, int serPort);
  void StopIocp();

  int GetProcessorsCnt();

  SOCKET_CONTEXT* MallocSocketContext();
  void FreeSocketContext(SOCKET_CONTEXT* pContext);

  static DWORD WINAPI WorkerThread(LPVOID lpParam);
  static DWORD WorkerMain(LPVOID lpParam);

#else

public:
  bool StartEpoll(const char* pSerIp, int serPort);
  void StopEpoll();

  void EpollThread();
  void WorkerThread();

#endif

private:
  enum {
    kMaxWorkThreadCnt = 128,    //最大的线程数
    kThreadsPerProcessor = 8,   //每一个处理器上运行多少个线程
    kConcurrentPerProcessor = 4, //每一个处理器可同时激活的线程数

    kSocketListenCnt = 1,      //监听套接字个数
    kSocketAcceptCnt = 15,     //提交的Accept数量
    kSocketWorkerCnt = 1024,   //套接字连接个数

    kContextCntPerBlk = 16,     //一个内存块存放64个对象
  };

  EventHandle* NewEvent(int socket, const char* pRemoteIp, int remotePort);
  void RemoveEvent(EventHandle* pEvent);

  bool InitLog();
  bool InitUser();
  bool InitTable();
  bool InitCommitLog();

private:
  std::string serIP_;
  int         serPort_;

#ifdef _WIN32

  HANDLE iocpHandle_;

  SOCKET_CONTEXT* pListenContext_;

  LPFN_ACCEPTEX              lpfnAcceptEx_;
  LPFN_GETACCEPTEXSOCKADDRS  lpfnGetAcceptExSockAddrs_;

  int workThreadCnt_;
  HANDLE workThreads_[kMaxWorkThreadCnt];

  std::mutex iocpMutex_;
  ObjectPool* pContextPool_;
#else

  int epollFd_;
  int licenseFd_;
  std::thread* workThreads_[kMaxWorkThreadCnt];

  std::mutex taskMutex_;
  std::list<EventHandle*> taskList_;
  std::condition_variable taskVariable_;

#endif

};

