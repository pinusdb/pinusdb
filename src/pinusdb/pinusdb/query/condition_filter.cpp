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
  alwaysFalse_ = false;
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

  DBVal leftVal;
  DBVAL_SET_NULL(&leftVal);
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
    if (pLeftExpr == nullptr)
      return PdbE_SQL_CONDITION_EXPR_ERROR;

    if (pLeftExpr->GetOp() == TK_ID)
    {
      // 左边是字段
      const std::string& fieldName = pLeftExpr->GetValueStr();
      retVal = pTabInfo->GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
      if (retVal != PdbE_OK)
        return retVal;
    }
    else
    {
      // 左边是值
      if (!pLeftExpr->GetDBVal(&leftVal))
        return PdbE_SQL_CONDITION_EXPR_ERROR;
    }

    if (op == TK_ISNOTNULL || op == TK_ISNULL)
    {
      if (pLeftExpr->GetOp() != TK_ID)
        return PdbE_SQL_CONDITION_EXPR_ERROR;

      if (op == TK_ISNOTNULL)
        pConditionItem = new IsNotNullCondition(fieldPos);
      else
        pConditionItem = new IsNullCondition(fieldPos);
    }
    else
    {
      // 右边只能是值
      if (!pRightExpr->GetDBVal(&rightVal))
        return PdbE_SQL_CONDITION_EXPR_ERROR;

      if (pLeftExpr->GetOp() == TK_ID)
      {
        // field = val 条件
        // 如果左边字段类型是DateTime, 右边值类型为string, 尝试将string转成DateTime
        if (fieldType == PDB_FIELD_TYPE::TYPE_DATETIME && DBVAL_IS_STRING(&rightVal))
        {
          DateTime dt;
          if (!dt.Parse((const char*)rightVal.val_.pData_, rightVal.dataLen_))
            return PdbE_SQL_ERROR;

          DBVAL_SET_DATETIME(&rightVal, dt.TotalMilliseconds());
        }

        if (PDB_TYPE_IS_REAL(fieldType))
        {
          //如果字段类型是real系列，当作double处理
          fieldType = PDB_FIELD_TYPE::TYPE_DOUBLE;
        }

        //表达式两边的类型不一样
        if (fieldType != DBVAL_GET_TYPE(&rightVal))
        {
          if (DBVAL_IS_INT64(&rightVal))
          {
            // int64 可以转成 datetime, double
            int64_t tmpI64 = DBVAL_GET_INT64(&rightVal);
            if (fieldType == PDB_FIELD_TYPE::TYPE_DATETIME)
            {
              if (tmpI64 >= MinMillis && tmpI64 < MaxMillis)
              {
                DBVAL_SET_DATETIME(&rightVal, tmpI64);
              }
            }
            else if (fieldType == PDB_FIELD_TYPE::TYPE_DOUBLE)
            {
              DBVAL_SET_DOUBLE(&rightVal, static_cast<double>(tmpI64));
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
      else
      {
        //处理左右两边都是值的情况，直接比较是否相等
        //若结果相等：忽略条件；若结果不相等：结果集为空
        if (DBVAL_GET_TYPE(&leftVal) != DBVAL_GET_TYPE(&rightVal))
        {
          alwaysFalse_ = true;
        }
        else
        {
          switch (DBVAL_GET_TYPE(&leftVal))
          {
          case PDB_VALUE_TYPE::VAL_BOOL:
            if (DBVAL_GET_BOOL(&leftVal) != DBVAL_GET_BOOL(&rightVal))
            {
              alwaysFalse_ = true;
            }
            break;
          case PDB_VALUE_TYPE::VAL_INT64:
            if (DBVAL_GET_INT64(&leftVal) != DBVAL_GET_INT64(&rightVal))
            {
              alwaysFalse_ = true;
            }
            break;
          }
        }

        return PdbE_OK;
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
  case PDB_VALUE_TYPE::VAL_BOOL:
    return new EqBoolCondition(fieldPos, (!DBVAL_GET_BOOL(pVal)));
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
  case PDB_VALUE_TYPE::VAL_BOOL:
    return new EqBoolCondition(fieldPos, DBVAL_GET_BOOL(pVal));
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
