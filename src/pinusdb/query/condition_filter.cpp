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
  minDevId_ = 1;
  maxDevId_ = INT64_MAX;
  minTstamp_ = MinMillis;
  maxTstamp_ = MaxMillis;
}

ConditionFilter::~ConditionFilter()
{
  //devIdCondiVec_ 中的内容已包含在 conditionVec_ 中
  for (auto conIt = conditionVec_.begin(); conIt != conditionVec_.end(); conIt++)
  {
    delete *conIt;
  }
}

PdbErr_t ConditionFilter::BuildCondition(const TableInfo* pTabInfo, 
  const ExprValue* pCondition, int64_t nowMillis)
{
  PdbErr_t retVal = PdbE_OK;
  DBVal result;
  int64_t tmpMin, tmpMax;

  if (pTabInfo == nullptr)
    return PdbE_INVALID_PARAM;

  if (pCondition == nullptr)
    return PdbE_OK;

  if (pCondition->GetValueType() == TK_AND)
  {
    const ExprValue* pLeftExpr = pCondition->GetLeftParam();
    const ExprValue* pRightExpr = pCondition->GetRightParam();

    if (pLeftExpr == nullptr || pRightExpr == nullptr)
      return PdbE_SQL_CONDITION_EXPR_ERROR;

    retVal = BuildCondition(pTabInfo, pLeftExpr, nowMillis);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = BuildCondition(pTabInfo, pRightExpr, nowMillis);
    if (retVal != PdbE_OK)
      return retVal;
  }
  else
  {
    ValueItem* pConditionItem = BuildGeneralValueItem(pTabInfo, pCondition, nowMillis);
    if (pConditionItem == nullptr)
      return PdbE_SQL_CONDITION_EXPR_ERROR;

    conditionVec_.push_back(pConditionItem);
    if (pConditionItem->GetValueType() != PDB_VALUE_TYPE::VAL_BOOL)
      return PdbE_SQL_CONDITION_EXPR_ERROR;

    if (pConditionItem->IsConstValue())
    {
      if (pConditionItem->GetValue(nullptr, &result) == PdbE_OK)
      {
        if (DBVAL_IS_BOOL(&result) && (!DBVAL_GET_BOOL(&result)))
        {
          alwaysFalse_ = true;
        }
      }
    }

    if (pConditionItem->IsDevIdCondition())
    {
      devIdCondiVec_.push_back(pConditionItem);
      if (pConditionItem->GetDevIdRange(&tmpMin, &tmpMax))
      {
        if (minDevId_ <= tmpMin)
          minDevId_ = tmpMin;

        if (maxDevId_ >= tmpMax)
          maxDevId_ = tmpMax;
      }
    }

    if (pConditionItem->IsTstampCondition())
    {
      if (pConditionItem->GetTstampRange(&tmpMin, &tmpMax))
      {
        if (minTstamp_ <= tmpMin)
          minTstamp_ = tmpMin;

        if (maxTstamp_ >= tmpMax)
          maxTstamp_ = tmpMax;
      }
    }
  }

  return PdbE_OK;
}

PdbErr_t ConditionFilter::RunCondition(
  const DBVal* pVals, size_t valCnt, bool& resultVal) const
{
  PdbErr_t retVal = PdbE_OK;
  DBVal result;
  for (auto condiIt = conditionVec_.begin(); condiIt != conditionVec_.end(); condiIt++)
  {
    retVal = (*condiIt)->GetValue(pVals, &result);
    if (retVal != PdbE_OK)
    {
      resultVal = false;
      return retVal;
    }

    if ((!DBVAL_IS_BOOL(&result)) || (!DBVAL_GET_BOOL(&result)))
    {
      resultVal = false;
      return PdbE_OK;
    } 
  }

  resultVal = true;
  return PdbE_OK;
}

void ConditionFilter::GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const
{
  if (pMinDevId != nullptr)
    *pMinDevId = minDevId_;

  if (pMaxDevId != nullptr)
    *pMaxDevId = maxDevId_;
}

void ConditionFilter::GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const
{
  if (pMinTstamp != nullptr)
    *pMinTstamp = minTstamp_;

  if (pMaxTstamp != nullptr)
    *pMaxTstamp = maxTstamp_;
}

bool ConditionFilter::FilterDevId(int64_t devId) const
{
  PdbErr_t retVal = PdbE_OK;
  DBVal tmpVal, result;
  DBVAL_SET_INT64(&tmpVal, devId);
  for (auto condiIt = devIdCondiVec_.begin(); condiIt != devIdCondiVec_.end(); condiIt++)
  {
    retVal = (*condiIt)->GetValue(&tmpVal, &result);
    if (retVal != PdbE_OK)
      return false;
    
    if ((!DBVAL_IS_BOOL(&result)) || (!DBVAL_GET_BOOL(&result)))    
      return false;
  }

  return true;
}
