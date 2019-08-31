#pragma once

#include "internal.h"
#include "util/object_pool.h"

class IOCPEvent
{
public:
  IOCPEvent()
  {
    eventState_ = EventState::kRecv;
  }
  virtual ~IOCPEvent()
  {
  }

  virtual bool RecvPostedEvent(const uint8_t* pBuf, size_t bytesTransfered) = 0;

  virtual bool SendPostedEvent(size_t bytesTransfered) = 0;

  virtual bool ExecTask() = 0;

  virtual bool GetRecvBuf(WSABUF* pWsaBuf) = 0;
  virtual bool GetSendBuf(WSABUF* pWsaBuf) = 0;

  enum EventState
  {
    kRecv = 1,     //正在接收报文
    kExec = 2,     //正在执行或等待被执行
    kSend = 3,  //正在发送返回报文
    kEnd = 4,   //全部数据已提交
  };

  inline int GetState() { return (eventState_); }

  inline bool IsEnd()
  {
    return eventState_ == EventState::kEnd;
  }

  inline void SetEnd()
  {
    eventState_ = EventState::kEnd;
  }

protected:
  int eventState_;                      //状态

};