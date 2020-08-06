
#include "query/query_group.h"
#include "pdb_error.h"
#include "query/group_field.h"
#include "util/coding.h"
#include "server/proto_header.h"

QueryGroup::QueryGroup()
{
  queryOffset_ = 0;
  queryRecord_ = PDB_QUERY_DEFAULT_COUNT;
  lastGroupId_ = 0;
  pLastObj_ = nullptr;
}

QueryGroup::~QueryGroup()
{
  for (auto fieldIt = rstFieldVec_.begin(); fieldIt != rstFieldVec_.end(); fieldIt++)
  {
    delete (*fieldIt);
  }

  for (auto fieldIt = grpFieldVec_.begin(); fieldIt != grpFieldVec_.end(); fieldIt++)
  {
    delete (*fieldIt);
  }

  for (auto objIt = objVec_.begin(); objIt != objVec_.end(); objIt++)
  {
    delete (*objIt);
  }
}

PdbErr_t QueryGroup::AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded)
{
  bool resultVal = true;
  PdbErr_t retVal = PdbE_OK;

  retVal = condiFilter_.RunCondition(pVals, valCnt, resultVal);
  if (retVal != PdbE_OK)
    return retVal;

  if (pIsAdded != nullptr)
    *pIsAdded = false;

  if (!resultVal)
    return PdbE_OK;

  uint64_t groupId = GetGroupId(pVals, valCnt);
  if (lastGroupId_ != groupId || pLastObj_ == nullptr)
  {
    auto objIter = objMap_.find(groupId);
    if (objIter == objMap_.end())
      return PdbE_OK;

    lastGroupId_ = groupId;
    pLastObj_ = objIter->second;
  }

  retVal = pLastObj_->AppendData(pVals, valCnt);
  if (pIsAdded != nullptr)
  {
    *pIsAdded = (retVal == PdbE_OK);
  }

  return PdbE_OK;
}

bool QueryGroup::GetIsFullFlag() const
{
  return false;
}

PdbErr_t QueryGroup::GetResult(std::string& dataBuf, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  PdbErr_t retVal = PdbE_OK;
  std::vector<DBVal> recVec;
  size_t valSize = grpFieldVec_.size();
  recVec.resize(valSize);
  DBVal* pRec = recVec.data();
  DBVal tmpVal;

  dataBuf.reserve(256 * 1024);
  dataBuf.resize(ProtoHeader::kProtoHeadLength);
  resultInfo_.Serialize(dataBuf);

  for (auto objIt = objVec_.begin(); objIt != objVec_.end(); objIt++)
  {
    retVal = (*objIt)->GetRecord(pRec, valSize);
    if (retVal != PdbE_OK)
      return retVal;

    for (auto fieldIt = rstFieldVec_.begin(); fieldIt != rstFieldVec_.end(); fieldIt++)
    {
      retVal = (*fieldIt)->GetValue(pRec, &tmpVal);
      if (retVal != PdbE_OK)
        return retVal;

      dataBuf.push_back(static_cast<char>(DBVAL_GET_TYPE(&tmpVal)));
      switch (DBVAL_GET_TYPE(&tmpVal))
      {
      case PDB_VALUE_TYPE::VAL_NULL:
        break;
      case PDB_VALUE_TYPE::VAL_BOOL:
        dataBuf.push_back(static_cast<char>(DBVAL_GET_BOOL(&tmpVal) ? PDB_BOOL_TRUE : PDB_BOOL_FALSE));
        break;
      case PDB_VALUE_TYPE::VAL_INT64:
        Coding::PutVarint64(&dataBuf, Coding::ZigzagEncode64(DBVAL_GET_INT64(&tmpVal)));
        break;
      case PDB_VALUE_TYPE::VAL_DATETIME:
        Coding::PutVarint64(&dataBuf, static_cast<uint64_t>(DBVAL_GET_INT64(&tmpVal)));
        break;
      case PDB_VALUE_TYPE::VAL_DOUBLE:
        dataBuf.append(reinterpret_cast<const char*>(DBVAL_GET_BYTES(&tmpVal)), 8);
        break;
      case PDB_VALUE_TYPE::VAL_STRING:
      case PDB_VALUE_TYPE::VAL_BLOB:
        Coding::PutVarint64(&dataBuf, static_cast<uint64_t>(DBVAL_GET_LEN(&tmpVal)));
        dataBuf.append(DBVAL_GET_STRING(&tmpVal), DBVAL_GET_LEN(&tmpVal));
        break;
      }
    }
  }

  if (pFieldCnt != nullptr)
    *pFieldCnt = static_cast<uint32_t>(rstFieldVec_.size());
  if (pRecordCnt != nullptr)
    *pRecordCnt = static_cast<uint32_t>(grpFieldVec_.size());

  return PdbE_OK;
}

void QueryGroup::GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const
{
  condiFilter_.GetDevIdRange(pMinDevId, pMaxDevId);
}

void QueryGroup::GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const
{
  condiFilter_.GetTstampRange(pMinTstamp, pMaxTstamp);
}

bool QueryGroup::FilterDevId(int64_t devId) const
{
  return condiFilter_.FilterDevId(devId);
}

bool QueryGroup::IsEmptySet() const
{
  return condiFilter_.AlwaysFalse();
}

size_t QueryGroup::GetQueryOffset() const
{
  return queryOffset_;
}

size_t QueryGroup::GetQueryRecord() const
{
  return queryRecord_;
}

bool QueryGroup::IsQueryLast() const
{
  for (auto fieldIt = grpFieldVec_.begin(); fieldIt != grpFieldVec_.end(); fieldIt++)
  {
    if (!(*fieldIt)->IsLastFunc())
      return false;
  }

  return true;
}

bool QueryGroup::IsQueryFirst() const
{
  for (auto fieldIt = grpFieldVec_.begin(); fieldIt != grpFieldVec_.end(); fieldIt++)
  {
    if (!(*fieldIt)->IsFirstFunc())
      return false;
  }

  return true;
}

PdbErr_t QueryGroup::BuildQuery(const QueryParam* pQueryParam, const TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;
  if (pQueryParam == nullptr)
    return PdbE_INVALID_PARAM;

  int32_t fieldType = 0;
  size_t fieldPos = 0;
  DBVal nameVal;
  std::string fieldName;
  int64_t nowMillis = DateTime::NowMilliseconds();
  if (pQueryParam->pWhere_ != nullptr)
  {
    retVal = condiFilter_.BuildCondition(pTabInfo, pQueryParam->pWhere_, nowMillis);
    if (retVal != PdbE_OK)
      return retVal;
  }

  if (pQueryParam->pLimit_ != nullptr)
  {
    retVal = pQueryParam->pLimit_->Valid();
    if (retVal != PdbE_OK)
      return retVal;

    queryOffset_ = pQueryParam->pLimit_->GetOffset();
    queryRecord_ = pQueryParam->pLimit_->GetQueryCnt();
  }

  if (pQueryParam->pGroup_ != nullptr)
  {
    if (!pQueryParam->pGroup_->Valid())
      return PdbE_SQL_GROUP_ERROR;

    if (pQueryParam->pGroup_->IsDevIdGroup())
    {
      groupInfo_.AddField(DEVID_FIELD_NAME, PDB_FIELD_TYPE::TYPE_INT64);
      grpFieldVec_.push_back(new GroupDevIdField(0));
    }

    if (pQueryParam->pGroup_->IsTStampGroup())
    {
      groupInfo_.AddField(TSTAMP_FIELD_NAME, PDB_FIELD_TYPE::TYPE_DATETIME);
      grpFieldVec_.push_back(new GroupTstampField(0));
    }
  }

  if (pQueryParam->pTagList_ == nullptr)
    return PdbE_INVALID_PARAM;

  const std::vector<TargetItem>* pTagItemList = pQueryParam->pTagList_->GetTargetList();
  for (auto tagIt = pTagItemList->begin(); tagIt != pTagItemList->end(); tagIt++)
  {
    if (tagIt->first->GetValueType() == TK_STAR)
      return PdbE_SQL_ERROR;

    retVal = BuildTargetGroupItem(pTabInfo, tagIt->first, &groupInfo_, grpFieldVec_, nowMillis);
    if (retVal != PdbE_OK)
      return retVal;
  }

  ValueItem* pTagItem = nullptr;
  for (auto tagIt = pTagItemList->begin(); tagIt != pTagItemList->end(); tagIt++)
  {
    if (tagIt->first->GetValueType() == TK_STRING)
    {
      nameVal = tagIt->first->GetValue();
      if (!DBVAL_IS_STRING(&nameVal))
        return PdbE_SQL_ERROR;

      pTagItem = nullptr;
      do {
        fieldName = std::string(DBVAL_GET_STRING(&nameVal), DBVAL_GET_LEN(&nameVal));
        retVal = groupInfo_.GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
        if (retVal != PdbE_OK)
          break;

        pTagItem = new FieldValue(fieldPos, fieldType);
      } while (false);

      if (pTagItem == nullptr)
      {
        pTagItem = new ConstValue(nameVal);
      }

      rstFieldVec_.push_back(pTagItem);
      retVal = resultInfo_.AddField(tagIt->second.c_str(), pTagItem->GetValueType());
      if (retVal != PdbE_OK)
        return retVal;
    }
    else
    {
      ValueItem* pNewValue = BuildGeneralValueItem(&groupInfo_, tagIt->first, nowMillis);
      if (pNewValue == nullptr)
        return PdbE_SQL_ERROR;

      rstFieldVec_.push_back(pNewValue);
      retVal = resultInfo_.AddField(tagIt->second.c_str(), pNewValue->GetValueType());
      if (retVal != PdbE_OK)
        return retVal;
    }
  }

  retVal = CustomBuild(pQueryParam);
  if (retVal != PdbE_OK)
    return retVal;

  return PdbE_OK;
}


QueryGroupAll::QueryGroupAll()
{
}

QueryGroupAll::~QueryGroupAll()
{
}

bool QueryGroupAll::IsEmptySet() const
{
  return condiFilter_.AlwaysFalse() || queryOffset_ > 0;
}

int64_t QueryGroupAll::GetGroupId(const DBVal* pVals, size_t valCnt)
{
  return 0;
}

PdbErr_t QueryGroupAll::CustomBuild(const QueryParam* pQueryParam)
{
  if (queryOffset_ == 0)
  {
    //创建结果
    ResultObject* pObj = new ResultObject(grpFieldVec_, 0, 0);
    if (pObj == nullptr)
      return PdbE_NOMEM;

    objVec_.push_back(pObj);
    objMap_.insert(std::pair<uint64_t, ResultObject*>(0, pObj));
  }

  return PdbE_OK;
}

QueryGroupDevID::QueryGroupDevID()
{

}

QueryGroupDevID::~QueryGroupDevID()
{

}

PdbErr_t QueryGroupDevID::InitGroupDevID(const std::list<int64_t>& devIdList)
{
  ResultObject* pObj = nullptr;
  for (auto devIdIt = devIdList.begin(); devIdIt != devIdList.end(); devIdIt++)
  {
    pObj = new ResultObject(grpFieldVec_, *devIdIt, 0);
    if (pObj == nullptr)
      return PdbE_NOMEM;

    objVec_.push_back(pObj);
    objMap_.insert(std::pair<uint64_t, ResultObject*>((uint64_t)*devIdIt, pObj));
  }

  return PdbE_OK;
}

int64_t QueryGroupDevID::GetGroupId(const DBVal* pVals, size_t valCnt)
{
  return DBVAL_ELE_GET_INT64(pVals, PDB_DEVID_INDEX);
}

QueryGroupTstamp::QueryGroupTstamp()
{
  this->timeGroupRange_ = 0;
  this->minTstamp_ = 0;
  this->maxTstamp_ = 0;
}

QueryGroupTstamp::~QueryGroupTstamp()
{

}

bool QueryGroupTstamp::IsEmptySet() const
{
  return condiFilter_.AlwaysFalse() || minTstamp_ > maxTstamp_;
}

void QueryGroupTstamp::GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const
{
  if (pMinTstamp != nullptr)
    *pMinTstamp = minTstamp_;

  if (pMaxTstamp != nullptr)
    *pMaxTstamp = maxTstamp_;
}

int64_t QueryGroupTstamp::GetGroupId(const DBVal* pVals, size_t valCnt)
{
  return (DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX) - minTstamp_) / timeGroupRange_;
}

PdbErr_t QueryGroupTstamp::CustomBuild(const QueryParam* pQueryParam)
{
  if (pQueryParam->pGroup_ == nullptr)
    return PdbE_SQL_ERROR;

  if (!pQueryParam->pGroup_->Valid())
    return PdbE_SQL_ERROR;

  if (!pQueryParam->pGroup_->IsTStampGroup())
    return PdbE_SQL_ERROR;

  PdbErr_t retVal = pQueryParam->pGroup_->GetTStampStep(timeGroupRange_);
  if (retVal != PdbE_OK)
    return retVal;

  int64_t minTs, maxTs;
  condiFilter_.GetTstampRange(&minTs, &maxTs);
  if (minTs == 0)
    return PdbE_SQL_GROUP_LOST_BEGIN_TSTAMP;

  minTstamp_ = minTs + (queryOffset_ * timeGroupRange_);
  maxTstamp_ = minTstamp_ + (queryRecord_ * timeGroupRange_);
  
  if (maxTs < maxTstamp_)
    maxTstamp_ = maxTs;

  ResultObject* pObj = nullptr;
  int64_t grpCnt = (maxTstamp_ - minTstamp_ + timeGroupRange_ - 1) / timeGroupRange_;
  for (int64_t grpId = 0; grpId < grpCnt; grpId++)
  {
    pObj = new ResultObject(grpFieldVec_, 0, (minTstamp_ + (grpId * timeGroupRange_)));
    if (pObj == nullptr)
      return PdbE_NOMEM;

    objVec_.push_back(pObj);
    objMap_.insert(std::pair<uint64_t, ResultObject*>((uint64_t)grpId, pObj));
  }

  return PdbE_OK;

}
