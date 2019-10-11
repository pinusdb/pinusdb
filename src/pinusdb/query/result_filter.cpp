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

#include "query/result_filter.h"
#include "query/condition_filter.h"
#include "expr/parse.h"
#include "pdb_error.h"

#include "query/agg_avg_function.h"
#include "query/agg_count_function.h"
#include "query/agg_last_function.h"
#include "query/agg_max_function.h"
#include "query/agg_min_function.h"
#include "query/agg_sum_function.h"
#include "query/agg_first_function.h"
#include "query/agg_devid.h"
#include "query/agg_tstamp.h"
#include "query/raw_field.h"

#include <algorithm>

IResultFilter::IResultFilter()
{
  isEmptySet_ = false;
  isFull_ = false;

  queryOffset_ = 0;
  queryRecord_ = PDB_QUERY_DEFAULT_COUNT;
}

IResultFilter::~IResultFilter()
{
  for (auto fieldIter = fieldVec_.begin(); fieldIter != fieldVec_.end(); fieldIter++)
  {
    delete *fieldIter;
  }

  for (auto objIter = objVec_.begin(); objIter != objVec_.end(); objIter++)
  {
    delete *objIter;
  }
}

PdbErr_t IResultFilter::GetData(DataTable* pDataTable)
{
  PdbErr_t retVal = PdbE_OK;

  if (pDataTable == nullptr)
    return PdbE_INVALID_PARAM;

  int32_t fieldType = 0;
  const char* pFieldName = nullptr;

  size_t fieldCnt = tabInfo_.GetFieldCnt();
  for (size_t i = 0; i < fieldCnt; i++)
  {
    tabInfo_.GetFieldInfo(i, &fieldType);
    pFieldName = tabInfo_.GetFieldName(i);

    retVal = pDataTable->AddColumn(pFieldName, fieldType);
    if (retVal != PdbE_OK)
      return retVal;
  }

  DBObj* pTmpObj = nullptr;

  for (auto objIter = objVec_.begin(); objIter != objVec_.end(); objIter++)
  {
    pTmpObj = new DBObj(nullptr);
    if (pTmpObj == nullptr)
      return PdbE_NOMEM;

    retVal = (*objIter)->GetResultObj(pTmpObj);
    if (retVal != PdbE_OK)
    {
      delete pTmpObj;
      return retVal;
    }

    pDataTable->AppendData(pTmpObj);
  }

  return PdbE_OK;
}

PdbErr_t IResultFilter::BuildFilter(const QueryParam* pQueryParam, const TableInfo* pTabInfo, Arena* pArena)
{
  PdbErr_t retVal = PdbE_OK;

  const ExprItem* pConditionItem = pQueryParam->pWhere_;
  const LimitOpt* pLimit = pQueryParam->pLimit_;

  if (pLimit != nullptr)
  {
    retVal = pLimit->Valid();
    if (retVal != PdbE_OK)
      return retVal;

    queryOffset_ = pLimit->GetOffset();
    queryRecord_ = pLimit->GetQueryCnt();
  }

  retVal = this->condiFilter_.BuildCondition(pConditionItem, pTabInfo);
  if (retVal != PdbE_OK)
    return retVal;

  retVal = BuildCustomFilter(pQueryParam, pTabInfo, pArena);
  if (retVal != PdbE_OK)
    return retVal;

  return retVal;
}

PdbErr_t IResultFilter::AddCountField(const std::string& aliasName, size_t fieldPos)
{
  ResultField* pRetField = new CountFunc(fieldPos);
  return AddResultField(pRetField, aliasName);
}

PdbErr_t IResultFilter::AddAggField(int funcId, const std::string& aliasName, 
  size_t fieldPos, int32_t fieldType, Arena* pArena)
{
  ResultField* pRetField = nullptr;

  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_BOOL:
    pRetField = AddAggBoolField(funcId, fieldPos);
    break;
  case PDB_FIELD_TYPE::TYPE_INT64:
    pRetField = AddAggInt64Field(funcId, fieldPos);
    break;
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
  case PDB_FIELD_TYPE::TYPE_REAL2:
  case PDB_FIELD_TYPE::TYPE_REAL3:
  case PDB_FIELD_TYPE::TYPE_REAL4:
  case PDB_FIELD_TYPE::TYPE_REAL6:
    pRetField = AddAggDoubleField(funcId, fieldPos);
    break;
  case PDB_FIELD_TYPE::TYPE_STRING:
    pRetField = AddAggStringField(funcId, fieldPos, pArena);
    break;
  case PDB_FIELD_TYPE::TYPE_BLOB:
    pRetField = AddAggBlobField(funcId, fieldPos, pArena);
    break;
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    pRetField = AddAggDateTimeField(funcId, fieldPos);
    break;
  }

  if (pRetField == nullptr)
    return PdbE_SQL_RESULT_ERROR;

  return AddResultField(pRetField, aliasName);
}

ResultField* IResultFilter::AddAggBoolField(int funcId, size_t fieldPos)
{
  switch (funcId)
  {
  case PDB_SQL_FUNC::FUNC_COUNT:
    return new CountFunc(fieldPos);
  case PDB_SQL_FUNC::FUNC_FIRST:
    return new FirstValFunc<PDB_FIELD_TYPE::TYPE_BOOL>(fieldPos);
  case PDB_SQL_FUNC::FUNC_LAST:
    return new LastValFunc<PDB_FIELD_TYPE::TYPE_BOOL>(fieldPos);
  }

  return nullptr;
}
ResultField* IResultFilter::AddAggInt64Field(int funcId, size_t fieldPos)
{
  switch (funcId)
  {
  case PDB_SQL_FUNC::FUNC_COUNT:
    return new CountFunc(fieldPos);
  case PDB_SQL_FUNC::FUNC_FIRST:
    return new FirstValFunc<PDB_FIELD_TYPE::TYPE_INT64>(fieldPos);
  case PDB_SQL_FUNC::FUNC_LAST:
    return new LastValFunc<PDB_FIELD_TYPE::TYPE_INT64>(fieldPos);
  case PDB_SQL_FUNC::FUNC_AVG:
    return new AvgBigIntFunc(fieldPos);
  case PDB_SQL_FUNC::FUNC_MIN:
    return new MinNumFunc<PDB_FIELD_TYPE::TYPE_INT64>(fieldPos);
  case PDB_SQL_FUNC::FUNC_MAX:
    return new MaxNumFunc<PDB_FIELD_TYPE::TYPE_INT64>(fieldPos);
  case PDB_SQL_FUNC::FUNC_SUM:
    return new SumNumFunc(fieldPos);
  }

  return nullptr;
}

ResultField* IResultFilter::AddAggDoubleField(int funcId, size_t fieldPos)
{
  switch (funcId)
  {
  case PDB_SQL_FUNC::FUNC_COUNT:
    return new CountFunc(fieldPos);
  case PDB_SQL_FUNC::FUNC_FIRST:
    return new FirstValFunc<PDB_FIELD_TYPE::TYPE_DOUBLE>(fieldPos);
  case PDB_SQL_FUNC::FUNC_LAST:
    return new LastValFunc<PDB_FIELD_TYPE::TYPE_DOUBLE>(fieldPos);
  case PDB_SQL_FUNC::FUNC_AVG:
    return new AvgDoubleFunc(fieldPos);
  case PDB_SQL_FUNC::FUNC_MIN:
    return new MinDoubleFunc(fieldPos);
  case PDB_SQL_FUNC::FUNC_MAX:
    return new MaxDoubleFunc(fieldPos);
  case PDB_SQL_FUNC::FUNC_SUM:
    return new SumDoubleFunc(fieldPos);
  }

  return nullptr;
}
ResultField* IResultFilter::AddAggDateTimeField(int funcId, size_t fieldPos)
{
  switch (funcId)
  {
  case PDB_SQL_FUNC::FUNC_COUNT:
    return new CountFunc(fieldPos);
  case PDB_SQL_FUNC::FUNC_FIRST:
    return new FirstValFunc<PDB_FIELD_TYPE::TYPE_DATETIME>(fieldPos);
  case PDB_SQL_FUNC::FUNC_LAST:
    return new LastValFunc<PDB_FIELD_TYPE::TYPE_DATETIME>(fieldPos);
  case PDB_SQL_FUNC::FUNC_MIN:
    return new MinNumFunc<PDB_FIELD_TYPE::TYPE_DATETIME>(fieldPos);
  case PDB_SQL_FUNC::FUNC_MAX:
    return new MaxNumFunc<PDB_FIELD_TYPE::TYPE_DATETIME>(fieldPos);
  }

  return nullptr;
}
ResultField* IResultFilter::AddAggStringField(int funcId, size_t fieldPos, Arena* pArena)
{
  switch (funcId)
  {
  case PDB_SQL_FUNC::FUNC_COUNT:
    return new CountFunc(fieldPos);
  case PDB_SQL_FUNC::FUNC_FIRST:
    return new FirstBlockFunc<PDB_FIELD_TYPE::TYPE_STRING>(fieldPos, pArena);
  case PDB_SQL_FUNC::FUNC_LAST:
    return new LastBlockFunc<PDB_FIELD_TYPE::TYPE_STRING>(fieldPos, pArena);
  }

  return nullptr;
}
ResultField* IResultFilter::AddAggBlobField(int funcId, size_t fieldPos, Arena* pArena)
{
  switch (funcId)
  {
  case PDB_SQL_FUNC::FUNC_COUNT:
    return new CountFunc(fieldPos);
  case PDB_SQL_FUNC::FUNC_FIRST:
    return new FirstBlockFunc<PDB_FIELD_TYPE::TYPE_BLOB>(fieldPos, pArena);
  case PDB_SQL_FUNC::FUNC_LAST:
    return new LastBlockFunc<PDB_FIELD_TYPE::TYPE_BLOB>(fieldPos, pArena);
  }

  return nullptr;
}

int IResultFilter::GetFuncIdByName(const std::string& funcName)
{
  if (StringTool::ComparyNoCase(funcName, PDB_SQL_FUNC_COUNT_NAME, PDB_SQL_FUNC_COUNT_NAME_LEN))
    return PDB_SQL_FUNC::FUNC_COUNT;
  else if (StringTool::ComparyNoCase(funcName, PDB_SQL_FUNC_FIRST_NAME, PDB_SQL_FUNC_FIRST_NAME_LEN))
    return PDB_SQL_FUNC::FUNC_FIRST;
  else if (StringTool::ComparyNoCase(funcName, PDB_SQL_FUNC_LAST_NAME, PDB_SQL_FUNC_LAST_NAME_LEN))
    return PDB_SQL_FUNC::FUNC_LAST;
  else if (StringTool::ComparyNoCase(funcName, PDB_SQL_FUNC_AVG_NAME, PDB_SQL_FUNC_AVG_NAME_LEN))
    return PDB_SQL_FUNC::FUNC_AVG;
  else if (StringTool::ComparyNoCase(funcName, PDB_SQL_FUNC_MIN_NAME, PDB_SQL_FUNC_MIN_NAME_LEN))
    return PDB_SQL_FUNC::FUNC_MIN;
  else if (StringTool::ComparyNoCase(funcName, PDB_SQL_FUNC_MAX_NAME, PDB_SQL_FUNC_MAX_NAME_LEN))
    return PDB_SQL_FUNC::FUNC_MAX;
  else if (StringTool::ComparyNoCase(funcName, PDB_SQL_FUNC_SUM_NAME, PDB_SQL_FUNC_SUM_NAME_LEN))
    return PDB_SQL_FUNC::FUNC_SUM;

  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////

PdbErr_t IResultFilter::AddRawField(size_t fieldPos, const std::string& aliasName,
  int32_t fieldType, Arena* pArena)
{
  ResultField* pRetField = nullptr;

  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_BOOL:
    pRetField = new RawValField<PDB_FIELD_TYPE::TYPE_BOOL>(fieldPos);
    break;
  case PDB_FIELD_TYPE::TYPE_INT64:
    pRetField = new RawValField<PDB_FIELD_TYPE::TYPE_INT64>(fieldPos);
    break;
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
  case PDB_FIELD_TYPE::TYPE_REAL2:
  case PDB_FIELD_TYPE::TYPE_REAL3:
  case PDB_FIELD_TYPE::TYPE_REAL4:
  case PDB_FIELD_TYPE::TYPE_REAL6:
    pRetField = new RawValField<PDB_FIELD_TYPE::TYPE_DOUBLE>(fieldPos);
    break;
  case PDB_FIELD_TYPE::TYPE_STRING:
    pRetField = new RawBlockField<PDB_FIELD_TYPE::TYPE_STRING>(fieldPos, pArena);
    break;
  case PDB_FIELD_TYPE::TYPE_BLOB:
    pRetField = new RawBlockField<PDB_FIELD_TYPE::TYPE_BLOB>(fieldPos, pArena);
    break;
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    pRetField = new RawValField<PDB_FIELD_TYPE::TYPE_DATETIME>(fieldPos);
    break;
  }

  if (pRetField == nullptr)
    return PdbE_SQL_RESULT_ERROR;

  return AddResultField(pRetField, aliasName);
}


PdbErr_t IResultFilter::AddAggTStampField(const std::string& aliasName)
{
  ResultField* pRetField = new GroupTstampField(0);
  return AddResultField(pRetField, aliasName);
}

PdbErr_t IResultFilter::AddAggDevIdField(const std::string& aliasName)
{
  ResultField* pRetField = new GroupDevIdField(0);
  return AddResultField(pRetField, aliasName);
}

PdbErr_t IResultFilter::BuildGroupResultField(const std::vector<ExprItem*>& colItemVec,
  const TableInfo* pTabInfo, const GroupOpt* pGroup, Arena* pArena)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  size_t fieldPos = 0;

  fieldVec_.clear();

  for (auto colItem = colItemVec.begin(); colItem != colItemVec.end(); colItem++)
  {
    if ((*colItem)->GetOp() == TK_ID)
    {
      //如果在聚合查询中出现的字段名一定要在 分组中
      if (pGroup == nullptr)
      {
        //将所有数据分为一组的情况，不能出现字段名
        return PdbE_SQL_RESULT_ERROR;
      }

      std::string aliasName = (*colItem)->GetAliasName();
      if (aliasName.size() == 0)
        aliasName = (*colItem)->GetValueStr();

      const std::string& fieldName = (*colItem)->GetValueStr();
      if (StringTool::ComparyNoCase(fieldName.c_str(), DEVID_FIELD_NAME)
        && pGroup->IsDevIdGroup())
      {
        retVal = AddAggDevIdField(aliasName);
        if (retVal != PdbE_OK)
          return retVal;
      }
      else if (StringTool::ComparyNoCase(fieldName.c_str(), TSTAMP_FIELD_NAME)
        && pGroup->IsTStampGroup())
      {
        retVal = AddAggTStampField(aliasName);
        if (retVal != PdbE_OK)
          return retVal;
      }
      else
      {
        return PdbE_SQL_RESULT_ERROR;
      }
    }
    else if ((*colItem)->GetOp() == TK_FUNCTION)
    {
      //聚合函数
      const std::string& funcName = (*colItem)->GetFuncName();
      int funcId = GetFuncIdByName(funcName);

      const std::string& aliasName = (*colItem)->GetAliasName();
      if (aliasName.size() == 0)
        return PdbE_SQL_LOST_ALIAS;

      const ExprList* pFuncArgs = (*colItem)->GetExprList();
      if (pFuncArgs == nullptr)
        return PdbE_SQL_ERROR;

      const std::vector<ExprItem*> argList = pFuncArgs->GetExprList();
      if (argList.size() != 1) //目前所有聚合函数只有一个参数
        return PdbE_SQL_ERROR;

      const std::string& fieldName = argList[0]->GetValueStr();

      if (fieldName.compare("*") == 0  && funcId == PDB_SQL_FUNC::FUNC_COUNT )
      {
        retVal = AddCountField(aliasName, 0);
        if (retVal != PdbE_OK)
          return retVal;
      }
      else
      {
        retVal = pTabInfo->GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
        if (retVal != PdbE_OK)
          return retVal;

        if (pGroup != nullptr)
        {
          //group by objectname 的查询，objectname不能出现在聚合函数中
          if (fieldPos == PDB_DEVID_INDEX
            && pGroup->IsDevIdGroup())
            return PdbE_SQL_RESULT_ERROR;

          //group by timestamp 的查询，timestamp不能出现在聚合函数中
          if (fieldPos == PDB_TSTAMP_INDEX
            && pGroup->IsTStampGroup())
            return PdbE_SQL_RESULT_ERROR;
        }

        retVal = AddAggField(funcId, aliasName, fieldPos, fieldType, pArena);
        if (retVal != PdbE_OK)
          return retVal;
      }
    }
    else
    {
      return PdbE_SQL_ERROR;
    }
  }

  return PdbE_OK;

}

PdbErr_t IResultFilter::AddResultField(ResultField* pField, const std::string& aliasName)
{
  fieldVec_.push_back(pField);
  int32_t resultType = pField->FieldType();
  return tabInfo_.AddField(aliasName.c_str(), resultType);
}
