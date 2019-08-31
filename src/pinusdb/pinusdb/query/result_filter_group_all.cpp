#include "query/result_filter_group_all.h"
#include "expr/parse.h"
#include "util/string_tool.h"

ResultFilterGroupAll::ResultFilterGroupAll()
{
}
ResultFilterGroupAll::~ResultFilterGroupAll()
{
}

PdbErr_t ResultFilterGroupAll::AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded)
{
  bool resultVal = true;
  PdbErr_t retVal = PdbE_OK;

  retVal = this->condiFilter_.RunCondition(pVals, valCnt, resultVal);
  if (retVal != PdbE_OK)
    return retVal;

  if (resultVal)
  {
    if (objVec_.size() != 1)
      return PdbE_SQL_RESULT_ERROR;

    retVal = objVec_[0]->AppendData(pVals, valCnt);
    if (pIsAdded != nullptr)
    {
      *pIsAdded = (retVal == PdbE_OK);
    }

    return retVal;
  }

  if (pIsAdded != nullptr)
    *pIsAdded = false;

  return PdbE_OK;
}

PdbErr_t ResultFilterGroupAll::BuildCustomFilter(const QueryParam* pQueryParam,
  const TableInfo* pTabInfo, Arena* pArena)
{
  PdbErr_t retVal = PdbE_OK;

  const ExprList* pColList = pQueryParam->pSelList_;
  const GroupOpt* pGroup = pQueryParam->pGroup_;

  retVal = BuildGroupResultField(pColList->GetExprList(), pTabInfo, pGroup, pArena);
  if (retVal != PdbE_OK)
    return retVal;

  if (queryOffset_ > 0)
  {
    //group all 只有一个结果，使用limit跳过后是空结果集
    isEmptySet_ = true;
  }
  else
  {
    //这里和其它聚合不太相同的地方，只有一条返回值
    ResultObject* pRstObj = new ResultObject(fieldVec_, 0, 0);
    if (pRstObj == nullptr)
      return PdbE_NOMEM;

    objVec_.push_back(pRstObj);
    std::pair<uint64_t, ResultObject*> objPair(0, pRstObj);
    objMap_.insert(objPair);
  }

  return retVal;
}

bool ResultFilterGroupAll::GetIsFullFlag() const
{
  return false;
}



