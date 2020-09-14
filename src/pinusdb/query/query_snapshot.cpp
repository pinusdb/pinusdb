/*
* Copyright (c) 2020 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
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

#include "query/query_snapshot.h"
#include "server/proto_header.h"
#include "util/coding.h"
#include <algorithm>
#include <string>

class TmpRecord
{
public:
  TmpRecord()
  {
    devId_ = 0;
  }

  int64_t devId_;
  std::string recData_;
};

bool TmpRecordComp(const TmpRecord* pObjA, const TmpRecord* pObjB)
{
  return pObjA->devId_ < pObjB->devId_;
}

QuerySnapshot::QuerySnapshot()
{
  this->queryOffset_ = 0;
  this->queryRecord_ = PDB_QUERY_DEFAULT_COUNT;
  this->maxDevId_ = 0;
}

QuerySnapshot::~QuerySnapshot()
{
  for (auto recIt = recVec_.begin(); recIt != recVec_.end(); recIt++)
  {
    delete (*recIt);
  }

  for (auto fieldIt = fieldVec_.begin(); fieldIt != fieldVec_.end(); fieldIt++)
  {
    delete (*fieldIt);
  }
}

PdbErr_t QuerySnapshot::AppendSingle(const DBVal* pVals, size_t valCnt, bool* pIsAdded)
{
  bool resultVal = true;
  PdbErr_t retVal = PdbE_OK;
  DBVal tmpVal;

  retVal = this->condiFilter_.RunCondition(pVals, valCnt, resultVal);
  if (retVal != PdbE_OK)
    return retVal;

  if (pIsAdded != nullptr)
    *pIsAdded = false;

  std::string recData;
  recData.reserve(256);

  if (resultVal)
  {
    for (auto fieldIt = fieldVec_.begin(); fieldIt != fieldVec_.end(); fieldIt++)
    {
      retVal = (*fieldIt)->GetValue(pVals, &tmpVal);
      if (retVal != PdbE_OK)
        return retVal;

      PushBackToResult(recData, tmpVal);
    }

    TmpRecord* pNewRecord = new TmpRecord();
    pNewRecord->recData_.swap(recData);
    pNewRecord->devId_ = DBVAL_ELE_GET_INT64(pVals, PDB_DEVID_INDEX);
    recVec_.push_back(pNewRecord);
    if (maxDevId_ < pNewRecord->devId_)
    {
      maxDevId_ = pNewRecord->devId_;
    }

    if (pIsAdded != nullptr)
      *pIsAdded = true;
  }

  return PdbE_OK;
}

PdbErr_t QuerySnapshot::AppendArray(BlockValues& blockValues, bool* pIsAdded)
{
  PdbErr_t retVal = PdbE_OK;
  size_t columnSize = blockValues.GetColumnSize();
  std::vector<DBVal> valVec;
  valVec.reserve(columnSize);
  size_t resultSize = blockValues.GetResultSize();
  if (resultSize > 0)
  {
    retVal = blockValues.GetRecordValue(0, valVec);
    if (retVal != PdbE_OK)
      return retVal;

    return AppendSingle(valVec.data(), columnSize, pIsAdded);
  }

  if (pIsAdded != nullptr)
    *pIsAdded = false;

  return PdbE_OK;
}

bool QuerySnapshot::GetIsFullFlag() const
{
  return false;
}

PdbErr_t QuerySnapshot::GetResult(std::string& dataBuf, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  std::sort(recVec_.begin(), recVec_.end(), TmpRecordComp);
  size_t edPos = queryOffset_ + queryRecord_;
  if (edPos > recVec_.size())
    edPos = recVec_.size();

  if (pFieldCnt != nullptr)
  {
    *pFieldCnt = static_cast<uint32_t>(fieldVec_.size());
  }

  if (pRecordCnt != nullptr)
  {
    if (edPos > queryOffset_)
    {
      *pRecordCnt = static_cast<uint32_t>(edPos - queryOffset_);
    }
    else
    {
      *pRecordCnt = 0;
    }
  }

  dataBuf.resize(256 * 1024);
  dataBuf.resize(ProtoHeader::kProtoHeadLength);
  tabInfo_.Serialize(dataBuf);
  for (size_t pos = queryOffset_; pos < edPos; pos++)
  {
    dataBuf.append(recVec_[pos]->recData_);
  }

  return PdbE_OK;
}


void QuerySnapshot::GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const
{
  condiFilter_.GetDevIdRange(pMinDevId, pMaxDevId);
}

void QuerySnapshot::GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const
{
  condiFilter_.GetTstampRange(pMinTstamp, pMaxTstamp);
}

bool QuerySnapshot::FilterDevId(int64_t devId) const
{
  return condiFilter_.FilterDevId(devId);
}

bool QuerySnapshot::IsEmptySet() const
{
  return condiFilter_.AlwaysFalse();
}

size_t QuerySnapshot::GetQueryOffset() const
{
  return queryOffset_;
}

size_t QuerySnapshot::GetQueryRecord() const
{
  return queryRecord_;
}


//snapshot สนำร
int64_t QuerySnapshot::GetMaxDevId() const
{
  if (recVec_.size() > (queryOffset_ + queryRecord_))
  {
    return maxDevId_;
  }

  return INT64_MAX;
}

void QuerySnapshot::GetUseFields(std::unordered_set<size_t>& fieldSet) const
{
  condiFilter_.GetUseFields(fieldSet);
  for (size_t idx = 0; idx < fieldVec_.size(); idx++)
  {
    fieldVec_[idx]->GetUseFields(fieldSet);
  }
}

PdbErr_t QuerySnapshot::BuildQuery(const QueryParam* pQueryParam, const TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  size_t fieldPos = 0;
  DBVal nameVal;
  std::string fieldName;
  int64_t nowMicroseconds = DateTime::NowMicrosecond();
  if (pQueryParam->pTagList_ == nullptr)
    return PdbE_INVALID_PARAM;

  if (pQueryParam->pLimit_ != nullptr)
  {
    retVal = pQueryParam->pLimit_->Valid();
    if (retVal != PdbE_OK)
      return retVal;

    queryOffset_ = pQueryParam->pLimit_->GetOffset();
    queryRecord_ = pQueryParam->pLimit_->GetQueryCnt();
  }

  if (pQueryParam->pWhere_ != nullptr)
  {
    retVal = condiFilter_.BuildCondition(pTabInfo, pQueryParam->pWhere_, nowMicroseconds);
    if (retVal != PdbE_OK)
      return retVal;
  }

  ValueItem* pTagItem = nullptr;
  const std::vector<TargetItem>* pTagItemList = pQueryParam->pTagList_->GetTargetList();
  for (auto tagIt = pTagItemList->begin(); tagIt != pTagItemList->end(); tagIt++)
  {
    if (tagIt->first->GetValueType() == TK_STAR)
    {
      size_t fieldCnt = pTabInfo->GetFieldCnt();
      for (size_t pos = 0; pos < fieldCnt; pos++)
      {
        pTabInfo->GetFieldInfo(pos, &fieldType);

        retVal = tabInfo_.AddField(pTabInfo->GetFieldName(pos), fieldType);
        if (retVal != PdbE_OK)
          return retVal;

        fieldVec_.push_back(CreateFieldValue(fieldType, pos));
      }
    }
    else if (tagIt->first->GetValueType() == TK_STRING)
    {
      nameVal = tagIt->first->GetValue();
      if (!DBVAL_IS_STRING(&nameVal))
        return PdbE_SQL_ERROR;

      pTagItem = nullptr;
      do {
        if (pTabInfo == nullptr)
          break;

        fieldName = std::string(DBVAL_GET_STRING(&nameVal), DBVAL_GET_LEN(&nameVal));
        retVal = pTabInfo->GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
        if (retVal != PdbE_OK)
          break;

        pTagItem = CreateFieldValue(fieldType, fieldPos);
      } while (false);

      if (pTagItem == nullptr)
      {
        pTagItem = new ConstValue(nameVal);
      }

      fieldVec_.push_back(pTagItem);
      retVal = tabInfo_.AddField(tagIt->second.c_str(), pTagItem->GetValueType());
      if (retVal != PdbE_OK)
        return PdbE_SQL_ERROR;
    }
    else
    {
      pTagItem = BuildGeneralValueItem(pTabInfo, tagIt->first, nowMicroseconds);
      if (pTagItem == nullptr)
        return PdbE_SQL_ERROR;

      fieldVec_.push_back(pTagItem);
      retVal = tabInfo_.AddField(tagIt->second.c_str(), pTagItem->GetValueType());
      if (retVal != PdbE_OK)
        return PdbE_SQL_ERROR;
    }
  }

  if (fieldVec_.size() == 0)
    return PdbE_SQL_ERROR;

  return PdbE_OK;
}



