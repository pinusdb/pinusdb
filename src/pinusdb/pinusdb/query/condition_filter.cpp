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

#include "query/condition_filter.h"
#include "internal.h"
#include "pdb_error.h"
#include "util/string_tool.h"
#include "expr/parse.h"
#include "util/date_time.h"

////////////////////////////////////////////////////////////////////////////////////

ConditionFilter::ConditionFilter()
{
}

ConditionFilter::~ConditionFilter()
{
  for (auto conIt = conditionVec_.begin(); conIt != conditionVec_.end(); conIt++)
  {
    delete *conIt;
  }
}

PdbErr_t ConditionFilter::BuildCondition(
  const ExprItem* pExpr, const TableInfo* pTabInfo)
{
  if (pExpr == nullptr)
  {
    return PdbE_OK;
  }

  return _BuildCondition(pExpr, pTabInfo);
}

PdbErr_t ConditionFilter::RunCondition(
  const DBVal* pVals, size_t valCnt, bool& resultVal) const
{
  if (this->conditionVec_.size() == 0)
  {
    resultVal = true;
    return PdbE_OK;
  }

  for (auto conIt = conditionVec_.begin(); 
    conIt != conditionVec_.end(); conIt++)
  {
    if (!(*conIt)->GetLogic(pVals, valCnt))
    {
      resultVal = false;
      return PdbE_OK;
    }
  }

  resultVal = true;
  return PdbE_OK;
}

PdbErr_t ConditionFilter::_BuildCondition(const ExprItem* pExpr, const TableInfo* pTabInfo)
{
  if (pTabInfo == nullptr)
    return PdbE_INVALID_PARAM;

  if (pExpr == nullptr)
    return PdbE_OK;

  PdbErr_t retVal = PdbE_OK;

  ConditionItem* pConditionItem = nullptr;

  const ExprItem* pLeftExpr = pExpr->GetLeftExpr();
  const ExprItem* pRightExpr = pExpr->GetRightExpr();

  size_t fieldPos = 0;
  int32_t fieldType = 0;

  DBVal rightVal;
  DBVAL_SET_NULL(&rightVal);

  int op = pExpr->GetOp();

  if (op == TK_AND)
  {
    if (pLeftExpr == nullptr || pRightExpr == nullptr)
    {
      return PdbE_SQL_CONDITION_EXPR_ERROR;
    }

    retVal = _BuildCondition(pLeftExpr, pTabInfo);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = _BuildCondition(pRightExpr, pTabInfo);
    if (retVal != PdbE_OK)
      return retVal;
  }
  else
  {
    //左边必须是字段
    if (pLeftExpr == nullptr)
      return PdbE_SQL_CONDITION_EXPR_ERROR;

    if (pLeftExpr->GetOp() != TK_ID)
      return PdbE_SQL_CONDITION_EXPR_ERROR;

    const std::string& fieldName = pLeftExpr->GetValueStr();
    retVal = pTabInfo->GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
    if (retVal != PdbE_OK)
      return retVal;

    if (op == TK_ISTRUE || op == TK_ISFALSE)
    {
      if (fieldType != PDB_FIELD_TYPE::TYPE_BOOL)
        return PdbE_SQL_CONDITION_EXPR_ERROR;

      pConditionItem = new EqBoolCondition(fieldPos, (op == TK_ISTRUE));
    }
    else if (op == TK_ISNOTNULL)
    {
      pConditionItem = new IsNotNullCondition(fieldPos);
    }
    else if (op == TK_ISNULL)
    {
      pConditionItem = new IsNullCondition(fieldPos);
    }
    else
    {
      //所有操作都需要有两个操作数
      if (pRightExpr == nullptr)
        return PdbE_SQL_CONDITION_EXPR_ERROR;

      const std::string& valueStr = pRightExpr->GetValueStr();
      int32_t rightOp = pRightExpr->GetOp();

      int64_t tmpInt64 = 0;
      double tmpDouble = 0;

      switch (rightOp)
      {
      case TK_INTEGER:
      case TK_UINTEGER:
      {
        if (!StringTool::StrToInt64(valueStr.c_str(), valueStr.size(), &tmpInt64))
          return PdbE_SQL_CONDITION_EXPR_ERROR;

        if (rightOp == TK_UINTEGER)
          tmpInt64 *= -1;

        DBVAL_SET_INT64(&rightVal, tmpInt64);
        break;
      }
      case TK_DOUBLE:
      case TK_UDOUBLE:
      {
        if (!StringTool::StrToDouble(valueStr.c_str(), valueStr.size(), &tmpDouble))
          return PdbE_SQL_CONDITION_EXPR_ERROR;

        if (rightOp == TK_UDOUBLE)
          tmpDouble *= -1;

        DBVAL_SET_DOUBLE(&rightVal, tmpDouble);
        break;
      }
      case TK_STRING:
      {
        DBVAL_SET_STRING(&rightVal, valueStr.c_str(), valueStr.size());
        break;
      }
      default:
        return PdbE_SQL_CONDITION_EXPR_ERROR;
      }

      //如果左边的字段是时间戳，右边的表达式是字符串，尝试将字符串转成时间戳
      if (fieldType == PDB_FIELD_TYPE::TYPE_DATETIME && DBVAL_IS_STRING(&rightVal))
      {
        DateTime dt;
        if (!dt.Parse((const char*)rightVal.val_.pData_, rightVal.dataLen_))
          return PdbE_SQL_ERROR;

        DBVAL_SET_DATETIME(&rightVal, dt.TotalMilliseconds());
      }

      if (PDB_TYPE_IS_REAL(fieldType))
      {
        //如果字段类型是real系列，转换成double
        fieldType = PDB_FIELD_TYPE::TYPE_DOUBLE;
      }

      //如果表达式两边的类型不一样
      if (fieldType != DBVAL_GET_TYPE(&rightVal))
      {
        if (DBVAL_IS_INT64(&rightVal))
        {
          //1. int64 可以转成 datetime, double
          tmpInt64 = DBVAL_GET_INT64(&rightVal);
          if (fieldType == PDB_FIELD_TYPE::TYPE_DATETIME)
          {
            if (tmpInt64 >= MinMillis && tmpInt64 < MaxMillis)
            {
              DBVAL_SET_DATETIME(&rightVal, tmpInt64);
            }
          }
          else if (fieldType == PDB_FIELD_TYPE::TYPE_DOUBLE)
          {
            DBVAL_SET_DOUBLE(&rightVal, static_cast<double>(tmpInt64));
          }
        }

        if (fieldType != DBVAL_GET_TYPE(&rightVal))
        {
          return PdbE_SQL_ERROR;
        }
      }

      switch (op)
      {
      case TK_LT:
        pConditionItem = BuildLtCondition(fieldPos, &rightVal);
        break;
      case TK_LE:
        pConditionItem = BuildLeCondition(fieldPos, &rightVal);
        break;
      case TK_GT:
        pConditionItem = BuildGtCondition(fieldPos, &rightVal);
        break;
      case TK_GE:
        pConditionItem = BuildGeCondition(fieldPos, &rightVal);
        break;
      case TK_NE:
        pConditionItem = BuildNeCondition(fieldPos, &rightVal);
        break;
      case TK_EQ:
        pConditionItem = BuildEqCondition(fieldPos, &rightVal);
        break;
      case TK_LIKE:
        pConditionItem = BuildLikeCondition(fieldPos, &rightVal);
        break;
      }
    }

    if (pConditionItem == nullptr)
      return PdbE_SQL_ERROR;

    conditionVec_.push_back(pConditionItem);
  }

  return PdbE_OK;
}

ConditionItem* ConditionFilter::BuildLtCondition(size_t fieldPos, DBVal* pVal)
{
  switch (DBVAL_GET_TYPE(pVal))
  {
  case PDB_VALUE_TYPE::VAL_INT64:
    return new LtNumCondition<PDB_VALUE_TYPE::VAL_INT64>(fieldPos, DBVAL_GET_INT64(pVal));
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return new LtNumCondition<PDB_VALUE_TYPE::VAL_DATETIME>(fieldPos, DBVAL_GET_DATETIME(pVal));
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return new LtDoubleCondition(fieldPos, DBVAL_GET_DOUBLE(pVal));
  }

  return nullptr;
}

ConditionItem* ConditionFilter::BuildLeCondition(size_t fieldPos, DBVal* pVal)
{
  switch (DBVAL_GET_TYPE(pVal))
  {
  case PDB_VALUE_TYPE::VAL_INT64:
    return new LeNumCondition<PDB_VALUE_TYPE::VAL_INT64>(fieldPos, DBVAL_GET_INT64(pVal));
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return new LeNumCondition<PDB_VALUE_TYPE::VAL_DATETIME>(fieldPos, DBVAL_GET_DATETIME(pVal));
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return new LeDoubleCondition(fieldPos, DBVAL_GET_DOUBLE(pVal));
  }

  return nullptr;
}

ConditionItem* ConditionFilter::BuildGtCondition(size_t fieldPos, DBVal* pVal)
{
  switch (DBVAL_GET_TYPE(pVal))
  {
  case PDB_VALUE_TYPE::VAL_INT64:
    return new GtNumCondition<PDB_VALUE_TYPE::VAL_INT64>(fieldPos, DBVAL_GET_INT64(pVal));
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return new GtNumCondition<PDB_VALUE_TYPE::VAL_DATETIME>(fieldPos, DBVAL_GET_DATETIME(pVal));
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return new GtDoubleCondition(fieldPos, DBVAL_GET_DOUBLE(pVal));
  }

  return nullptr;
}

ConditionItem* ConditionFilter::BuildGeCondition(size_t fieldPos, DBVal* pVal)
{

  switch (DBVAL_GET_TYPE(pVal))
  {
  case PDB_VALUE_TYPE::VAL_INT64:
    return new GeNumCondition<PDB_VALUE_TYPE::VAL_INT64>(fieldPos, DBVAL_GET_INT64(pVal));
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return new GeNumCondition<PDB_VALUE_TYPE::VAL_DATETIME>(fieldPos, DBVAL_GET_DATETIME(pVal));
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return new GeDoubleCondition(fieldPos, DBVAL_GET_DOUBLE(pVal));
  }

  return nullptr;
}

ConditionItem* ConditionFilter::BuildNeCondition(size_t fieldPos, DBVal* pVal)
{
  switch (DBVAL_GET_TYPE(pVal))
  {
  case PDB_VALUE_TYPE::VAL_INT64:
    return new NeNumCondition<PDB_VALUE_TYPE::VAL_INT64>(fieldPos, DBVAL_GET_INT64(pVal));
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return new NeNumCondition<PDB_VALUE_TYPE::VAL_DATETIME>(fieldPos, DBVAL_GET_DATETIME(pVal));
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return new NeDoubleCondition(fieldPos, DBVAL_GET_DOUBLE(pVal));
  case PDB_VALUE_TYPE::VAL_STRING:
    return new NeStringCondition(fieldPos, DBVAL_GET_STRING(pVal), DBVAL_GET_LEN(pVal));
  }

  return nullptr;
}

ConditionItem* ConditionFilter::BuildEqCondition(size_t fieldPos, DBVal* pVal)
{
  switch (DBVAL_GET_TYPE(pVal))
  {
  case PDB_VALUE_TYPE::VAL_INT64:
    return new EqNumCondition<PDB_VALUE_TYPE::VAL_INT64>(fieldPos, DBVAL_GET_INT64(pVal));
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return new EqNumCondition<PDB_VALUE_TYPE::VAL_DATETIME>(fieldPos, DBVAL_GET_DATETIME(pVal));
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return new EqDoubleCondition(fieldPos, DBVAL_GET_DOUBLE(pVal));
  case PDB_VALUE_TYPE::VAL_STRING:
    return new EqStringCondition(fieldPos, DBVAL_GET_STRING(pVal), DBVAL_GET_LEN(pVal));
  }

  return nullptr;
}

ConditionItem* ConditionFilter::BuildLikeCondition(size_t fieldPos, DBVal* pVal)
{
  if (DBVAL_IS_STRING(pVal))
  {
    return new LikeCondition(fieldPos, DBVAL_GET_STRING(pVal), DBVAL_GET_LEN(pVal));
  }

  return nullptr;
}
