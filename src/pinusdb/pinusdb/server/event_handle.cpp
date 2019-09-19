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

#include "server/event_handle.h"
#include "pdb_error.h"
#include "util/date_time.h"
#include "util/log_util.h"
#include "util/string_tool.h"
#include "server/user_config.h"

#include "expr/sql_parser.h"
#include "expr/tokenize.h"

#include "db/db_impl.h"
#include "global_variable.h"
#include "util/coding.h"

EventHandle::EventHandle(const char* pRemoteIp, int remotePort)
{
  this->remotePort_ = remotePort;
  this->remoteIp_ = pRemoteIp;
  this->userRole_ = 0;
  this->userName_ = "";

  this->packetVersion_ = 0;
  this->method_ = 0;
  this->fieldCnt_ = 0;
  this->recCnt_ = 0;
  this->dataCrc_ = 0;

  this->recvHeadLen_ = 0;
  this->recvBodyLen_ = 0;
  this->totalRecvBodyLen_ = 0;

  this->pRecvBuf_ = nullptr;
  this->recvBufLen_ = 0;

  this->sendLen_ = 0;
  this->totalSendLen_ = 0;

  this->sendBufLen_ = 0;
  this->pSendBuf_ = nullptr;
}


EventHandle::~EventHandle()
{
  LOG_INFO("disconnect ({}:{})", remoteIp_.c_str(), remotePort_);

  if (pRecvBuf_ != nullptr)
    delete pRecvBuf_;

  if (pSendBuf_ != nullptr)
    delete pSendBuf_;
}

bool EventHandle::RecvPostedEvent(const uint8_t* pBuf, size_t bytesTransfered)
{
  PdbErr_t retVal = PdbE_OK;
  const uint8_t* pTmpBuf = pBuf;

  if (pBuf == nullptr || bytesTransfered <= 0)
  {
    LOG_ERROR("failed to RecvPostedEvent, transfered bytes ({}), socket will be closed", 
      bytesTransfered);
    this->SetEnd();
    return false;
  }

  if (this->IsEnd())
  {
    LOG_ERROR("failed to RecvPostedEvent, socket is closed");
    return false;
  }

  do {
    if (this->eventState_ != EventState::kRecv)
    {
      LOG_ERROR("execute recv faield, client ({}:{}), current state ({}) can't recv data",
        remoteIp_.c_str(), remotePort_, this->eventState_);
      retVal = PdbE_TASK_STATE_ERROR;
      break;
    }

    if (recvHeadLen_ < ProtoHeader::kProtoHeadLength)
    {
      size_t tmpLen = ProtoHeader::kProtoHeadLength - recvHeadLen_;
      tmpLen = tmpLen > bytesTransfered ? bytesTransfered : tmpLen;

      memcpy((recvHead_ + recvHeadLen_), pTmpBuf, tmpLen);

      bytesTransfered -= tmpLen;
      pTmpBuf += tmpLen;
      recvHeadLen_ += tmpLen;

      if (recvHeadLen_ == ProtoHeader::kProtoHeadLength)
      {
        retVal = DecodeHead();
        if (retVal != PdbE_OK)
        {
          LOG_ERROR("decode packet head failed, client ({}:{}) err:{}", 
            remoteIp_.c_str(), remotePort_, retVal);
          break;
        }
      }
    }
    
    if (bytesTransfered == 0)
      break;

    //接收报文体
    if ((recvBodyLen_ + bytesTransfered) > totalRecvBodyLen_)
    {
      LOG_ERROR("connection ({}:{}) received overflow, received:({}) total:({})",
        remoteIp_.c_str(), remotePort_, (recvBodyLen_ + bytesTransfered), totalRecvBodyLen_);
      retVal = PdbE_PACKET_ERROR;
      break;
    }

    //判断要不要拷贝
    if (recvBodyLen_ != 0 || pRecvBuf_ != pTmpBuf)
    {
      //需要拷贝
      memcpy((pRecvBuf_ + recvBodyLen_), pTmpBuf, bytesTransfered);
    }
    recvBodyLen_ += bytesTransfered;

    if (recvBodyLen_ == totalRecvBodyLen_)
    {
      uint32_t tmpCrc32 = StringTool::CRC32(pRecvBuf_, totalRecvBodyLen_);
      if (tmpCrc32 != dataCrc_)
      {
        LOG_ERROR("connection ({}:{}) packet body crc error", 
          remoteIp_.c_str(), remotePort_);
        retVal = PdbE_PACKET_ERROR;
        break;
      }

      this->eventState_ = EventState::kExec;
    }

  } while (false);

  if (retVal != PdbE_OK)
  {
    LOG_ERROR("connection ({}:{}) execute recv failed, connection will be closed",
      remoteIp_.c_str(), remotePort_);
    this->SetEnd();
    return false;
  }

  return true;
}

bool EventHandle::SendPostedEvent(size_t bytesTransfered)
{
  PdbErr_t retVal = PdbE_OK;

  do {

    if (bytesTransfered <= 0)
    {
      LOG_ERROR("failed to send packet, sent packet length ({})", bytesTransfered);
      retVal = PdbE_TASK_STATE_ERROR;
      break;
    }

    if (this->IsEnd())
    {
      LOG_ERROR("failed to send packet, socket closed");
      retVal = PdbE_TASK_STATE_ERROR;
      break;
    }
  
    if (this->eventState_ != EventState::kSend)
    {
      LOG_ERROR("failed to send packet, current state ({})", this->eventState_);
      retVal = PdbE_TASK_STATE_ERROR;
      break;
    }

    if (sendLen_ + bytesTransfered > totalSendLen_)
    {
      LOG_ERROR("failed to send packet, total length ({}),  sent length ({}) error", 
        totalSendLen_, (sendLen_ + bytesTransfered));
      retVal = PdbE_PACKET_ERROR;
      break;
    }

    sendLen_ += bytesTransfered;

    if (sendLen_ == totalSendLen_)
    {
      this->eventState_ = EventState::kRecv;

      this->recvHeadLen_ = 0;
      this->recvBodyLen_ = 0;
      this->totalRecvBodyLen_ = 0;

      this->sendLen_ = 0;
      this->totalSendLen_ = 0;

      FreeSendBuf();
    }

  } while (false);

  if (retVal != PdbE_OK)
  {
    this->SetEnd();
    return false;
  }

  return true;
}

bool EventHandle::ExecTask()
{
  PdbErr_t retVal = PdbE_OK;

  int32_t successCnt = 0;
  int32_t errPos = 0;
  DataTable resultTable;
  uint32_t repMethodId = METHOD_ERROR_REP;
  std::list<PdbErr_t> insertRet;

  if (this->IsEnd())
  {
    LOG_ERROR("failed to exec task, socket({}:{}) is closed",
      remoteIp_.c_str(), remotePort_);
    return false;
  }

  switch (method_)
  {
  case METHOD_CMD_LOGIN_REQ:
  {
    retVal = ExecLogin();
    repMethodId = METHOD_CMD_LOGIN_REP;
    break;
  }
  case METHOD_CMD_QUERY_REQ:
  {
    retVal = ExecQuery(&resultTable);
    repMethodId = METHOD_CMD_QUERY_REP;
    break;
  }
  case METHOD_CMD_INSERT_REQ:
  {
    retVal = ExecInsert(&successCnt);
    repMethodId = METHOD_CMD_INSERT_REP;
    break;
  }
  case METHOD_CMD_NONQUERY_REQ:
  {
    retVal = ExecNonQuery();
    repMethodId = METHOD_CMD_NONQUERY_REP;
    break;
  }
  case METHOD_CMD_INSERT_TABLE_REQ:
  {
    retVal = ExecInsertTable(insertRet);
    repMethodId = METHOD_CMD_INSERT_TABLE_REP;
    break;
  }
  default:
  {
    retVal = PdbE_PACKET_ERROR;
    break;
  }
  }

  if (method_ == METHOD_CMD_QUERY_REQ)
  {
    EncodeQueryPacket(retVal, &resultTable);
  }
  else if (method_ == METHOD_CMD_INSERT_REQ)
  {
    EncodeInsertPacket(retVal, successCnt);
  }
  else if (method_ == METHOD_CMD_INSERT_TABLE_REQ)
  {
    EncodeInsertTablePacket(retVal, insertRet);
  }
  else
  {
    //其它报文
    AllocSendBuf(ProtoHeader::kProtoHeadLength);
    ProtoHeader proHdr;
    proHdr.Load(pSendBuf_);
    proHdr.InitHeader(repMethodId, 0, retVal, 0);
    this->totalSendLen_ = ProtoHeader::kProtoHeadLength;
  }

  //如果接收缓冲大于某个值，则释放
  FreeRecvBuf();

  this->sendLen_ = 0;
  this->recvHeadLen_ = 0;
  this->recvBodyLen_ = 0;
  this->totalRecvBodyLen_ = 0;

  this->eventState_ = EventState::kSend;
  return true;
}

bool EventHandle::GetRecvBuf(WSABUF* pWsaBuf)
{
  //收报文头
  if (pWsaBuf == nullptr)
  {
    LOG_ERROR("failed to GetRecvBuf, invalid param");
    this->SetEnd();
    return false;
  }

  if (this->IsEnd())
  {
    LOG_ERROR("failed to get socket recv buffer, socket is closed");
    this->SetEnd();
    return false;
  }

  if (this->eventState_ != EventState::kRecv)
  {
    LOG_ERROR("failed to get socket recv buffer, socket state({}) error", this->eventState_);
    this->SetEnd();
    return false;
  }

  if (recvHeadLen_ < ProtoHeader::kProtoHeadLength)
  {
    //接收报文头
    pWsaBuf->len = static_cast<ULONG>(ProtoHeader::kProtoHeadLength - recvHeadLen_);
  }
  else if (recvBodyLen_ == 0)
  {
    //接收报文体，直接指向报文体的缓冲区
    pWsaBuf->buf = (char*)pRecvBuf_;
    pWsaBuf->len = static_cast<ULONG>(totalRecvBodyLen_);
  }
  //其它清空使用默认的缓冲区空间及大小

  return true;
}

bool EventHandle::GetSendBuf(WSABUF* pWsaBuf)
{
  if (pWsaBuf == nullptr)
  {
    LOG_ERROR("failed to get socket send buffer, invalid param");
    this->SetEnd();
    return false;
  }

  if (this->IsEnd())
  {
    LOG_ERROR("failed to get socket send buffer, socket is closed");
    this->SetEnd();
    return false;
  }

  if (this->eventState_ != EventState::kSend)
  {
    LOG_ERROR("failed to get socket send buffer, socket state({}) error", this->eventState_);
    this->SetEnd();
    return false;
  }

  if (sendLen_ == 0)
  {
    pWsaBuf->buf = (char*)pSendBuf_;
    pWsaBuf->len = static_cast<ULONG>(totalSendLen_);
  }
  else
  {
    size_t tmpLen = pWsaBuf->len > (totalSendLen_ - sendLen_) ? (totalSendLen_ - sendLen_) : pWsaBuf->len;
    memcpy(pWsaBuf->buf, (pRecvBuf_ + sendLen_), tmpLen);
    pWsaBuf->len = static_cast<ULONG>(tmpLen);
  }

  return true;
}

PdbErr_t EventHandle::DecodeHead()
{
  PdbErr_t retVal = PdbE_OK;

  ProtoHeader proHdr;

  if (recvHeadLen_ != ProtoHeader::kProtoHeadLength)
  {
    LOG_ERROR("failed to decode packet, packet head length ({}) error", recvHeadLen_);
    return PdbE_PACKET_ERROR;
  }

  proHdr.Load(recvHead_);

  packetVersion_ = proHdr.GetVersion();
  method_ = proHdr.GetMethod();
  fieldCnt_ = proHdr.GetFieldCnt();
  recCnt_ = proHdr.GetRecordCnt();
  totalRecvBodyLen_ = proHdr.GetBodyLen();
  dataCrc_ = proHdr.GetBodyCrc();
  //验证头部CRC是否正确
  {
    uint32_t headCrc = proHdr.GetHeadCrc();
    uint32_t tmpCrc = StringTool::CRC32(recvHead_, (ProtoHeader::kProtoHeadCalcCrcLen));
    if (headCrc != tmpCrc)
    {
      LOG_ERROR("packet crc error");
      return PdbE_PACKET_ERROR;
    }
  }

  //验证是否支持该版本的报文

  if (totalRecvBodyLen_ < 0
    || totalRecvBodyLen_ > PDB_MAX_PACKET_BODY_LEN
    || recCnt_ > PDB_MAX_PACKET_REC_CNT)
  {
    LOG_ERROR("packet length ({}) error", totalRecvBodyLen_);
    return PdbE_PACKET_ERROR;
  }
  //分配空间
  retVal = AllocRecvBuf(totalRecvBodyLen_);
  if (retVal != PdbE_OK)
    return retVal;

  recvBodyLen_ = 0;

  return PdbE_OK;
}


PdbErr_t EventHandle::DecodeSqlPacket(const char** ppSql, size_t* pSqlLen)
{
  if (recvBodyLen_ <= 0)
  {
    LOG_ERROR("failed to decode insert packet, packet length ({}) error", recvBodyLen_);
    return PdbE_PACKET_ERROR;
  }

  if (ppSql == nullptr || pSqlLen == nullptr)
  {
    LOG_ERROR("failed to decode insert packet, invalid param");
    return PdbE_INVALID_PARAM;
  }

  uint8_t* pTmp = pRecvBuf_;
  *pSqlLen = recvBodyLen_;
  *ppSql = (const char*)pRecvBuf_;
  return PdbE_OK;
}

PdbErr_t EventHandle::DecodeInsertTable(InsertTable* pInsertTab)
{
  const uint8_t* pTmp = pRecvBuf_;
  const uint8_t* pBufLimit = pRecvBuf_ + recvBodyLen_;
  DBVal dbVal;
  uint32_t vType = 0;
  uint32_t vLen = 0;
  uint32_t v32 = 0;
  uint64_t v64 = 0;

  if (recvBodyLen_ == 0 || recCnt_ == 0 || fieldCnt_ == 0)
  {
    LOG_ERROR("failed to decode insert packet, packet length: ({}), field count: ({}), row count: ({})",
      recvBodyLen_, fieldCnt_, recCnt_);
    return PdbE_PACKET_ERROR;
  }

  while (pTmp < pBufLimit)
  {
    vType = *pTmp++;
    switch (vType)
    {
    case PDB_VALUE_TYPE::VAL_NULL:
      DBVAL_SET_NULL(&dbVal);
      break;
    case PDB_VALUE_TYPE::VAL_BOOL:
      DBVAL_SET_BOOL(&dbVal, (*pTmp == PDB_BOOL_TRUE));
      pTmp++;
      break;
    case PDB_VALUE_TYPE::VAL_INT64:
      pTmp = Coding::VarintDecode64(pTmp, pBufLimit, &v64);
      DBVAL_SET_INT64(&dbVal, Coding::ZigzagDecode64(v64));
      break;
    case PDB_VALUE_TYPE::VAL_DATETIME:
      pTmp = Coding::VarintDecode64(pTmp, pBufLimit, &v64);
      if (v64 > MaxMillis)
      {
        return PdbE_INVALID_DATETIME_VAL;
      }
      DBVAL_SET_DATETIME(&dbVal, v64);
      break;
    case PDB_VALUE_TYPE::VAL_DOUBLE:
      v64 = Coding::FixedDecode64(pTmp);
      DBVAL_SET_DOUBLE(&dbVal, Coding::DecodeDouble(v64));
      pTmp += sizeof(uint64_t);
      break;
    case PDB_VALUE_TYPE::VAL_STRING:
      pTmp = Coding::VarintDecode32(pTmp, pBufLimit, &v32);
      if (pTmp == nullptr)
      {
        return PdbE_PACKET_ERROR;
      }
      DBVAL_SET_STRING(&dbVal, pTmp, v32);
      pTmp += v32;
      break;
    case PDB_VALUE_TYPE::VAL_BLOB:
      pTmp = Coding::VarintDecode32(pTmp, pBufLimit, &v32);
      if (pTmp == nullptr)
      {
        return PdbE_PACKET_ERROR;
      }
      DBVAL_SET_BLOB(&dbVal, pTmp, v32);
      pTmp += v32;
      break;
    default:
      return PdbE_PACKET_ERROR;
    }

    if (pTmp == nullptr)
      return PdbE_PACKET_ERROR;

    pInsertTab->AddVal(&dbVal);
  }

  if (pTmp != pBufLimit)
    return PdbE_PACKET_ERROR;

  return PdbE_OK;
}


PdbErr_t EventHandle::EncodeQueryPacket(PdbErr_t retVal, DataTable* pTable)
{
  do {
    if (retVal != PdbE_OK)
      break;

    uint32_t fieldCnt = static_cast<uint32_t>(pTable->GetColumnCnt());
    uint32_t recCnt = static_cast<uint32_t>(pTable->GetRecordCnt());

    size_t tmpLen = 0;
    retVal = pTable->GetSerializeLen(&tmpLen);
    if (retVal != PdbE_OK)
      break;

    retVal = AllocSendBuf((ProtoHeader::kProtoHeadLength + tmpLen));
    if (retVal != PdbE_OK)
      break;

    size_t bodyLen = 0;
    uint8_t* pTmpBodyBuf = pSendBuf_ + ProtoHeader::kProtoHeadLength;
    retVal = pTable->Serialize(pTmpBodyBuf, &bodyLen);
    if (retVal != PdbE_OK)
      break;

    uint32_t bodyCrc = StringTool::CRC32(pTmpBodyBuf, bodyLen);

    ProtoHeader proHdr;
    proHdr.Load(pSendBuf_);
    proHdr.InitHeader(METHOD_CMD_QUERY_REP, static_cast<int32_t>(bodyLen), PdbE_OK, bodyCrc);
    proHdr.SetRecordCnt(recCnt);
    proHdr.SetFieldCnt(fieldCnt);
    proHdr.UpdateHeadCrc();
    
    totalSendLen_ = (bodyLen + ProtoHeader::kProtoHeadLength);
    return PdbE_OK;
  } while (false);

  AllocSendBuf(ProtoHeader::kProtoHeadLength);
  ProtoHeader proHdr;
  proHdr.Load(pSendBuf_);
  proHdr.InitHeader(METHOD_CMD_QUERY_REP, 0, retVal, 0);
  this->totalSendLen_ = ProtoHeader::kProtoHeadLength;
  
  return PdbE_OK;
}

PdbErr_t EventHandle::EncodeInsertPacket(PdbErr_t retVal, int32_t successCnt)
{
  AllocSendBuf(ProtoHeader::kProtoHeadLength);
  ProtoHeader proHdr;
  proHdr.Load(pSendBuf_);
  proHdr.InitHeader(METHOD_CMD_INSERT_REP, 0, retVal, 0);
  proHdr.SetRecordCnt(successCnt);
  proHdr.SetErrPos(0);
  proHdr.UpdateHeadCrc();

  this->totalSendLen_ = ProtoHeader::kProtoHeadLength;
  return PdbE_OK;
}

PdbErr_t EventHandle::EncodeInsertTablePacket(PdbErr_t retVal, const std::list<PdbErr_t>& insertRet)
{
  ProtoHeader proHdr;
  
  if (retVal == PdbE_OK)
  {
    AllocSendBuf(ProtoHeader::kProtoHeadLength);
    proHdr.Load(pSendBuf_);
    proHdr.InitHeader(METHOD_CMD_INSERT_TABLE_REP, 0, PdbE_OK, 0);
    proHdr.SetRecordCnt(static_cast<uint32_t>(insertRet.size()));
    proHdr.UpdateHeadCrc();
    this->totalSendLen_ = ProtoHeader::kProtoHeadLength;
    return PdbE_OK;
  }

  do {
    if (retVal != PdbE_INSERT_PART_ERROR)
      break;

    size_t tmpLen = insertRet.size() * 9;
    retVal = AllocSendBuf((ProtoHeader::kProtoHeadLength + tmpLen));
    if (retVal != PdbE_OK)
      break;

    size_t bodyLen = 0;
    uint8_t* pBodyBuf = pSendBuf_ + ProtoHeader::kProtoHeadLength;
    uint8_t* pTmpBody = pBodyBuf;
    for (auto retIt = insertRet.begin(); retIt != insertRet.end(); retIt++)
    {
      pTmpBody = Coding::VarintEncode32(pTmpBody, Coding::ZigzagEncode32(*retIt));
    }

    bodyLen = pTmpBody - pBodyBuf;
    uint32_t bodyCrc = StringTool::CRC32(pBodyBuf, bodyLen);

    proHdr.Load(pSendBuf_);
    proHdr.InitHeader(METHOD_CMD_INSERT_TABLE_REP, static_cast<int32_t>(bodyLen), PdbE_INSERT_PART_ERROR, bodyCrc);
    proHdr.SetRecordCnt(static_cast<uint32_t>(insertRet.size()));
    proHdr.UpdateHeadCrc();

    totalSendLen_ = (bodyLen + ProtoHeader::kProtoHeadLength);
    return PdbE_OK;
  } while (false);

  AllocSendBuf(ProtoHeader::kProtoHeadLength);
  proHdr.Load(pSendBuf_);
  proHdr.InitHeader(METHOD_CMD_INSERT_TABLE_REP, 0, retVal, 0);
  proHdr.UpdateHeadCrc();

  this->totalSendLen_ = ProtoHeader::kProtoHeadLength;
  return PdbE_OK;
}

PdbErr_t EventHandle::ExecLogin()
{
  PdbErr_t retVal = PdbE_OK;

  userRole_ = 0;
  userName_ = "";

  if (recvBodyLen_ != (PDB_USER_NAME_LEN + PDB_PWD_CRC32_LEN))
  {
    LOG_INFO("failed  to login, packet length({}) error", recvBodyLen_);
    return PdbE_PACKET_ERROR;
  }

  uint8_t* pTmp = pRecvBuf_;

  uint32_t pwd = Coding::FixedDecode32((pTmp + PDB_USER_NAME_LEN));
  int32_t role = 0;
  size_t nameLen = 0;
  while (pTmp[nameLen] != '\0')
  {
    if (nameLen >= PDB_USER_NAME_LEN)
      return PdbE_INVALID_USER_NAME;

    nameLen++;
  }

  retVal = pGlbUser->Login((const char*)pTmp, pwd, &role);
  if (retVal != PdbE_OK)
  {
    LOG_DEBUG("login failed ({}:{}), ret: ({})", 
      remoteIp_.c_str(), remotePort_, retVal);
    return retVal;
  }

  userRole_ = role;
  userName_ = (char*)pTmp;

  pGlbServerConnction->ConnectionLogin((uint64_t)this, (const char*)pTmp, role);
  LOG_INFO("login successed, client:({}:{}), user:({}), role:({})", 
    remoteIp_.c_str(), remotePort_, userName_, userRole_);
  return PdbE_OK;
}


PdbErr_t EventHandle::ExecQuery(DataTable* pResultTable)
{
  PdbErr_t retVal = PdbE_OK;
  Arena arena;

  if (pResultTable == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  if (userRole_ != PDB_ROLE::ROLE_ADMIN
    && userRole_ != PDB_ROLE::ROLE_READ_ONLY
    && userRole_ != PDB_ROLE::ROLE_READ_WRITE)
  {
    return PdbE_OPERATION_DENIED; //没有权限
  }

  const char* pSql = nullptr;
  size_t sqlLen = 0;
  retVal = DecodeSqlPacket(&pSql, &sqlLen);
  if (retVal != PdbE_OK)
    return retVal;

  SQLParser sqlParser;
  Tokenize::RunParser(&arena, &sqlParser, pSql, sqlLen);
  if (sqlParser.GetError())
  {
    return PdbE_SQL_ERROR;
  }

  if (sqlParser.GetCmdType() != SQLParser::CmdType::CT_Select)
  {
    return PdbE_SQL_ERROR;
  }

  retVal = pGlbTableSet->ExecuteQuery(pResultTable, &sqlParser, userRole_);
  return retVal;
}

PdbErr_t EventHandle::ExecInsert(int32_t* pSuccessCnt)
{
  PdbErr_t retVal = PdbE_OK;
  Arena arena;
  InsertSql* pInsertSql = nullptr;
  std::list<PdbErr_t> resultList;

  *pSuccessCnt = 0;

  if (userRole_ != PDB_ROLE::ROLE_ADMIN
    && userRole_ != PDB_ROLE::ROLE_WRITE_ONLY
    && userRole_ != PDB_ROLE::ROLE_READ_WRITE)
  {
    return PdbE_OPERATION_DENIED; //没有权限
  }

  do {
    const char* pSql = nullptr;
    size_t sqlLen = 0;
    retVal = DecodeSqlPacket(&pSql, &sqlLen);
    if (retVal != PdbE_OK)
      break;

    pInsertSql = new InsertSql();
    if (pInsertSql == nullptr)
    {
      retVal = PdbE_NOMEM;
      break;
    }

    retVal = Tokenize::RunInsertParser(&arena, pInsertSql, pSql, sqlLen);
    if (retVal != PdbE_OK)
      break;

    retVal = pInsertSql->ParseMeta();
    if (retVal != PdbE_OK)
      break;

    retVal = pGlbTableSet->Insert(pInsertSql, true, resultList);

  } while (false);

  if (pInsertSql != nullptr)
    delete pInsertSql;

  *pSuccessCnt = static_cast<int32_t>(resultList.size());
  return retVal;
}

PdbErr_t EventHandle::ExecInsertTable(std::list<PdbErr_t>& resultList)
{
  PdbErr_t retVal = PdbE_OK;
  InsertTable* pInsertTab = nullptr;

  if (userRole_ != PDB_ROLE::ROLE_ADMIN
    && userRole_ != PDB_ROLE::ROLE_WRITE_ONLY
    && userRole_ != PDB_ROLE::ROLE_READ_WRITE)
  {
    return PdbE_OPERATION_DENIED; //没有权限
  }

  do {
    pInsertTab = new InsertTable(fieldCnt_, recCnt_);
    if (pInsertTab == nullptr)
    {
      retVal = PdbE_NOMEM;
      break;
    }

    retVal = DecodeInsertTable(pInsertTab);
    if (retVal != PdbE_OK)
      break;

    retVal = pInsertTab->ParseMeta();
    if (retVal != PdbE_OK)
      break;

    retVal = pGlbTableSet->Insert(pInsertTab, false, resultList);

  } while (false);

  if (pInsertTab != nullptr)
  {
    delete pInsertTab;
  }

  return retVal;
}

PdbErr_t EventHandle::ExecNonQuery()
{
  PdbErr_t retVal = PdbE_OK;
  const char* pSql = nullptr;
  size_t sqlLen = 0;
  Arena arena;

  retVal = DecodeSqlPacket(&pSql, &sqlLen);
  if (retVal != PdbE_OK)
    return retVal;

  SQLParser sqlParser;
  Tokenize::RunParser(&arena, &sqlParser, pSql, sqlLen);
  if (sqlParser.GetError())
  {
    return PdbE_SQL_ERROR;
  }

  int32_t cmdType = sqlParser.GetCmdType();
  switch (cmdType)
  {
    case SQLParser::CmdType::CT_DropTable: // 需要管理员权限
      retVal = _DropTable(sqlParser.GetDropTableParam());
      break;
    case SQLParser::CmdType::CT_Delete: // 需要管理员权限
      retVal = _DeleteDev(sqlParser.GetDeleteParam());
      break;
    case SQLParser::CmdType::CT_AttachTable: // 需要管理员权限
      retVal = _AttachTable(sqlParser.GetAttachTableParam());
      break;
    case SQLParser::CmdType::CT_DetachTable:
      retVal = _DetachTable(sqlParser.GetDetachTableParam());
      break;
    case SQLParser::CmdType::CT_AttachFile:
      retVal = _AttachFile(sqlParser.GetAttachFileParam());
      break;
    case SQLParser::CmdType::CT_DetachFile:
      retVal = _DetachFile(sqlParser.GetDetachFileParam());
      break;
    case SQLParser::CmdType::CT_DropFile:
      retVal = _DropDataFile(sqlParser.GetDropFileParam());
      break;
    case SQLParser::CmdType::CT_CreateTable: // 必须要Admin权限
      retVal = _CreateTable(sqlParser.GetCreateTableParam());
      break;
    case SQLParser::CmdType::CT_AddUser: // 必须要Admin权限
      retVal = _AddUser(sqlParser.GetAddUserParam());
      break;
    case SQLParser::CmdType::CT_ChangePwd:
      retVal = _ChangePwd(sqlParser.GetChangePwdParam());
      break;
    case SQLParser::CmdType::CT_ChangeRole: // 必须要Admin权限
      retVal = _ChangeRole(sqlParser.GetChangeRoleParam());
      break;
    case SQLParser::CmdType::CT_DropUser:
      retVal = _DropUser(sqlParser.GetDropUserParam());
      break;
    case SQLParser::CmdType::CT_AlterTable:
      retVal = _AlterTable(sqlParser.GetCreateTableParam());
      break;
    default:
    {
      retVal = PdbE_SQL_ERROR;
      break;
    }
  }

  return retVal;
}

PdbErr_t EventHandle::_DropTable(const DropTableParam* pDropTableParam)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  retVal = pGlbTableSet->DropTable(pDropTableParam->tabName_.c_str());
  if (retVal == PdbE_OK)
  {
    LOG_INFO("user ({}), drop table ({}) successed",
      userName_.c_str(), pDropTableParam->tabName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}), drop table ({}) failed, err: {}", 
      userName_.c_str(), pDropTableParam->tabName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_DeleteDev(const DeleteParam* pDeleteParam)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  retVal = pGlbTableSet->DeleteDev(pDeleteParam);
  if (retVal == PdbE_OK)
  {
    LOG_INFO("user ({}), delete device successed", userName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}), delete device failed, err: {}",
      userName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_AttachTable(const AttachTableParam* pAttachTableParam)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  retVal = pGlbTableSet->AttachTable(pAttachTableParam->tabName_.c_str());
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) attach table ({}) successful",
      userName_.c_str(), pAttachTableParam->tabName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}) attach table ({}) failed, err: {}",
      userName_.c_str(), pAttachTableParam->tabName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_DetachTable(const DetachTableParam* pDetachTableParam)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  retVal = pGlbTableSet->DetachTable(pDetachTableParam->tabName_.c_str());
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) detach table ({}) successful",
      userName_.c_str(), pDetachTableParam->tabName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}) detach table ({}) failed, err: {}",
      userName_.c_str(), pDetachTableParam->tabName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_AttachFile(const AttachFileParam* pAttachFileParam)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fileType = PDB_PART_TYPE_NORMAL_VAL;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  if (StringTool::ComparyNoCase(pAttachFileParam->fileType_.c_str(), PDB_PART_TYPE_NORMAL_STR))
    fileType = PDB_PART_TYPE_NORMAL_VAL;
  else if (StringTool::ComparyNoCase(pAttachFileParam->fileType_.c_str(), PDB_PART_TYPE_COMPRESS_STR))
    fileType = PDB_PART_TYPE_COMPRESS_VAL;
  else
  {
    LOG_INFO("user ({}) attach data file ({}) for table ({}) failed, unknown data file type",
      userName_.c_str(), pAttachFileParam->dateStr_.c_str(), pAttachFileParam->tabName_.c_str());
    return PdbE_INVALID_PARAM;
  }

  retVal = pGlbTableSet->AttachFile(pAttachFileParam->tabName_.c_str(), 
    pAttachFileParam->dateStr_.c_str(), fileType);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) attach data file ({}) for table ({}) failed, err: {}",
      userName_.c_str(), pAttachFileParam->dateStr_.c_str(), pAttachFileParam->tabName_.c_str(), retVal);
  }
  else
  {
    LOG_INFO("user ({}) attach data file ({}) for table ({}) successful",
      userName_.c_str(), pAttachFileParam->dateStr_.c_str(), pAttachFileParam->tabName_.c_str());
  }

  return retVal;
}

PdbErr_t EventHandle::_DetachFile(const DetachFileParam* pDetachFileParam)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t partCode = 0;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  if (!DateTime::ParseDate(pDetachFileParam->dateStr_.c_str(),
    pDetachFileParam->dateStr_.size(), &partCode))
  {
    LOG_INFO("user ({}) detach data file ({}) for table ({}) failed, invalid date param",
      userName_.c_str(), pDetachFileParam->dateStr_.c_str(), pDetachFileParam->tabName_.c_str());
    return PdbE_INVALID_PARAM;
  }

  retVal = pGlbTableSet->DetachFile(pDetachFileParam->tabName_.c_str(), partCode);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) detach data file ({}) for table ({}) failed, err: {}",
      userName_.c_str(), pDetachFileParam->dateStr_.c_str(), 
      pDetachFileParam->tabName_.c_str(), retVal);
  }
  else
  {
    LOG_INFO("user ({}) detach data file ({}) for table ({}) successful",
      userName_.c_str(), pDetachFileParam->dateStr_.c_str(),
      pDetachFileParam->tabName_.c_str());
  }

  return retVal;
}

PdbErr_t EventHandle::_DropDataFile(const DropFileParam* pDropFileParam)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t partCode = 0;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  if (!DateTime::ParseDate(pDropFileParam->dateStr_.c_str(),
    pDropFileParam->dateStr_.size(), &partCode))
  {
    LOG_INFO("user ({}) drop data file ({}) for table ({}) failed, invalid date param",
      userName_.c_str(), pDropFileParam->dateStr_.c_str(), 
      pDropFileParam->dateStr_.c_str(), pDropFileParam->tabName_.c_str());
    return PdbE_INVALID_PARAM;
  }

  retVal = pGlbTableSet->DropFile(pDropFileParam->tabName_.c_str(), partCode);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) drop data file ({}) for table ({}) failed, err: {}",
      userName_.c_str(), pDropFileParam->dateStr_.c_str(), pDropFileParam->tabName_.c_str(),  retVal);
  }
  else
  {
    LOG_INFO("user ({}) drop data file ({}) for table ({}) successful",
      userName_.c_str(), pDropFileParam->dateStr_.c_str(), pDropFileParam->tabName_.c_str());
  }

  return retVal;
}

PdbErr_t EventHandle::_CreateTable(const CreateTableParam* pCreateTableParam)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  retVal = pGlbTableSet->CreateTable(pCreateTableParam);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) create table ({}) failed, err: {}",
      userName_.c_str(), pCreateTableParam->tabName_.c_str(), retVal);
  }
  else
  {
    LOG_INFO("user ({}) create table ({}) successful",
      userName_.c_str(), pCreateTableParam->tabName_.c_str());
  }

  return retVal;
}

PdbErr_t EventHandle::_AddUser(const AddUserParam* pAddUserParam)
{
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED; //判断是否有添加用户的权限

  uint32_t pwd32 = StringTool::CRC32(pAddUserParam->pwd_.c_str());
  PdbErr_t retVal = pGlbUser->AddUser(pAddUserParam->userName_.c_str(),
    pwd32, "readwrite");
  if (retVal == PdbE_OK)
  {
    LOG_INFO("user ({}) add user ({}:readwrite) successful",
      userName_.c_str(), pAddUserParam->userName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}) add user ({}:readwrite) failed, err: {}",
      userName_.c_str(), pAddUserParam->userName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_ChangePwd(const ChangePwdParam* pChangePwdParam)
{
  if (!StringTool::ComparyNoCase(pChangePwdParam->userName_, userName_.c_str(), userName_.size())
    && userRole_ != PDB_ROLE::ROLE_ADMIN)
  {
    //如果不是管理员，并且不是修改自己的密码
    LOG_INFO("user ({}) change user ({}) password failed, operation denied",
      userName_.c_str(), pChangePwdParam->userName_.c_str());
    return PdbE_OPERATION_DENIED;
  }

  uint32_t pwd32 = StringTool::CRC32(pChangePwdParam->newPwd_.c_str());
  PdbErr_t retVal = pGlbUser->ChangePwd(pChangePwdParam->userName_, pwd32);
  if (retVal == PdbE_OK)
  {
    LOG_INFO("user ({}) change user ({}) password successful",
      userName_.c_str(), pChangePwdParam->userName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}) change user ({}) password failed, err: {}",
      userName_.c_str(), pChangePwdParam->userName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_ChangeRole(const ChangeRoleParam* pChangeRoleParam)
{
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  PdbErr_t retVal =  pGlbUser->ChangeRole(pChangeRoleParam->userName_.c_str(),
    pChangeRoleParam->roleName_.c_str());
  if (retVal == PdbE_OK)
  {
    LOG_INFO("user ({}) change user ({}) role to ({}) successful", userName_.c_str(),
      pChangeRoleParam->userName_.c_str(), pChangeRoleParam->roleName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}) change user ({}) role to ({}) failed, err: {}", userName_.c_str(),
      pChangeRoleParam->userName_.c_str(), pChangeRoleParam->roleName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_DropUser(const DropUserParam* pDropUserParam)
{
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  PdbErr_t retVal = pGlbUser->DropUser(pDropUserParam->userName_);
  if (retVal == PdbE_OK)
  {
    LOG_INFO("user ({}) drop user ({}) successful",
      userName_.c_str(), pDropUserParam->userName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}) drop user ({}) failed, err: {}", userName_.c_str(),
      pDropUserParam->userName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_AlterTable(const CreateTableParam* pCreateTableParam)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  retVal = pGlbTableSet->AlterTable(pCreateTableParam);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) alter table ({}) failed, err: {}",
      userName_.c_str(), pCreateTableParam->tabName_.c_str(), retVal);
  }
  else
  {
    LOG_INFO("user ({}) alter table ({}) successful",
      userName_.c_str(), pCreateTableParam->tabName_.c_str());
  }

  return retVal;
}

PdbErr_t EventHandle::AllocRecvBuf(size_t bufLen)
{
  const size_t bufAlign = 1024;

  if (pRecvBuf_ != nullptr && recvBufLen_ >= bufLen)
    return PdbE_OK;

  if (pRecvBuf_ != nullptr)
    delete[] pRecvBuf_;

  pRecvBuf_ = nullptr;
  recvBufLen_ = 0;

  PdbErr_t retVal = PdbE_OK;

  if (bufLen <= 0)
    return PdbE_INVALID_PARAM;

  bufLen = (bufLen + bufAlign - 1) & (~(bufAlign - 1));

  pRecvBuf_ = new (std::nothrow) uint8_t[bufLen];
  if (pRecvBuf_ == nullptr)
    return PdbE_NOMEM;

  recvBufLen_ = bufLen;

  return PdbE_OK;
}

void EventHandle::FreeRecvBuf()
{
  const int32_t maxRecvBuf = static_cast<int32_t>(PDB_MB_BYTES(1));  // 1M 最大读缓冲

  if (pRecvBuf_ != nullptr && recvBufLen_ > maxRecvBuf)
  {
    delete[] pRecvBuf_;

    pRecvBuf_ = nullptr;
    recvBufLen_ = 0;
  }
}


PdbErr_t EventHandle::AllocSendBuf(size_t bufLen)
{
  const size_t bufAlign = 1024;

  if (pSendBuf_ != nullptr && sendBufLen_ >= bufLen)
    return PdbE_OK;

  if (pSendBuf_ != nullptr)
    delete[] pSendBuf_;

  pSendBuf_ = nullptr;
  sendBufLen_ = 0;

  PdbErr_t retVal = PdbE_OK;

  if (bufLen <= 0)
    return PdbE_INVALID_PARAM;

  bufLen = (bufLen + bufAlign - 1) & (~(bufAlign - 1));

  pSendBuf_ = new (std::nothrow) uint8_t[bufLen];
  if (pSendBuf_ == nullptr)
    return PdbE_NOMEM;

  sendBufLen_ = bufLen;

  return PdbE_OK;
}

void EventHandle::FreeSendBuf()
{
  const int maxSendBuf = (64 * 1024); // 发送缓存超过64K会被释放

  if (pSendBuf_ != nullptr && sendBufLen_ > maxSendBuf)
  {
    delete []pSendBuf_;

    pSendBuf_ = nullptr;
    sendBufLen_ = 0;
  }
}


