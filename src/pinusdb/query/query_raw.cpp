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

PdbErr_t QueryRaw::AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded)
{
  bool resultVal = true;
  PdbErr_t retVal = PdbE_OK;
  DBVal tmpVal;

  if (curRecord_ >= queryRecord_)
    return PdbE_RESLT_FULL;

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

      resultData_.push_back(static_cast<char>(DBVAL_GET_TYPE(&tmpVal)));
      switch (DBVAL_GET_TYPE(&tmpVal))
      {
      case PDB_VALUE_TYPE::VAL_NULL:
        break;
      case PDB_VALUE_TYPE::VAL_BOOL:
        resultData_.push_back(static_cast<char>(DBVAL_GET_BOOL(&tmpVal) ? PDB_BOOL_TRUE : PDB_BOOL_FALSE));
        break;
      case PDB_VALUE_TYPE::VAL_INT64:
        Coding::PutVarint64(&resultData_, Coding::ZigzagEncode64(DBVAL_GET_INT64(&tmpVal)));
        break;
      case PDB_VALUE_TYPE::VAL_DATETIME:
        Coding::PutVarint64(&resultData_, static_cast<uint64_t>(DBVAL_GET_INT64(&tmpVal)));
        break;
      case PDB_VALUE_TYPE::VAL_DOUBLE:
        resultData_.append(reinterpret_cast<const char*>(DBVAL_GET_BYTES(&tmpVal)), 8);
        break;
      case PDB_VALUE_TYPE::VAL_STRING:
      case PDB_VALUE_TYPE::VAL_BLOB:
        Coding::PutVarint64(&resultData_, static_cast<uint64_t>(DBVAL_GET_LEN(&tmpVal)));
        resultData_.append(DBVAL_GET_STRING(&tmpVal), DBVAL_GET_LEN(&tmpVal));
        break;
      }
    }

    curRecord_++;
    if (pIsAdded != nullptr)
      *pIsAdded = true;
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

PdbErr_t QueryRaw::BuildQuery(const QueryParam* pQueryParam, const TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  size_t fieldPos = 0;
  DBVal nameVal;
  std::string fieldName;
  int64_t nowMillis = DateTime::NowMilliseconds();
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

  resultData_.reserve(256 * 1024);
  resultData_.resize(ProtoHeader::kProtoHeadLength);
  tabInfo_.Serialize(resultData_);

  return PdbE_OK;
}

