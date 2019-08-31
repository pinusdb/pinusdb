#include "query/dev_filter.h"
#include "util/string_tool.h"
#include "expr/parse.h"
#include "pdb_error.h"
#include "internal.h"

DevFilter::DevFilter()
{
  emptySet_ = false;
  includeMin_ = true;
  includeMax_ = true;
  minDevId_ = 1;
  maxDevId_ = INT64_MAX;
}

DevFilter::~DevFilter()
{
}

PdbErr_t DevFilter::BuildFilter(const ExprItem* pCondition)
{
  emptySet_ = false;
  includeMin_ = true;
  includeMax_ = true;
  minDevId_ = 1;
  maxDevId_ = INT64_MAX;

  return _BuildFilter(pCondition);
}

bool DevFilter::IsEmptySet() const
{
  return emptySet_;
}

bool DevFilter::HaveEqualObjId() const
{
  return (includeMin_ && includeMax_ && minDevId_ == maxDevId_);
}

int64_t DevFilter::GetEqualObjId() const
{
  if (HaveEqualObjId())
  {
    return minDevId_;
  }

  return 0;
}

bool DevFilter::Filter(int64_t devId) const
{
  if (devId > minDevId_ || (devId == minDevId_ && includeMin_))
  {
    if (devId < maxDevId_ || (devId == maxDevId_ && includeMax_))
      return true;
  }

  return false;
}

PdbErr_t DevFilter::_BuildFilter(const ExprItem* pExpr)
{
  if (pExpr == nullptr)
    return PdbE_OK;

  PdbErr_t retVal = PdbE_OK;

  int op = pExpr->GetOp();

  const ExprItem* pLeftExpr = pExpr->GetLeftExpr();
  const ExprItem* pRightExpr = pExpr->GetRightExpr();

  if (op == TK_AND)
  {
    if (pLeftExpr == nullptr || pRightExpr == nullptr)
      return PdbE_SQL_CONDITION_EXPR_ERROR;

    retVal = _BuildFilter(pLeftExpr);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = _BuildFilter(pRightExpr);
    if (retVal != PdbE_OK)
      return retVal;

    return retVal;
  }
  else
  {
    if (pLeftExpr == nullptr)
      return PdbE_SQL_CONDITION_EXPR_ERROR;

    if (pLeftExpr->GetOp() != TK_ID)
      return PdbE_SQL_CONDITION_EXPR_ERROR;

    const std::string& fieldName = pLeftExpr->GetValueStr();

    if (StringTool::ComparyNoCase(fieldName, DEVID_FIELD_NAME, (sizeof(DEVID_FIELD_NAME) - 1)))
    {
      //只处理对象的 等于，大于，大于等于，小于，小于等于
      if (op != TK_EQ
        && op != TK_GT
        && op != TK_GE
        && op != TK_LT
        && op != TK_LE)
        return PdbE_OK;

      int64_t tmpDevId = 0;

      if (pRightExpr == nullptr)
        return PdbE_SQL_CONDITION_EXPR_ERROR;

      const std::string rightValStr = pRightExpr->GetValueStr();
      if (pRightExpr->GetOp() == TK_INTEGER)
      {
        if (!StringTool::StrToInt64(rightValStr.c_str(), rightValStr.size(), &tmpDevId))
          return PdbE_INVALID_INT_VAL;
      }
      else
      {
        return PdbE_VALUE_MISMATCH;
      }

      switch (op)
      {
      case TK_EQ:
        if (minDevId_ < tmpDevId || (minDevId_ == tmpDevId && includeMin_))
        {
          minDevId_ = tmpDevId;
          includeMin_ = true;
        }
        else
          emptySet_ = true; //没有任何objid满足条件

        if (maxDevId_ > tmpDevId || (maxDevId_ == tmpDevId && includeMax_))
        {
          maxDevId_ = tmpDevId;
          includeMax_ = true;
        }
        else
          emptySet_ = true; //没有任何objid满足条件

        break;
      case TK_GT:
      case TK_GE:
        if (minDevId_ < tmpDevId || (minDevId_ == tmpDevId && includeMin_))
        {
          minDevId_ = tmpDevId;
          includeMin_ = (op == TK_GE);
        }
        else
          emptySet_ = true; //没有任何objid满足条件

        break;
      case TK_LT:
      case TK_LE:
        if (maxDevId_ > tmpDevId || (maxDevId_ == tmpDevId && includeMax_))
        {
          maxDevId_ = tmpDevId;
          includeMax_ = (op == TK_LE);
        }
        else
          emptySet_ = true; //没有任何objid满足条件

        break;
      }

    }
  }

  return PdbE_OK;
}
