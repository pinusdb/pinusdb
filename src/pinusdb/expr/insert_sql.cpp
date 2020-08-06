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

#include "expr/insert_sql.h"
#include "expr/parse.h"
#include "expr/sql_parser.h"
#include "util/string_tool.h"
#include "util/date_time.h"
#include <math.h>

#ifdef _WIN32
#define CONVERT_TO_REAL_VALUE(pVal, kMultiple) do { \
  if(DBVAL_IS_INT64(pVal) && DBVAL_GET_INT64(pVal) >= PDB_MIN_REAL_VALUE && DBVAL_GET_INT64(pVal) <= PDB_MAX_REAL_VALUE) \
  { \
    DBVAL_SET_INT64(pVal, (DBVAL_GET_INT64(pVal) * kMultiple)); \
  } \
  else if (DBVAL_IS_DOUBLE(pVal) && DBVAL_GET_DOUBLE(pVal) >= PDB_MIN_REAL_VALUE && DBVAL_GET_DOUBLE(pVal) <= PDB_MAX_REAL_VALUE) \
  { \
    DBVAL_SET_INT64(pVal, static_cast<int64_t>(std::round(DBVAL_GET_DOUBLE(pVal) * kMultiple))); \
  } \
  else \
  { \
    return PdbE_VALUE_MISMATCH;  \
  } \
} while(false)
#else
#define CONVERT_TO_REAL_VALUE(pVal, kMultiple) do { \
  if(DBVAL_IS_INT64(pVal) && DBVAL_GET_INT64(pVal) >= PDB_MIN_REAL_VALUE && DBVAL_GET_INT64(pVal) <= PDB_MAX_REAL_VALUE) \
  { \
    DBVAL_SET_INT64(pVal, (DBVAL_GET_INT64(pVal) * kMultiple)); \
  } \
  else if (DBVAL_IS_DOUBLE(pVal) && DBVAL_GET_DOUBLE(pVal) >= PDB_MIN_REAL_VALUE && DBVAL_GET_DOUBLE(pVal) <= PDB_MAX_REAL_VALUE) \
  { \
    DBVAL_SET_INT64(pVal, static_cast<int64_t>(round(DBVAL_GET_DOUBLE(pVal) * kMultiple))); \
  } \
  else \
  { \
    return PdbE_VALUE_MISMATCH;  \
  } \
} while(false)
#endif

PdbErr_t ConvertDBVal(DBVal* pVal, int valType) 
{
  int64_t intVal = 0;
  switch (valType)
  {
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    if (DBVAL_IS_STRING(pVal))
    {
      if (!DateTime::Parse(DBVAL_GET_STRING(pVal), DBVAL_GET_LEN(pVal), &intVal))
        return PdbE_INVALID_DATETIME_VAL;

      DBVAL_SET_DATETIME(pVal, intVal);
    }
    else if (DBVAL_IS_INT64(pVal) && DBVAL_GET_INT64(pVal) >= MinMillis && DBVAL_GET_INT64(pVal) <= MaxMillis)
    {
      DBVAL_SET_DATETIME(pVal, DBVAL_GET_INT64(pVal));
    }
    else
    {
      return PdbE_VALUE_MISMATCH;
    }
    break;
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    if (DBVAL_IS_INT64(pVal))
    {
      DBVAL_SET_DOUBLE(pVal, static_cast<double>(DBVAL_GET_INT64(pVal)));
    }
    else
    {
      return PdbE_VALUE_MISMATCH;
    }
    break;
  case PDB_FIELD_TYPE::TYPE_REAL2:
    CONVERT_TO_REAL_VALUE(pVal, DBVAL_REAL2_MULTIPLE);
    break;
  case PDB_FIELD_TYPE::TYPE_REAL3:
    CONVERT_TO_REAL_VALUE(pVal, DBVAL_REAL3_MULTIPLE);
    break;
  case PDB_FIELD_TYPE::TYPE_REAL4:
    CONVERT_TO_REAL_VALUE(pVal, DBVAL_REAL4_MULTIPLE);
    break;
  case PDB_FIELD_TYPE::TYPE_REAL6:
    CONVERT_TO_REAL_VALUE(pVal, DBVAL_REAL6_MULTIPLE);
    break;
  default:
    return PdbE_VALUE_MISMATCH;
  }

  return PdbE_OK;
}


InsertSql::InsertSql()
{
  fieldCnt_ = 0;
  recCnt_ = 0;
}

InsertSql::~InsertSql()
{
}

void InsertSql::SetTableName(const char* pName, size_t len)
{
  tabName_ = std::string(pName, len);
}

void InsertSql::SetTableName(const std::string& tabName)
{
  tabName_ = tabName;
}

void InsertSql::AppendFieldName(const char* pName, size_t len)
{
  colNameVec_.push_back(std::string(pName, len));
}

void InsertSql::AppendFieldName(const std::string& fieldName)
{
  colNameVec_.push_back(fieldName);
}

PdbErr_t InsertSql::AppendVal(const DBVal* pVal)
{
  valList_.push_back(*pVal);
  return PdbE_OK;
}

bool InsertSql::Valid() const
{
  return valList_.size() == fieldCnt_ * recCnt_;
}

const std::string& InsertSql::GetTableName() const
{
  return tabName_;
}

PdbErr_t InsertSql::InitTableInfo(const TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  size_t fieldPos = 0;
  posVec_.clear();
  typeVec_.clear();
  
  for (auto nameIt = colNameVec_.begin(); nameIt != colNameVec_.end(); nameIt++)
  {
    retVal = pTabInfo->GetFieldRealInfo(nameIt->c_str(), &fieldPos, &fieldType);
    if (retVal != PdbE_OK)
      return retVal;

    posVec_.push_back(fieldPos);
    typeVec_.push_back(fieldType);
  }
  
  valIter_ = valList_.begin();
  return PdbE_OK;
}

bool InsertSql::IsEnd() const
{
  return valIter_ == valList_.end();
}

PdbErr_t InsertSql::GetNextRec(DBVal* pVals, size_t valCnt)
{
  PdbErr_t retVal = PdbE_OK;
  DBVal val;

  for (size_t idx = 0; idx < fieldCnt_; idx++)
  {
    size_t pos = posVec_[idx];
    if (valIter_->dataType_ == PDB_VALUE_TYPE::VAL_NULL)
    {
      switch (typeVec_[idx])
      {
      case PDB_FIELD_TYPE::TYPE_BOOL:
        DBVAL_ELE_SET_BOOL(pVals, pos, false);
        break;
      case PDB_FIELD_TYPE::TYPE_INT64:
      case PDB_FIELD_TYPE::TYPE_REAL2:
      case PDB_FIELD_TYPE::TYPE_REAL3:
      case PDB_FIELD_TYPE::TYPE_REAL4:
      case PDB_FIELD_TYPE::TYPE_REAL6:
        DBVAL_ELE_SET_INT64(pVals, pos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_DATETIME:
        DBVAL_ELE_SET_DATETIME(pVals, pos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_DOUBLE:
        DBVAL_ELE_SET_DOUBLE(pVals, pos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_STRING:
        DBVAL_ELE_SET_STRING(pVals, pos, nullptr, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_BLOB:
        DBVAL_ELE_SET_BLOB(pVals, pos, nullptr, 0);
        break;
      }
    }
    else if (valIter_->dataType_ == typeVec_[idx])
    {
      pVals[pos] = *valIter_;
    }
    else
    {
      val = *valIter_;
      if (ConvertDBVal(&val, typeVec_[idx]) != PdbE_OK)
        retVal = PdbE_VALUE_MISMATCH;

      pVals[pos] = val;
    }

    valIter_++;
  }

  return retVal;
}
