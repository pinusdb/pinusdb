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

#include <string>
#include <mutex>
#include "pdb.h"
#include "internal.h"
#include "server/proto_header.h"
#include "expr/sql_parser.h"
#include "util/arena.h"
#include "expr/insert_sql.h"

class EventHandle
{
public:
  EventHandle(int socket, const char* pRemoteIp, int remotePort);
  ~EventHandle();

#ifdef _WIN32
  bool RecvPostedEvent(const uint8_t* pBuf, size_t bytesTransfered);
  bool SendPostedEvent(size_t bytesTransfered);
  bool GetRecvBuf(WSABUF* pWsaBuf);
  bool GetSendBuf(WSABUF* pWsaBuf);
#else
  bool RecvData();
  bool SendData();

  int GetSocket() const { return socket_; }
  inline void AddRef() { this->refCnt_.fetch_add(1); }
  inline bool MinusRef() { return this->refCnt_.fetch_sub(1) == 1; }
protected:
  bool _SendData();
#endif

public:
  bool ExecTask();

  inline int GetState() { return eventState_; }
  inline bool IsEnd() { return eventState_ == EventState::kEnd; }
  inline void SetEnd() { eventState_ = EventState::kEnd; }

  enum EventState
  {
    kRecv = 1,     //正在接收报文
    kExec = 2,     //正在执行或等待被执行
    kSend = 3,  //正在发送返回报文
    kEnd = 4,   //全部数据已提交
  };
protected:
  PdbErr_t DecodeHead();

  PdbErr_t DecodeSqlPacket(const char** ppSql, size_t* pSqlLen);
  PdbErr_t DecodeInsertTable(InsertSql* pInsertSql);
  PdbErr_t DecodeInsertSql(InsertSql* pInsertSql, Arena* pArena);

  PdbErr_t EncodeQueryPacket(PdbErr_t retVal, uint32_t fieldCnt, uint32_t recordCnt);
  PdbErr_t EncodeInsertPacket(PdbErr_t retVal, int32_t successCnt);
  PdbErr_t EncodeInsertTablePacket(PdbErr_t retVal, const std::list<PdbErr_t>& insertRet);

  PdbErr_t ExecLogin();

  PdbErr_t ExecQuery(std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt);
  PdbErr_t ExecInsertSql(int32_t* pSuccessCnt);
  PdbErr_t ExecInsertTable(std::list<PdbErr_t>& resultList);
  PdbErr_t ExecNonQuery();

  PdbErr_t _DropTable(const char* pTabName);
  PdbErr_t _DeleteDev(const char* pTabName, const DeleteParam* pDeleteParam);
  PdbErr_t _AttachTable(const char* pTabName);
  PdbErr_t _DetachTable(const char* pTabName);
  PdbErr_t _AttachFile(const char* pTabName, const DataFileParam* pDataFile);
  PdbErr_t _DetachFile(const char* pTabName, const DataFileParam* pDataFile);
  PdbErr_t _DropDataFile(const char* pTabName, const DataFileParam* pDataFile);
  PdbErr_t _CreateTable(const char* pTabName, const CreateTableParam* pCreateTableParam);
  PdbErr_t _CreateTable(const SQLParser* pParser);
  PdbErr_t _AddUser(const UserParam* pUserParam);
  PdbErr_t _ChangePwd(const UserParam* pUserParam);
  PdbErr_t _ChangeRole(const UserParam* pUserParam);
  PdbErr_t _DropUser(const UserParam* pUserParam);
  PdbErr_t _AlterTable(const char* pTabName, const CreateTableParam* pCreateTableParam);

  void FreeRecvBuf();
  void FreeSendBuf();

protected:
#ifndef _WIN32
  std::mutex eventMutex_;
  std::atomic<int> refCnt_;
  int socket_;
#endif

  int32_t eventState_;
  int32_t remotePort_;
  int32_t userRole_;                   //登录的角色
  std::string remoteIp_;
  std::string userName_;   //登录的用户

  uint32_t packetVersion_;                 //报文的版本
  uint32_t method_;                        //执行的方法
  uint32_t fieldCnt_;                      //字段数量
  uint32_t recCnt_;                        //记录条数
  uint32_t dataCrc_;                       //数据的CRC

  size_t   recvHeadLen_;                    //接收到的报文头长度
  char     recvHead_[ProtoHeader::kProtoHeadLength];   //报文头内容
  size_t   recvBodyLen_;                    //接收到的报文体长度
  size_t   sendLen_;                        //已发送报文长度

  std::string recvBuf_;  //接收报文缓冲区
  std::string sendBuf_;  //发送报文缓冲区
};



