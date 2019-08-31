#include "query/tstamp_filter.h"
#include "util/string_tool.h"
#include "expr/parse.h"
#include "pdb_error.h"
#include "util/date_time.h"
#include "internal.h"

TStampFilter::TStampFilter()
{
  includeMinTimeStamp_ = true;
  includeMaxTimeStamp_ = true;
  minTimeStamp_ = MinMillis;
  maxTimeStamp_ = MaxMillis;
}
TStampFilter::~TStampFilter()
{

}

bool TStampFilter::BuildFilter(const ExprItem* pCondition)
{
  minTimeStamp_ = MinMillis;
  maxTimeStamp_ = MaxMillis;

  return _BuildFilter(pCondition);
}

bool TStampFilter::_BuildFilter(const ExprItem* pExpr)
{
  if (pExpr == nullptr)
    return true;

  int op = pExpr->GetOp();

  const ExprItem* pLeftExpr = pExpr->GetLeftExpr();
  const ExprItem* pRightExpr = pExpr->GetRightExpr();

  if (op == TK_AND)
  {
    if (pLeftExpr == nullptr || pRightExpr == nullptr)
      return false;

    if (!_BuildFilter(pLeftExpr))
      return false;

    if (!_BuildFilter(pRightExpr))
      return false;

    return true;
  }
  else
  {
    if (pLeftExpr == nullptr)
      return false;

    if (pLeftExpr->GetOp() != TK_ID)
      return false;

    const std::string& fieldName = pLeftExpr->GetValueStr();

    if (StringTool::ComparyNoCase(fieldName, TSTAMP_FIELD_NAME, (sizeof(TSTAMP_FIELD_NAME) - 1)))
    {
      //只处理时间戳的 等于，大于，大于等于，小于，小于等于
      if (op != TK_EQ
        && op != TK_GT
        && op != TK_GE
        && op != TK_LT
        && op != TK_LE)
        return true;

      int64_t timeStamp = 0;

      if (pRightExpr == nullptr)
        return false;

      const std::string rightValStr = pRightExpr->GetValueStr();
      if (pRightExpr->GetOp() == TK_INTEGER)
      {
        if (!StringTool::StrToInt64(rightValStr.c_str(), rightValStr.size(), &timeStamp))
          return false;
      }
      else if (pRightExpr->GetOp() == TK_STRING)
      {
        DateTime dt;
        if (!dt.Parse(rightValStr.c_str(), rightValStr.size()))
          return false;

        timeStamp = dt.TotalMilliseconds();
      }
      else
      {
        return false;
      }

      switch (op)
      {
      case TK_EQ:
      {
        if (minTimeStamp_ == 0 || minTimeStamp_ <= timeStamp)
        {
          minTimeStamp_ = timeStamp;
          includeMinTimeStamp_ = true;
        }
        else
        {
          return false;
        }

        if (maxTimeStamp_ == 0 || maxTimeStamp_ >= timeStamp)
        {
          maxTimeStamp_ = timeStamp;
          includeMaxTimeStamp_ = true;
        }
        else
        {
          return false;
        }

        break;
      }
      case TK_GT:
      case TK_GE:
      {
        if (minTimeStamp_ == 0 || minTimeStamp_ <= timeStamp)
        {
          minTimeStamp_ = timeStamp;
          includeMinTimeStamp_ = (op == TK_GE);
        }
        else
        {
          return false;
        }
        break;
      }
      case TK_LT:
      case TK_LE:
      {
        if (maxTimeStamp_ == 0 || maxTimeStamp_ >= timeStamp)
        {
          maxTimeStamp_ = timeStamp;
          includeMaxTimeStamp_ = (op == TK_LE);
        }
        else
        {
          return false;
        }
        break;
      }
      }
    }
  }

  return true;
}


