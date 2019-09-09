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

#include "server/proto_header.h"
#include "util/string_tool.h"

typedef struct _ProtoHdrInfo
{
  uint32_t version_;
  uint32_t method_;
  uint32_t bodyLen_;
  uint32_t retVal_;
  uint32_t fieldCnt_;
  uint32_t recCnt_;
  uint32_t errPos_;
  uint32_t padding_[7];
  uint32_t bodyCrc_;
  uint32_t headCrc_;
}ProtoHdrInfo;

ProtoHeader::ProtoHeader()
{
  this->pHead_ = nullptr;
}
ProtoHeader::~ProtoHeader()
{
}

void ProtoHeader::Load(uint8_t* pHead)
{
  this->pHead_ = pHead;
}

uint32_t ProtoHeader::GetVersion()
{
  if (this->pHead_ == nullptr)
    return 0;

  const ProtoHdrInfo* pHdrInfo = (const ProtoHdrInfo*)pHead_;
  return pHdrInfo->version_;
}
uint32_t ProtoHeader::GetMethod()
{
  if (this->pHead_ == nullptr)
    return 0;

  const ProtoHdrInfo* pHdrInfo = (const ProtoHdrInfo*)pHead_;
  return pHdrInfo->method_;
}
uint32_t ProtoHeader::GetBodyLen()
{
  if (this->pHead_ == nullptr)
    return 0;

  const ProtoHdrInfo* pHdrInfo = (const ProtoHdrInfo*)pHead_;
  return pHdrInfo->bodyLen_;
}
uint32_t ProtoHeader::GetReturnVal()
{
  if (this->pHead_ == nullptr)
    return 0;

  const ProtoHdrInfo* pHdrInfo = (const ProtoHdrInfo*)pHead_;
  return pHdrInfo->retVal_;
}
uint32_t ProtoHeader::GetFieldCnt()
{
  if (this->pHead_ == nullptr)
    return 0;

  const ProtoHdrInfo* pHdrInfo = (const ProtoHdrInfo*)pHead_;
  return pHdrInfo->fieldCnt_;
}
uint32_t ProtoHeader::GetRecordCnt()
{
  if (this->pHead_ == nullptr)
    return 0;

  const ProtoHdrInfo* pHdrInfo = (const ProtoHdrInfo*)pHead_;
  return pHdrInfo->recCnt_;
}
uint32_t ProtoHeader::GetErrPos()
{
  if (this->pHead_ == nullptr)
    return 0;

  const ProtoHdrInfo* pHdrInfo = (const ProtoHdrInfo*)pHead_;
  return pHdrInfo->errPos_;
}

uint32_t ProtoHeader::GetBodyCrc()
{
  if (this->pHead_ == nullptr)
    return 0;

  const ProtoHdrInfo* pHdrInfo = (const ProtoHdrInfo*)pHead_;
  return pHdrInfo->bodyCrc_;
}
uint32_t ProtoHeader::GetHeadCrc()
{
  if (this->pHead_ == nullptr)
    return 0;

  const ProtoHdrInfo* pHdrInfo = (const ProtoHdrInfo*)pHead_;
  return pHdrInfo->headCrc_;
}

void ProtoHeader::SetVersion(uint32_t version)
{
  if (this->pHead_ != nullptr)
  {
    ProtoHdrInfo* pHdrInfo = (ProtoHdrInfo*)pHead_;
    pHdrInfo->version_ = version;
  }
}
void ProtoHeader::SetMethod(uint32_t methodId)
{
  if (this->pHead_ != nullptr)
  {
    ProtoHdrInfo* pHdrInfo = (ProtoHdrInfo*)pHead_;
    pHdrInfo->method_ = methodId;
  }
}
void ProtoHeader::SetBodyLen(uint32_t bodyLen)
{
  if (this->pHead_ != nullptr)
  {
    ProtoHdrInfo* pHdrInfo = (ProtoHdrInfo*)pHead_;
    pHdrInfo->bodyLen_ = bodyLen;
  }
}
void ProtoHeader::SetReturnVal(uint32_t retVal)
{
  if (this->pHead_ != nullptr)
  {
    ProtoHdrInfo* pHdrInfo = (ProtoHdrInfo*)pHead_;
    pHdrInfo->retVal_ = retVal;
  }
}
void ProtoHeader::SetFieldCnt(uint32_t fieldCnt)
{
  if (this->pHead_ != nullptr)
  {
    ProtoHdrInfo* pHdrInfo = (ProtoHdrInfo*)pHead_;
    pHdrInfo->fieldCnt_ = fieldCnt;
  }
}
void ProtoHeader::SetRecordCnt(uint32_t recordCnt)
{
  if (this->pHead_ != nullptr)
  {
    ProtoHdrInfo* pHdrInfo = (ProtoHdrInfo*)pHead_;
    pHdrInfo->recCnt_ = recordCnt;
  }
}

void ProtoHeader::SetErrPos(uint32_t errPos)
{
  if (this->pHead_ != nullptr)
  {
    ProtoHdrInfo* pHdrInfo = (ProtoHdrInfo*)pHead_;
    pHdrInfo->errPos_ = errPos;
  }
}

void ProtoHeader::SetBodyCrc(uint32_t bodyCrc)
{
  if (this->pHead_ != nullptr)
  {
    ProtoHdrInfo* pHdrInfo = (ProtoHdrInfo*)pHead_;
    pHdrInfo->bodyCrc_ = bodyCrc;
  }
}
void ProtoHeader::UpdateHeadCrc()
{
  if (this->pHead_ != nullptr)
  {
    ProtoHdrInfo* pHdrInfo = (ProtoHdrInfo*)pHead_;
    uint32_t headCrc = StringTool::CRC32(pHead_, kProtoHeadCalcCrcLen);
    pHdrInfo->headCrc_ = headCrc;
  }
}


void ProtoHeader::InitHeader(int32_t methodId, int32_t bodyLen, int32_t retVal, int32_t bodyCrc)
{
  if (this->pHead_ != nullptr)
  {
    memset(this->pHead_, 0, kProtoHeadLength);
    SetVersion(kProtoVersion);
    SetMethod(methodId);
    SetBodyLen(bodyLen);
    SetReturnVal(retVal);
    SetBodyCrc(bodyCrc);

    //º∆À„HeadCrc
    UpdateHeadCrc();
  }
}
