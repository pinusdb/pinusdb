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

PdbErr_t IResultFilter::AddCountField(const std::string& aliasName)
{
  ResultField* pRetField = new CountFunc();
  return AddResultField(pRetField, aliasName);
}

PdbErr_t IResultFilter::AddAggField(int32_t opFunc, const std::string& aliasName,
  size_t fieldPos, int32_t fieldType, Arena* pArena)
{
  ResultField* pRetField = nullptr;

  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_BOOL:
    pRetField = AddAggBoolField(opFunc, fieldPos);
    break;
  case PDB_FIELD_TYPE::TYPE_INT64:
    pRetField = AddAggInt64Field(opFunc, fieldPos);
    break;
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
  case PDB_FIELD_TYPE::TYPE_REAL2:
  case PDB_FIELD_TYPE::TYPE_REAL3:
  case PDB_FIELD_TYPE::TYPE_REAL4:
  case PDB_FIELD_TYPE::TYPE_REAL6:
    pRetField = AddAggDoubleField(opFunc, fieldPos);
    break;
  case PDB_FIELD_TYPE::TYPE_STRING:
    pRetField = AddAggStringField(opFunc, fieldPos, pArena);
    break;
  case PDB_FIELD_TYPE::TYPE_BLOB:
    pRetField = AddAggBlobField(opFunc, fieldPos, pArena);
    break;
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    pRetField = AddAggDateTimeField(opFunc, fieldPos);
    break;
  }

  if (pRetField == nullptr)
    return PdbE_SQL_RESULT_ERROR;

  return AddResultField(pRetField, aliasName);
}

ResultField* IResultFilter::AddAggBoolField(int32_t opFunc, size_t fieldPos)
{
  switch (opFunc)
  {
  case TK_COUNT_FUNC:
    return new CountFunc();
  case TK_FIRST_FUNC:
    return new FirstValFunc<PDB_FIELD_TYPE::TYPE_BOOL>(fieldPos);
  case TK_LAST_FUNC:
    return new LastValFunc<PDB_FIELD_TYPE::TYPE_BOOL>(fieldPos);
  }

  return nullptr;
}
ResultField* IResultFilter::AddAggInt64Field(int32_t opFunc, size_t fieldPos)
{
  switch (opFunc)
  {
  case TK_AVG_FUNC:
    return new AvgBigIntFunc(fieldPos);
  case TK_COUNT_FUNC:
    return new CountFunc();
  case TK_LAST_FUNC:
    return new LastValFunc<PDB_FIELD_TYPE::TYPE_INT64>(fieldPos);
  case TK_MAX_FUNC:
    return new MaxNumFunc<PDB_FIELD_TYPE::TYPE_INT64>(fieldPos);
  case TK_MIN_FUNC:
    return new MinNumFunc<PDB_FIELD_TYPE::TYPE_INT64>(fieldPos);
  case TK_SUM_FUNC:
    return new SumNumFunc<PDB_FIELD_TYPE::TYPE_INT64>(fieldPos);
  case TK_FIRST_FUNC:
    return new FirstValFunc<PDB_FIELD_TYPE::TYPE_INT64>(fieldPos);
  }

  return nullptr;
}

ResultField* IResultFilter::AddAggDoubleField(int32_t opFunc, size_t fieldPos)
{
  switch (opFunc)
  {
  case TK_AVG_FUNC:
    return new AvgDoubleFunc(fieldPos);
  case TK_COUNT_FUNC:
    return new CountFunc();
  case TK_LAST_FUNC:
    return new LastValFunc<PDB_FIELD_TYPE::TYPE_DOUBLE>(fieldPos);
  case TK_MAX_FUNC:
    return new MaxDoubleFunc(fieldPos);
  case TK_MIN_FUNC:
    return new MinDoubleFunc(fieldPos);
  case TK_SUM_FUNC:
    return new SumDoubleFunc(fieldPos);
  case TK_FIRST_FUNC:
    return new FirstValFunc<PDB_FIELD_TYPE::TYPE_DOUBLE>(fieldPos);
  }

  return nullptr;
}
ResultField* IResultFilter::AddAggDateTimeField(int32_t opFunc, size_t fieldPos)
{
  switch (opFunc)
  {
  case TK_COUNT_FUNC:
    return new CountFunc();
  case TK_LAST_FUNC:
    return new LastValFunc<PDB_FIELD_TYPE::TYPE_DATETIME>(fieldPos);
  case TK_MAX_FUNC:
    return new MaxNumFunc<PDB_FIELD_TYPE::TYPE_DATETIME>(fieldPos);
  case TK_MIN_FUNC:
    return new MinNumFunc<PDB_FIELD_TYPE::TYPE_DATETIME>(fieldPos);
  case TK_FIRST_FUNC:
    return new FirstValFunc<PDB_FIELD_TYPE::TYPE_DATETIME>(fieldPos);
  }

  return nullptr;
}
ResultField* IResultFilter::AddAggStringField(int32_t opFunc, size_t fieldPos, Arena* pArena)
{
  switch (opFunc)
  {
  case TK_COUNT_FUNC:
    return new CountFunc();
  case TK_LAST_FUNC:
    return new LastBlockFunc<PDB_FIELD_TYPE::TYPE_STRING>(fieldPos, pArena);
  case TK_FIRST_FUNC:
    return new FirstBlockFunc<PDB_FIELD_TYPE::TYPE_STRING>(fieldPos, pArena);
  }

  return nullptr;
}
ResultField* IResultFilter::AddAggBlobField(int32_t opFunc, size_t fieldPos, Arena* pArena)
{
  switch (opFunc)
  {
  case TK_COUNT_FUNC:
    return new CountFunc();
  case TK_LAST_FUNC:
    return new LastBlockFunc<PDB_FIELD_TYPE::TYPE_BLOB>(fieldPos, pArena);
  case TK_FIRST_FUNC:
    return new FirstBlockFunc<PDB_FIELD_TYPE::TYPE_BLOB>(fieldPos, pArena);
  }

  return nullptr;
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
    else
    {
      //聚合函数
      const std::string aliasName = (*colItem)->GetAliasName();
      if (aliasName.size() == 0)
        return PdbE_SQL_LOST_ALIAS;

      const ExprList* pFuncArgs = (*colItem)->GetExprList();
      if (pFuncArgs == nullptr)
        return PdbE_SQL_ERROR;

      const std::vector<ExprItem*> argList = pFuncArgs->GetExprList();
      if (argList.size() != 1) //目前所有聚合函数只有一个参数
        return PdbE_SQL_ERROR;

      const std::string& fieldName = argList[0]->GetValueStr();

      if (fieldName.compare("*") == 0 && (*colItem)->GetOp() == TK_COUNT_FUNC)
      {
        retVal = AddCountField(aliasName);
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

        retVal = AddAggField((*colItem)->GetOp(), aliasName, fieldPos, fieldType, pArena);
        if (retVal != PdbE_OK)
          return retVal;
      }
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
