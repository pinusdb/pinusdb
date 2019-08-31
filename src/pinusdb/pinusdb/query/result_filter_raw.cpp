#include "query/result_filter_raw.h"
#include "expr/parse.h"
#include "query/raw_field.h"

ResultFilterRaw::ResultFilterRaw()
{
  this->isFull_ = false;
}

ResultFilterRaw::~ResultFilterRaw()
{
}

PdbErr_t ResultFilterRaw::AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded)
{
  bool resultVal = true;
  PdbErr_t retVal = PdbE_OK;

  if (isFull_)
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

    ResultObject* pRstObj = new ResultObject(fieldVec_, 0, 0);
    if (pRstObj == nullptr)
      return PdbE_NOMEM;

    retVal = pRstObj->AppendData(pVals, valCnt);
    if (retVal != PdbE_OK)
    {
      delete pRstObj;
      return retVal;
    }

    objVec_.push_back(pRstObj);

    isFull_ = objVec_.size() >= queryRecord_;

    if (pIsAdded != nullptr)
      *pIsAdded = true;
  }

  return PdbE_OK;
}
PdbErr_t ResultFilterRaw::BuildCustomFilter(const QueryParam* pQueryParam, const TableInfo* pTabInfo, Arena* pArena)
{
  const ExprList* pColList = pQueryParam->pSelList_;

  if (pQueryParam->pGroup_ != nullptr)
  {
    return PdbE_INVALID_PARAM; //原始数据查询，不能有 group by
  }

  return BuildResultField(pColList->GetExprList(), pTabInfo, pArena);
}

PdbErr_t ResultFilterRaw::BuildResultField(const std::vector<ExprItem*>& colItemVec,
  const TableInfo* pTabInfo, Arena* pArena)
{
  PdbErr_t retVal = PdbE_OK;

  int32_t fieldType = 0;
  size_t  fieldPos = 0;
  std::string fieldName;

  for (auto colItem = colItemVec.begin(); colItem != colItemVec.end(); colItem++)
  {
    if ((*colItem)->GetOp() == TK_STAR)
    {
      //将所有列添加到结果集
      size_t tmpFieldCnt = pTabInfo->GetFieldCnt();

      for (size_t i = 0; i < tmpFieldCnt; i++)
      {
        pTabInfo->GetFieldInfo(i, &fieldType);
        fieldName = pTabInfo->GetFieldName(i);


        retVal = AddRawField(i, fieldName, fieldType, pArena);
        if (retVal != PdbE_OK)
          return retVal;
      }
    }
    else if ((*colItem)->GetOp() == TK_ID)
    {
      fieldName = (*colItem)->GetValueStr();
      retVal = pTabInfo->GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
      if (retVal != PdbE_OK)
        return retVal;

      std::string aliasName = (*colItem)->GetAliasName();
      if (aliasName.size() == 0)
        aliasName = fieldName;

      retVal = AddRawField(fieldPos, aliasName, fieldType, pArena);
      if (retVal != PdbE_OK)
        return retVal;
    }
    else
    {
      return PdbE_SQL_RESULT_ERROR;
    }
  }

  return PdbE_OK;
}