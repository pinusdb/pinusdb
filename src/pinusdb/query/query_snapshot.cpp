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

PdbErr_t QuerySnapshot::AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded)
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

      recData.push_back(static_cast<char>(DBVAL_GET_TYPE(&tmpVal)));
      switch (DBVAL_GET_TYPE(&tmpVal))
      {
      case PDB_VALUE_TYPE::VAL_NULL:
        break;
      case PDB_VALUE_TYPE::VAL_BOOL:
        recData.push_back(static_cast<char>(DBVAL_GET_BOOL(&tmpVal) ? PDB_BOOL_TRUE : PDB_BOOL_FALSE));
        break;
      case PDB_VALUE_TYPE::VAL_INT64:
        Coding::PutVarint64(&recData, Coding::ZigzagEncode64(DBVAL_GET_INT64(&tmpVal)));
        break;
      case PDB_VALUE_TYPE::VAL_DATETIME:
        Coding::PutVarint64(&recData, static_cast<uint64_t>(DBVAL_GET_INT64(&tmpVal)));
        break;
      case PDB_VALUE_TYPE::VAL_DOUBLE:
        recData.append(reinterpret_cast<const char*>(DBVAL_GET_BYTES(&tmpVal)), 8);
        break;
      case PDB_VALUE_TYPE::VAL_STRING:
      case PDB_VALUE_TYPE::VAL_BLOB:
        Coding::PutVarint64(&recData, static_cast<uint64_t>(DBVAL_GET_LEN(&tmpVal)));
        recData.append(DBVAL_GET_STRING(&tmpVal), DBVAL_GET_LEN(&tmpVal));
        break;
      }
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

PdbErr_t QuerySnapshot::BuildQuery(const QueryParam* pQueryParam, const TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  size_t fieldPos = 0;
  DBVal nameVal;
  std::string fieldName;
  int64_t nowMillis = DateTime::NowMilliseconds();
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
    retVal = condiFilter_.BuildCondition(pTabInfo, pQueryParam->pWhere_, nowMillis);
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

        fieldVec_.push_back(new FieldValue(pos, fieldType));
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

        pTagItem = new FieldValue(fieldPos, fieldType);
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
      pTagItem = BuildGeneralValueItem(pTabInfo, tagIt->first, nowMillis);
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



