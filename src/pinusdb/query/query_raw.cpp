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

#include "query/query_raw.h"
#include "server/proto_header.h"
#include "util/coding.h"

QueryRaw::QueryRaw()
{
  queryOffset_ = 0;
  queryRecord_ = PDB_QUERY_DEFAULT_COUNT;
  curRecord_ = 0;
}

QueryRaw::~QueryRaw()
{
  for (auto fieldIt = fieldVec_.begin(); fieldIt != fieldVec_.end(); fieldIt++)
  {
    delete (*fieldIt);
  }
}

PdbErr_t QueryRaw::AppendSingle(const DBVal* pVals, size_t valCnt, bool* pIsAdded)
{
  bool resultVal = true;
  PdbErr_t retVal = PdbE_OK;
  DBVal tmpVal;

  if (curRecord_ >= queryRecord_)
    return PdbE_RESULT_FULL;

  retVal = this->condiFilter_.RunCondition(pVals, valCnt, resultVal);
  if (retVal != PdbE_OK)
    return retVal;

  if (pIsAdded != nullptr)
    *pIsAdded = false;

  if (resultVal)
  {
    if (queryOffset_ > 0)
    {
      queryOffset_--;
      return PdbE_OK;
    }

    for (auto fieldIt = fieldVec_.begin(); fieldIt != fieldVec_.end(); fieldIt++)
    {
      retVal = (*fieldIt)->GetValue(pVals, &tmpVal);
      if (retVal != PdbE_OK)
        return retVal;

      PushBackToResult(resultData_, tmpVal);
    }

    curRecord_++;
    if (pIsAdded != nullptr)
      *pIsAdded = true;
  }

  return PdbE_OK;
}

PdbErr_t QueryRaw::AppendArray(BlockValues& blockValues, bool* pIsAdded)
{
  PdbErr_t retVal = PdbE_OK;
  DBVal tmpVal;
  std::vector<DBVal> valVec;

  if (curRecord_ >= queryRecord_)
    return PdbE_RESULT_FULL;

  retVal = this->condiFilter_.RunConditionArray(blockValues);
  if (retVal != PdbE_OK)
    return retVal;

  if (pIsAdded != nullptr)
    *pIsAdded = false;

  size_t resultSize = blockValues.GetResultSize();
  if (resultSize > 0)
  {
    if (resultSize < queryOffset_)
    {
      queryOffset_ -= resultSize;
      return PdbE_OK;
    }

    size_t idx = queryOffset_;
    queryOffset_ = 0;

    if (pIsAdded != nullptr)
      *pIsAdded = true;

    size_t columnSize = blockValues.GetColumnSize();
    valVec.reserve(columnSize);
    for (; idx < resultSize; idx++)
    {
      retVal = blockValues.GetRecordValue(idx, valVec);
      if (retVal != PdbE_OK)
        return retVal;

      const DBVal* pVals = valVec.data();
      for (auto fieldIt = fieldVec_.begin(); fieldIt != fieldVec_.end(); fieldIt++)
      {
        retVal = (*fieldIt)->GetValue(pVals, &tmpVal);
        if (retVal != PdbE_OK)
          return retVal;

        PushBackToResult(resultData_, tmpVal);
      }

      curRecord_++;
      if (curRecord_ >= queryRecord_)
        return PdbE_RESULT_FULL;
    }
  }

  return PdbE_OK;
}

bool QueryRaw::GetIsFullFlag() const
{
  return curRecord_ >= queryRecord_;
}

PdbErr_t QueryRaw::GetResult(std::string& dataBuf, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  dataBuf.swap(resultData_);
  if (pFieldCnt != nullptr)
    *pFieldCnt = static_cast<uint32_t>(fieldVec_.size());
  if (pRecordCnt != nullptr)
    *pRecordCnt = static_cast<uint32_t>(curRecord_);

  return PdbE_OK;
}

void QueryRaw::GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const
{
  condiFilter_.GetDevIdRange(pMinDevId, pMaxDevId);
}

void QueryRaw::GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const
{
  condiFilter_.GetTstampRange(pMinTstamp, pMaxTstamp);
}

bool QueryRaw::FilterDevId(int64_t devId) const
{
  return condiFilter_.FilterDevId(devId);
}

bool QueryRaw::IsEmptySet() const
{
  return condiFilter_.AlwaysFalse();
}

size_t QueryRaw::GetQueryOffset() const
{
  return queryOffset_;
}

size_t QueryRaw::GetQueryRecord() const
{
  return queryRecord_;
}

void QueryRaw::GetUseFields(std::unordered_set<size_t>& fieldSet) const
{
  for (size_t idx = 0; idx < fieldVec_.size(); idx++)
  {
    fieldVec_[idx]->GetUseFields(fieldSet);
  }

  condiFilter_.GetUseFields(fieldSet);
}

PdbErr_t QueryRaw::BuildQuery(const QueryParam* pQueryParam, const TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  size_t fieldPos = 0;
  DBVal nameVal;
  std::string fieldName;
  int64_t nowMicroseconds = DateTime::NowMicrosecond();
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

  if (pQueryParam->pTagList_ == nullptr)
    return PdbE_SQL_ERROR;

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

  resultData_.reserve(256 * 1024);
  resultData_.resize(ProtoHeader::kProtoHeadLength);
  tabInfo_.Serialize(resultData_);

  return PdbE_OK;
}




