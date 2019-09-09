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
#include "server/proto_header.h"
#include "query/data_table.h"
#include "expr/sql_parser.h"
#include "util/arena.h"
#include "server/iocp_event.h"
#include "expr/insert_sql.h"

class EventHandle : public IOCPEvent
{
public:
  EventHandle(const char* pRemoteIp, int remotePort);
  virtual ~EventHandle();

  virtual bool RecvPostedEvent(const uint8_t* pBuf, size_t bytesTransfered);
  virtual bool SendPostedEvent(size_t bytesTransfered);
  virtual bool ExecTask();

  virtual bool GetRecvBuf(WSABUF* pWsaBuf);
  virtual bool GetSendBuf(WSABUF* pWsaBuf);

protected:
  PdbErr_t DecodeHead();

  PdbErr_t DecodeSqlPacket(const char** ppSql, size_t* pSqlLen);
  PdbErr_t DecodeInsertTable(InsertTable* pInsertTab);

  PdbErr_t EncodeQueryPacket(PdbErr_t retVal, DataTable* pTable);
  PdbErr_t EncodeInsertPacket(PdbErr_t retVal, int32_t successCnt);
  PdbErr_t EncodeInsertTablePacket(PdbErr_t retVal, const std::list<PdbErr_t>& insertRet);

  PdbErr_t ExecLogin();

  PdbErr_t ExecQuery(DataTable* pResultTable);
  PdbErr_t ExecInsert(int32_t* pSuccessCnt);
  PdbErr_t ExecInsertTable(std::list<PdbErr_t>& resultList);
  PdbErr_t ExecNonQuery();

  PdbErr_t _DropTable(const DropTableParam* pDropTableParam);
  PdbErr_t _DeleteDev(const DeleteParam* pDeleteParam);
  PdbErr_t _AttachTable(const AttachTableParam* pAttachTableParam);
  PdbErr_t _DetachTable(const DetachTableParam* pDetachTableParam);
  PdbErr_t _AttachFile(const AttachFileParam* pAttachFileParam);
  PdbErr_t _DetachFile(const DetachFileParam* pDetachFileParam);
  PdbErr_t _DropDataFile(const DropFileParam* pDropFileParam);
  PdbErr_t _CreateTable(const CreateTableParam* pCreateTableParam);
  PdbErr_t _AddUser(const AddUserParam* pAddUserParam);
  PdbErr_t _ChangePwd(const ChangePwdParam* pChangePwdParam);
  PdbErr_t _ChangeRole(const ChangeRoleParam* pChangeRoleParam);
  PdbErr_t _DropUser(const DropUserParam* pDropUserParam);

  PdbErr_t AllocRecvBuf(size_t bufLen);
  void FreeRecvBuf();

  PdbErr_t AllocSendBuf(size_t bufLen);
  void FreeSendBuf();

private:
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
  uint8_t  recvHead_[ProtoHeader::kProtoHeadLength];   //报文头内容
  size_t   recvBodyLen_;                    //接收到的报文体长度
  size_t   totalRecvBodyLen_;               //报文体的总长度

  uint8_t* pRecvBuf_;                      //接收缓存
  size_t   recvBufLen_;                    //接收缓存大小

  size_t   sendLen_;                        //已发送报文长度
  size_t   totalSendLen_;                   //发送报文总长度

  size_t   sendBufLen_;                     //发送缓存大小
  uint8_t* pSendBuf_;                      //发送缓存

};



