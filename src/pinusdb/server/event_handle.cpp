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
#include "query/value_item.h"

#include "db/db_impl.h"
#include "global_variable.h"
#include "util/coding.h"

EventHandle::EventHandle(int socket, const char* pRemoteIp, int remotePort)
{
#ifndef _WIN32
  this->socket_ = socket;
  this->refCnt_ = 0;
#endif

  this->eventState_ = EventState::kRecv;
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
  this->sendLen_ = 0;
}


EventHandle::~EventHandle()
{
  LOG_INFO("disconnect ({}:{})", remoteIp_.c_str(), remotePort_);

#ifndef _WIN32
  close(socket_);
#endif
}

#ifdef _WIN32

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
    {
      if (recvBuf_.size() == 0)
      {
        //没有报文体
        this->eventState_ = EventState::kExec;
      }
      break;
    }

    //接收报文体
    if ((recvBodyLen_ + bytesTransfered) > recvBuf_.size())
    {
      LOG_ERROR("connection ({}:{}) received overflow, received:({}) total:({})",
        remoteIp_.c_str(), remotePort_, (recvBodyLen_ + bytesTransfered), recvBuf_.size());
      retVal = PdbE_PACKET_ERROR;
      break;
    }

    uint8_t* pRecvBuf = (uint8_t*)&(recvBuf_[0]);
    //判断要不要拷贝
    if (recvBodyLen_ != 0 || pRecvBuf != pTmpBuf)
    {
      //需要拷贝
      memcpy((pRecvBuf + recvBodyLen_), pTmpBuf, bytesTransfered);
    }
    recvBodyLen_ += bytesTransfered;

    if (recvBodyLen_ == recvBuf_.size())
    {
      uint32_t tmpCrc32 = StringTool::CRC32(pRecvBuf, recvBodyLen_);
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

    if (sendLen_ + bytesTransfered > sendBuf_.size())
    {
      LOG_ERROR("failed to send packet, total length ({}),  sent length ({}) error",
        sendBuf_.size(), (sendLen_ + bytesTransfered));
      retVal = PdbE_PACKET_ERROR;
      break;
    }

    sendLen_ += bytesTransfered;

    if (sendLen_ == sendBuf_.size())
    {
      this->eventState_ = EventState::kRecv;

      this->recvHeadLen_ = 0;
      this->recvBodyLen_ = 0;
      this->recvBuf_.resize(0);

      this->sendLen_ = 0;
      this->sendBuf_.resize(0);

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
    pWsaBuf->buf = &(recvBuf_[0]);
    pWsaBuf->len = static_cast<ULONG>(recvBuf_.size());
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
    pWsaBuf->buf = (char*)&(sendBuf_[0]);
    pWsaBuf->len = static_cast<ULONG>(sendBuf_.size());
  }
  else
  {
    size_t tmpLen = pWsaBuf->len > (sendBuf_.size() - sendLen_) ? (sendBuf_.size() - sendLen_) : pWsaBuf->len;
    memcpy(pWsaBuf->buf, (&(sendBuf_[0]) + sendLen_), tmpLen);
    pWsaBuf->len = static_cast<ULONG>(tmpLen);
  }

  return true;
}

#else

bool EventHandle::RecvData()
{
  PdbErr_t retVal = PdbE_OK;
  int nread = 0;

  if (eventState_ != EventState::kRecv)
    return false;

  if (this->IsEnd())
    return false;

  std::unique_lock<std::mutex> eventLock(eventMutex_);
  do {
    if (this->eventState_ != EventState::kRecv)
    {
      retVal = PdbE_TASK_STATE_ERROR;
      break;
    }

    if (recvHeadLen_ < ProtoHeader::kProtoHeadLength)
    {
      while (recvHeadLen_ < ProtoHeader::kProtoHeadLength)
      {
        int tmpLen = ProtoHeader::kProtoHeadLength - recvHeadLen_;
        nread = read(socket_, (recvHead_ + recvHeadLen_), tmpLen);

        if (nread <= 0)
        {
          if (nread < 0 && errno != EAGAIN)
          {
            retVal = PdbE_NET_ERROR;
          }
          break;
        }

        recvHeadLen_ += nread;
      }

      if (recvHeadLen_ == ProtoHeader::kProtoHeadLength)
      {
        retVal = DecodeHead();
        if (retVal != PdbE_OK)
        {
          LOG_ERROR("recv socket data, decode protocal head failed, error:({})", retVal);
          break;
        }
      }
    }

    uint8_t* pRecvBuf = (uint8_t*)&(recvBuf_[0]);

    while (recvBodyLen_ < recvBuf_.size())
    {
      int tmpLen = recvBuf_.size() - recvBodyLen_;
      nread = read(socket_, (pRecvBuf + recvBodyLen_), tmpLen);

      if (nread <= 0)
      {
        if (nread < 0 && errno != EAGAIN)
        {
          retVal = PdbE_NET_ERROR;
        }
        break;
      }

      recvBodyLen_ += nread;
    }

    if (recvBodyLen_ == recvBuf_.size())
    {
      uint32_t tmpCrc32 = StringTool::CRC32(pRecvBuf, recvBuf_.size());
      if (tmpCrc32 != dataCrc_)
      {
        LOG_ERROR("recv socket data, protocal body crc error");
        retVal = PdbE_PACKET_ERROR;
        break;
      }

      this->eventState_ = EventState::kExec;
    }

  } while (false);

  if (retVal != PdbE_OK)
  {
    LOG_ERROR("recv socket data failed, connection close");
    this->SetEnd();
    return false;
  }

  return true;
}

bool EventHandle::SendData()
{
  std::unique_lock<std::mutex> eventLock(eventMutex_);
  return _SendData();
}

bool EventHandle::_SendData()
{
  if (eventState_ == EventState::kSend)
  {
    char* pSendBuf = (&(sendBuf_[0]));
    while (sendLen_ < sendBuf_.size())
    {
      int nwrite = write(socket_, (pSendBuf + sendLen_), (sendBuf_.size() - sendLen_));
      if (nwrite <= 0)
      {
        if (nwrite < 0 && errno != EAGAIN)
        {
          this->eventState_ = EventState::kEnd;
          return false;
        }
        break;
      }

      sendLen_ += nwrite;
    }

    if (sendLen_ == sendBuf_.size())
    {
      this->eventState_ = EventState::kRecv;

      this->recvHeadLen_ = 0;
      this->recvBodyLen_ = 0;
      this->recvBuf_.resize(0);

      this->sendLen_ = 0;
      this->sendBuf_.resize(0);

      FreeSendBuf();
    }
  }

  return true;
}

#endif

bool EventHandle::ExecTask()
{
  PdbErr_t retVal = PdbE_OK;

  int32_t successCnt = 0;
  uint32_t repMethodId = METHOD_ERROR_REP;
  std::list<PdbErr_t> insertRet;

  uint32_t fieldCount = 0;
  uint32_t recordCount = 0;

#ifndef _WIN32
  std::unique_lock<std::mutex> eventLock(eventMutex_);
#endif
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
    retVal = ExecQuery(sendBuf_, &fieldCount, &recordCount);
    repMethodId = METHOD_CMD_QUERY_REP;
    break;
  }
  case METHOD_CMD_INSERT_REQ:
  {
    retVal = ExecInsertSql(&successCnt);
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
    EncodeQueryPacket(retVal, fieldCount, recordCount);
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
    sendBuf_.resize(ProtoHeader::kProtoHeadLength);
    ProtoHeader proHdr;
    proHdr.Load(&(sendBuf_[0]));
    proHdr.InitHeader(repMethodId, 0, retVal, 0);
  }

  this->sendLen_ = 0;
  this->recvHeadLen_ = 0;
  this->recvBodyLen_ = 0;
  this->recvBuf_.resize(0);
  this->eventState_ = EventState::kSend;

  //如果接收缓冲大于某个值，则释放
  FreeRecvBuf();

#ifdef _WIN32
  return true;
#else
  return _SendData();
#endif
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

  if (proHdr.GetBodyLen() > PDB_MAX_PACKET_BODY_LEN)
  {
    LOG_ERROR("packet length ({}) error", proHdr.GetBodyLen());
    return PdbE_PACKET_ERROR;
  }
  //分配空间
  recvBuf_.resize(proHdr.GetBodyLen());
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

  *pSqlLen = recvBodyLen_;
  *ppSql = &(recvBuf_[0]);
  return PdbE_OK;
}

PdbErr_t EventHandle::DecodeInsertTable(InsertSql* pInsertSql)
{
  const char* pTmp = &(recvBuf_[0]);
  const char* pBufLimit = pTmp + recvBodyLen_;
  DBVal dbVal;
  uint32_t vType = 0;
  uint32_t v32 = 0;
  uint64_t v64 = 0;

  if (recvBodyLen_ == 0 || recCnt_ == 0 || fieldCnt_ == 0)
  {
    LOG_ERROR("failed to decode insert packet, packet length: ({}), field count: ({}), row count: ({})",
      recvBodyLen_, fieldCnt_, recCnt_);
    return PdbE_PACKET_ERROR;
  }

  pInsertSql->SetFieldCnt(fieldCnt_);
  pInsertSql->SetRecCnt(recCnt_);

  //包括表名和字段名，所以此处为 idx <= fieldCnt_
  for (uint32_t idx = 0; idx <= fieldCnt_ && pTmp < pBufLimit; idx++)
  {
    vType = *pTmp++;
    if (pTmp >= pBufLimit)
      return PdbE_PACKET_ERROR;

    if (vType != PDB_VALUE_TYPE::VAL_STRING)
      return PdbE_PACKET_ERROR;

    pTmp = Coding::VarintDecode32(pTmp, pBufLimit, &v32);
    if (pTmp == nullptr || (pTmp + v32) > pBufLimit)
      return PdbE_PACKET_ERROR;

    if (idx == 0)
      pInsertSql->SetTableName((const char*)pTmp, v32);
    else
      pInsertSql->AppendFieldName((const char*)pTmp, v32);

    pTmp += v32;
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

    pInsertSql->AppendVal(&dbVal);
  }

  if (pTmp != pBufLimit)
    return PdbE_PACKET_ERROR;

  if (!pInsertSql->Valid())
    return PdbE_PACKET_ERROR;

  return PdbE_OK;
}


PdbErr_t EventHandle::DecodeInsertSql(InsertSql* pInsertSql, Arena* pArena)
{
  PdbErr_t retVal = PdbE_OK;
  const char* pSql = nullptr;
  size_t sqlLen = 0;
  TableInfo nonTab;
  retVal = DecodeSqlPacket(&pSql, &sqlLen);
  if (retVal != PdbE_OK)
    return retVal;

  SQLParser sqlParser;
  Tokenize::RunParser(pArena, &sqlParser, pSql, sqlLen);
  if (sqlParser.GetError())
    return PdbE_SQL_ERROR;

  if (sqlParser.GetCmdType() != SQLParser::CmdType::CT_Insert)
    return PdbE_SQL_ERROR;

  const InsertParam* pInsertParam = sqlParser.GetInsertParam();
  pInsertSql->SetTableName(sqlParser.GetTableName());
  const std::vector<TargetItem>* pColVec = pInsertParam->pTagList_->GetTargetList();
  pInsertSql->SetFieldCnt(pColVec->size());

  DBVal colNameVal;
  for (auto colIt = pColVec->begin(); colIt != pColVec->end(); colIt++)
  {
    if ((*colIt).first->GetValueType() != TK_ID)
      return PdbE_SQL_ERROR;

    colNameVal = (*colIt).first->GetValue();
    if (!DBVAL_IS_STRING(&colNameVal))
      return PdbE_SQL_ERROR;

    pInsertSql->AppendFieldName(DBVAL_GET_STRING(&colNameVal), DBVAL_GET_LEN(&colNameVal));
  }

  int64_t nowMillis = DateTime::NowMilliseconds();
  ValueItem* pTmpValItem = nullptr;
  int valType = 0;
  DBVal val;
  const std::vector<ExprValueList*>& recList = pInsertParam->pValRecList_->GetRecList();
  pInsertSql->SetRecCnt(recList.size());
  for (auto recIt = recList.begin(); recIt != recList.end(); recIt++)
  {
    const std::vector<ExprValue*>* pValVec  = (*recIt)->GetValueList();
    if (pValVec->size() != pColVec->size())
      return PdbE_SQL_ERROR;

    for (auto valIt = pValVec->begin(); valIt != pValVec->end(); valIt++)
    {
      valType = (*valIt)->GetValueType();
      val = (*valIt)->GetValue();
      do {
        if (valType == TK_TRUE
          || valType == TK_FALSE
          || valType == TK_INTEGER
          || valType == TK_DOUBLE
          || valType == TK_STRING
          || valType == TK_BLOB)
        {
          retVal = pInsertSql->AppendVal(&val);
        }
        else
        {
          pTmpValItem = BuildGeneralValueItem(&nonTab, (*valIt), nowMillis);
          if (pTmpValItem == nullptr)
          {
            retVal = PdbE_SQL_ERROR;
            break;
          }

          if (!pTmpValItem->IsValid())
          {
            retVal = PdbE_SQL_ERROR;
            break;
          }

          if (!pTmpValItem->IsConstValue())
          {
            retVal = PdbE_SQL_ERROR;
            break;
          }

          if (pTmpValItem->GetValue(nullptr, &val) != PdbE_OK)
          {
            retVal = PdbE_SQL_ERROR;
            break;
          }

          pInsertSql->AppendVal(&val);
        }
      } while (false);

      if (pTmpValItem != nullptr)
      {
        delete pTmpValItem;
        pTmpValItem = nullptr;
      }
    }
  }

  return PdbE_OK;
}

PdbErr_t EventHandle::EncodeQueryPacket(PdbErr_t retVal, uint32_t fieldCnt, uint32_t recordCnt)
{
  if (retVal == PdbE_OK)
  {
    char* pSendBuf = (&(sendBuf_[0]));
    size_t bodyLen = sendBuf_.size() - ProtoHeader::kProtoHeadLength;

    uint32_t bodyCrc = StringTool::CRC32((pSendBuf + ProtoHeader::kProtoHeadLength), bodyLen);

    ProtoHeader proHdr;
    proHdr.Load(pSendBuf);
    proHdr.InitHeader(METHOD_CMD_QUERY_REP, static_cast<int32_t>(bodyLen), PdbE_OK, bodyCrc);
    proHdr.SetRecordCnt(recordCnt);
    proHdr.SetFieldCnt(fieldCnt);
    proHdr.UpdateHeadCrc();
  }
  else
  {
    sendBuf_.resize(ProtoHeader::kProtoHeadLength);
    ProtoHeader proHdr;
    proHdr.Load((&(sendBuf_[0])));
    proHdr.InitHeader(METHOD_CMD_QUERY_REP, 0, retVal, 0);
  }

  return PdbE_OK;
}

PdbErr_t EventHandle::EncodeInsertPacket(PdbErr_t retVal, int32_t successCnt)
{
  ProtoHeader proHdr;
  sendBuf_.resize(ProtoHeader::kProtoHeadLength);
  proHdr.Load(&(sendBuf_[0]));
  proHdr.InitHeader(METHOD_CMD_INSERT_REP, 0, retVal, 0);
  proHdr.SetRecordCnt(successCnt);
  proHdr.SetErrPos(0);
  proHdr.UpdateHeadCrc();

  return PdbE_OK;
}

PdbErr_t EventHandle::EncodeInsertTablePacket(PdbErr_t retVal, const std::list<PdbErr_t>& insertRet)
{
  ProtoHeader proHdr;
  
  if (retVal == PdbE_OK)
  {
    sendBuf_.resize(ProtoHeader::kProtoHeadLength);
    proHdr.Load(&(sendBuf_[0]));
    proHdr.InitHeader(METHOD_CMD_INSERT_TABLE_REP, 0, PdbE_OK, 0);
    proHdr.SetRecordCnt(static_cast<uint32_t>(insertRet.size()));
    proHdr.UpdateHeadCrc();
    return PdbE_OK;
  }

  do {
    if (retVal != PdbE_INSERT_PART_ERROR)
      break;

    size_t tmpLen = insertRet.size() * 9;
    sendBuf_.resize((ProtoHeader::kProtoHeadLength + tmpLen));

    size_t bodyLen = 0;
    uint8_t* pBodyBuf = (uint8_t*)&(sendBuf_[0]) + ProtoHeader::kProtoHeadLength;
    uint8_t* pTmpBody = pBodyBuf;
    for (auto retIt = insertRet.begin(); retIt != insertRet.end(); retIt++)
    {
      pTmpBody = Coding::VarintEncode32(pTmpBody, Coding::ZigzagEncode32(*retIt));
    }

    bodyLen = pTmpBody - pBodyBuf;
    uint32_t bodyCrc = StringTool::CRC32(pBodyBuf, bodyLen);

    proHdr.Load(&(sendBuf_[0]));
    proHdr.InitHeader(METHOD_CMD_INSERT_TABLE_REP, static_cast<int32_t>(bodyLen), PdbE_INSERT_PART_ERROR, bodyCrc);
    proHdr.SetRecordCnt(static_cast<uint32_t>(insertRet.size()));
    proHdr.UpdateHeadCrc();

    sendBuf_.resize(bodyLen + ProtoHeader::kProtoHeadLength);
    return PdbE_OK;
  } while (false);

  sendBuf_.resize(ProtoHeader::kProtoHeadLength);
  proHdr.Load(&(sendBuf_[0]));
  proHdr.InitHeader(METHOD_CMD_INSERT_TABLE_REP, 0, retVal, 0);
  proHdr.UpdateHeadCrc();
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

  const char* pTmp = &(recvBuf_[0]);

  uint32_t pwd = Coding::FixedDecode32((pTmp + PDB_USER_NAME_LEN));
  int32_t role = 0;
  size_t nameLen = 0;
  while (pTmp[nameLen] != '\0')
  {
    if (nameLen >= PDB_USER_NAME_LEN)
      return PdbE_INVALID_USER_NAME;

    nameLen++;
  }

  retVal = pGlbUser->Login(pTmp, pwd, &role);
  if (retVal != PdbE_OK)
  {
    LOG_DEBUG("login failed ({}:{}), ret: ({})", 
      remoteIp_.c_str(), remotePort_, retVal);
    return retVal;
  }

  userRole_ = role;
  userName_ = (char*)pTmp;

  pGlbServerConnction->ConnectionLogin((uint64_t)this, pTmp, role);
  LOG_INFO("login successed, client:({}:{}), user:({}), role:({})", 
    remoteIp_.c_str(), remotePort_, userName_, userRole_);
  return PdbE_OK;
}


PdbErr_t EventHandle::ExecQuery(std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  PdbErr_t retVal = PdbE_OK;
  Arena arena;
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

  LOG_DEBUG(pSql);

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

  retVal = pGlbTableSet->ExecuteQuery(&sqlParser, userRole_, resultData, pFieldCnt, pRecordCnt);
  return retVal;
}

PdbErr_t EventHandle::ExecInsertSql(int32_t* pSuccessCnt)
{
  PdbErr_t retVal = PdbE_OK;
  Arena arena;
  InsertSql insertSql;
  std::list<PdbErr_t> resultList;
  
  *pSuccessCnt = 0;

  if (userRole_ != PDB_ROLE::ROLE_ADMIN
    && userRole_ != PDB_ROLE::ROLE_WRITE_ONLY
    && userRole_ != PDB_ROLE::ROLE_READ_WRITE)
  {
    return PdbE_OPERATION_DENIED; //没有权限
  }

  retVal = DecodeInsertSql(&insertSql, &arena);
  if (retVal != PdbE_OK)
    return retVal;

  retVal = pGlbTableSet->Insert(&insertSql, true, resultList);
  *pSuccessCnt = static_cast<int32_t>(resultList.size());
  return retVal;
}

PdbErr_t EventHandle::ExecInsertTable(std::list<PdbErr_t>& resultList)
{
  PdbErr_t retVal = PdbE_OK;
  InsertSql insertSql;

  if (userRole_ != PDB_ROLE::ROLE_ADMIN
    && userRole_ != PDB_ROLE::ROLE_WRITE_ONLY
    && userRole_ != PDB_ROLE::ROLE_READ_WRITE)
  {
    return PdbE_OPERATION_DENIED; //没有权限
  }

  retVal = DecodeInsertTable(&insertSql);
  if (retVal != PdbE_OK)
    return retVal;

  return pGlbTableSet->Insert(&insertSql, false, resultList);
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
      retVal = _DropTable(sqlParser.GetTableName());
      break;
    case SQLParser::CmdType::CT_Delete: // 需要管理员权限
      retVal = _DeleteDev(sqlParser.GetTableName(), sqlParser.GetDeleteParam());
      break;
    case SQLParser::CmdType::CT_AttachTable: // 需要管理员权限
      retVal = _AttachTable(sqlParser.GetTableName());
      break;
    case SQLParser::CmdType::CT_DetachTable:
      retVal = _DetachTable(sqlParser.GetTableName());
      break;
    case SQLParser::CmdType::CT_AttachFile:
      retVal = _AttachFile(sqlParser.GetTableName(), sqlParser.GetDataFileParam());
      break;
    case SQLParser::CmdType::CT_DetachFile:
      retVal = _DetachFile(sqlParser.GetTableName(), sqlParser.GetDataFileParam());
      break;
    case SQLParser::CmdType::CT_DropFile:
      retVal = _DropDataFile(sqlParser.GetTableName(), sqlParser.GetDataFileParam());
      break;
    case SQLParser::CmdType::CT_CreateTable: // 必须要Admin权限
      retVal = _CreateTable(sqlParser.GetTableName(), sqlParser.GetCreateTableParam());
      break;
    case SQLParser::CmdType::CT_AddUser: // 必须要Admin权限
      retVal = _AddUser(sqlParser.GetUserParam());
      break;
    case SQLParser::CmdType::CT_ChangePwd:
      retVal = _ChangePwd(sqlParser.GetUserParam());
      break;
    case SQLParser::CmdType::CT_ChangeRole: // 必须要Admin权限
      retVal = _ChangeRole(sqlParser.GetUserParam());
      break;
    case SQLParser::CmdType::CT_DropUser:
      retVal = _DropUser(sqlParser.GetUserParam());
      break;
    case SQLParser::CmdType::CT_AlterTable:
      retVal = _AlterTable(sqlParser.GetTableName(), sqlParser.GetCreateTableParam());
      break;
    default:
    {
      retVal = PdbE_SQL_ERROR;
      break;
    }
  }

  return retVal;
}

PdbErr_t EventHandle::_DropTable(const char* pTabName)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  retVal = pGlbTableSet->DropTable(pTabName);
  if (retVal == PdbE_OK)
  {
    LOG_INFO("user ({}), drop table ({}) successed",
      userName_.c_str(), pTabName);
  }
  else
  {
    LOG_INFO("user ({}), drop table ({}) failed, err: {}", 
      userName_.c_str(), pTabName, retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_DeleteDev(const char* pTabName, const DeleteParam* pDeleteParam)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  if (pTabName == nullptr || pDeleteParam == nullptr)
    return PdbE_INVALID_PARAM;

  retVal = pGlbTableSet->DeleteDev(pTabName, pDeleteParam);
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

PdbErr_t EventHandle::_AttachTable(const char* pTabName)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  retVal = pGlbTableSet->AttachTable(pTabName);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) attach table ({}) successful",
      userName_.c_str(), pTabName);
  }
  else
  {
    LOG_INFO("user ({}) attach table ({}) failed, err: {}",
      userName_.c_str(), pTabName, retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_DetachTable(const char* pTabName)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  retVal = pGlbTableSet->DetachTable(pTabName);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) detach table ({}) successful",
      userName_.c_str(), pTabName);
  }
  else
  {
    LOG_INFO("user ({}) detach table ({}) failed, err: {}",
      userName_.c_str(), pTabName, retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_AttachFile(const char* pTabName, const DataFileParam* pDataFile)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fileType = PDB_PART_TYPE_NORMAL_VAL;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  if (pTabName == nullptr || pDataFile == nullptr)
    return PdbE_INVALID_PARAM;

  if (StringTool::ComparyNoCase(pDataFile->fileType_.c_str(), PDB_PART_TYPE_NORMAL_STR))
    fileType = PDB_PART_TYPE_NORMAL_VAL;
  else if (StringTool::ComparyNoCase(pDataFile->fileType_.c_str(), PDB_PART_TYPE_COMPRESS_STR))
    fileType = PDB_PART_TYPE_COMPRESS_VAL;
  else
  {
    LOG_INFO("user ({}) attach data file ({}) for table ({}) failed, unknown data file type",
      userName_.c_str(), pDataFile->dateStr_.c_str(), pTabName);
    return PdbE_INVALID_PARAM;
  }

  retVal = pGlbTableSet->AttachFile(pTabName,
    pDataFile->dateStr_.c_str(), fileType);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) attach data file ({}) for table ({}) failed, err: {}",
      userName_.c_str(), pDataFile->dateStr_.c_str(), pTabName, retVal);
  }
  else
  {
    LOG_INFO("user ({}) attach data file ({}) for table ({}) successful",
      userName_.c_str(), pDataFile->dateStr_.c_str(), pTabName);
  }

  return retVal;
}

PdbErr_t EventHandle::_DetachFile(const char* pTabName, const DataFileParam* pDataFile)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t partCode = 0;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  if (pTabName == nullptr || pDataFile == nullptr)
    return PdbE_INVALID_PARAM;

  if (!DateTime::ParseDate(pDataFile->dateStr_.c_str(),
    pDataFile->dateStr_.size(), &partCode))
  {
    LOG_INFO("user ({}) detach data file ({}) for table ({}) failed, invalid date param",
      userName_.c_str(), pDataFile->dateStr_.c_str(), pTabName);
    return PdbE_INVALID_PARAM;
  }

  retVal = pGlbTableSet->DetachFile(pTabName, partCode);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) detach data file ({}) for table ({}) failed, err: {}",
      userName_.c_str(), pDataFile->dateStr_.c_str(), pTabName, retVal);
  }
  else
  {
    LOG_INFO("user ({}) detach data file ({}) for table ({}) successful",
      userName_.c_str(), pDataFile->dateStr_.c_str(), pTabName);
  }

  return retVal;
}

PdbErr_t EventHandle::_DropDataFile(const char* pTabName, const DataFileParam* pDataFile)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t partCode = 0;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  if (pTabName == nullptr || pDataFile == nullptr)
    return PdbE_INVALID_PARAM;

  if (!DateTime::ParseDate(pDataFile->dateStr_.c_str(),
    pDataFile->dateStr_.size(), &partCode))
  {
    LOG_INFO("user ({}) drop data file ({}) for table ({}) failed, invalid date param",
      userName_.c_str(), pDataFile->dateStr_.c_str(),
      pDataFile->dateStr_.c_str(), pTabName);
    return PdbE_INVALID_PARAM;
  }

  retVal = pGlbTableSet->DropFile(pTabName, partCode);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) drop data file ({}) for table ({}) failed, err: {}",
      userName_.c_str(), pDataFile->dateStr_.c_str(), pTabName,  retVal);
  }
  else
  {
    LOG_INFO("user ({}) drop data file ({}) for table ({}) successful",
      userName_.c_str(), pDataFile->dateStr_.c_str(), pTabName);
  }

  return retVal;
}

PdbErr_t EventHandle::_CreateTable(const char* pTabName, const CreateTableParam* pCreateTableParam)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  if (pTabName == nullptr || pCreateTableParam == nullptr)
    return PdbE_INVALID_PARAM;

  retVal = pGlbTableSet->CreateTable(pTabName, pCreateTableParam->pColList_);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) create table ({}) failed, err: {}",
      userName_.c_str(), pTabName, retVal);
  }
  else
  {
    LOG_INFO("user ({}) create table ({}) successful",
      userName_.c_str(), pTabName);
  }

  return retVal;
}

PdbErr_t EventHandle::_CreateTable(const SQLParser* pParser)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  const char* pTableName = pParser->GetTableName();
  const CreateTableParam* pParam = pParser->GetCreateTableParam();
  retVal = pGlbTableSet->CreateTable(pTableName, pParam->pColList_);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) create table ({}) failed, err: {}",
      userName_.c_str(), pTableName, retVal);
  }
  else
  {
    LOG_INFO("user ({}) create table ({}) successful",
      userName_.c_str(), pTableName);
  }

  return retVal;
}

PdbErr_t EventHandle::_AddUser(const UserParam* pUserParam)
{
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED; //判断是否有添加用户的权限

  if (pUserParam == nullptr)
    return PdbE_INVALID_PARAM;

  uint32_t pwd32 = StringTool::CRC32(pUserParam->pwd_.c_str());
  PdbErr_t retVal = pGlbUser->AddUser(pUserParam->userName_.c_str(),
    pwd32, "readwrite");
  if (retVal == PdbE_OK)
  {
    LOG_INFO("user ({}) add user ({}:readwrite) successful",
      userName_.c_str(), pUserParam->userName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}) add user ({}:readwrite) failed, err: {}",
      userName_.c_str(), pUserParam->userName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_ChangePwd(const UserParam* pUserParam)
{
  if (pUserParam == nullptr)
    return PdbE_INVALID_PARAM;

  if (!StringTool::ComparyNoCase(pUserParam->userName_, userName_.c_str(), userName_.size())
    && userRole_ != PDB_ROLE::ROLE_ADMIN)
  {
    //如果不是管理员，并且不是修改自己的密码
    LOG_INFO("user ({}) change user ({}) password failed, operation denied",
      userName_.c_str(), pUserParam->userName_.c_str());
    return PdbE_OPERATION_DENIED;
  }

  uint32_t pwd32 = StringTool::CRC32(pUserParam->pwd_.c_str());
  PdbErr_t retVal = pGlbUser->ChangePwd(pUserParam->userName_, pwd32);
  if (retVal == PdbE_OK)
  {
    LOG_INFO("user ({}) change user ({}) password successful",
      userName_.c_str(), pUserParam->userName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}) change user ({}) password failed, err: {}",
      userName_.c_str(), pUserParam->userName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_ChangeRole(const UserParam* pUserParam)
{
  if (pUserParam == nullptr)
    return PdbE_INVALID_PARAM;

  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  PdbErr_t retVal =  pGlbUser->ChangeRole(pUserParam->userName_.c_str(),
    pUserParam->roleName_.c_str());
  if (retVal == PdbE_OK)
  {
    LOG_INFO("user ({}) change user ({}) role to ({}) successful", userName_.c_str(),
      pUserParam->userName_.c_str(), pUserParam->roleName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}) change user ({}) role to ({}) failed, err: {}", userName_.c_str(),
      pUserParam->userName_.c_str(), pUserParam->roleName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_DropUser(const UserParam* pUserParam)
{
  if (pUserParam == nullptr)
    return PdbE_INVALID_PARAM;

  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  PdbErr_t retVal = pGlbUser->DropUser(pUserParam->userName_);
  if (retVal == PdbE_OK)
  {
    LOG_INFO("user ({}) drop user ({}) successful",
      userName_.c_str(), pUserParam->userName_.c_str());
  }
  else
  {
    LOG_INFO("user ({}) drop user ({}) failed, err: {}", userName_.c_str(),
      pUserParam->userName_.c_str(), retVal);
  }

  return retVal;
}

PdbErr_t EventHandle::_AlterTable(const char* pTabName, const CreateTableParam* pCreateTableParam)
{
  PdbErr_t retVal = PdbE_OK;
  if (userRole_ != PDB_ROLE::ROLE_ADMIN)
    return PdbE_OPERATION_DENIED;

  if (pTabName == nullptr || pCreateTableParam == nullptr)
    return PdbE_INVALID_PARAM;

  retVal = pGlbTableSet->AlterTable(pTabName, pCreateTableParam->pColList_);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("user ({}) alter table ({}) failed, err: {}",
      userName_.c_str(), pTabName, retVal);
  }
  else
  {
    LOG_INFO("user ({}) alter table ({}) successful", userName_.c_str(), pTabName);
  }

  return retVal;
}

void EventHandle::FreeRecvBuf()
{
  std::string tmp;
  if (recvBuf_.capacity() > PDB_KB_BYTES(32))
  {
    recvBuf_.swap(tmp);
  }
}

void EventHandle::FreeSendBuf()
{
  std::string tmp;
  if (sendBuf_.capacity() > PDB_KB_BYTES(32))
  {
    sendBuf_.swap(tmp);
  }
}


