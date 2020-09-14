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
#include "util/coding.h"
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
  case PDB_FIELD_TYPE::TYPE_INT8:
    if (DBVAL_IS_INT64(pVal) && DBVAL_GET_INT64(pVal) >= INT8_MIN && DBVAL_GET_INT64(pVal) <= INT8_MAX)
      DBVAL_SET_INT8(pVal, static_cast<int8_t>(DBVAL_GET_INT64(pVal)));
    else
      return PdbE_VALUE_MISMATCH;
    break;
  case PDB_FIELD_TYPE::TYPE_INT16:
    if (DBVAL_IS_INT64(pVal) && DBVAL_GET_INT64(pVal) >= INT16_MIN && DBVAL_GET_INT64(pVal) <= INT16_MAX)
      DBVAL_SET_INT16(pVal, static_cast<int16_t>(DBVAL_GET_INT64(pVal)));
    else
      return PdbE_VALUE_MISMATCH;
    break;
  case PDB_FIELD_TYPE::TYPE_INT32:
    if (DBVAL_IS_INT64(pVal) && DBVAL_GET_INT64(pVal) >= INT32_MIN && DBVAL_GET_INT64(pVal) <= INT32_MAX)
      DBVAL_SET_INT32(pVal, static_cast<int32_t>(DBVAL_GET_INT64(pVal)));
    else
      return PdbE_VALUE_MISMATCH;
    break;
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    if (DBVAL_IS_STRING(pVal))
    {
      if (!DateTime::Parse(DBVAL_GET_STRING(pVal), DBVAL_GET_LEN(pVal), &intVal))
        return PdbE_INVALID_DATETIME_VAL;

      DBVAL_SET_DATETIME(pVal, intVal);
    }
    else if (DBVAL_IS_INT64(pVal) && DBVAL_GET_INT64(pVal) >= DateTime::MinMicrosecond && DBVAL_GET_INT64(pVal) <= DateTime::MaxMicrosecond)
    {
      DBVAL_SET_DATETIME(pVal, DBVAL_GET_INT64(pVal));
    }
    else
    {
      return PdbE_VALUE_MISMATCH;
    }
    break;
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    if (DBVAL_IS_INT64(pVal))
    {
      DBVAL_SET_FLOAT(pVal, static_cast<float>(DBVAL_GET_INT64(pVal)));
    }
    else if (DBVAL_IS_DOUBLE(pVal))
    {
      DBVAL_SET_FLOAT(pVal, static_cast<float>(DBVAL_GET_DOUBLE(pVal)));
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
  fixedSize_ = 0;
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
  size_t storePos = 0;
  fieldVec_.clear();
  storeVec_.clear();
  typeVec_.clear();
  fixedSize_ = pTabInfo->GetFixedSize();
  
  for (auto nameIt = colNameVec_.begin(); nameIt != colNameVec_.end(); nameIt++)
  {
    retVal = pTabInfo->GetFieldRealInfo(nameIt->c_str(), &fieldPos, &fieldType, &storePos);
    if (retVal != PdbE_OK)
      return retVal;

    fieldVec_.push_back(fieldPos);
    storeVec_.push_back(storePos);
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
    size_t fieldPos = fieldVec_[idx];
    if (valIter_->dataType_ == PDB_VALUE_TYPE::VAL_NULL)
    {
      switch (typeVec_[idx])
      {
      case PDB_FIELD_TYPE::TYPE_BOOL:
        DBVAL_ELE_SET_BOOL(pVals, fieldPos, false);
        break;
      case PDB_FIELD_TYPE::TYPE_INT8:
        DBVAL_ELE_SET_INT8(pVals, fieldPos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_INT16:
        DBVAL_ELE_SET_INT16(pVals, fieldPos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_INT32:
        DBVAL_ELE_SET_INT32(pVals, fieldPos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_INT64:
      case PDB_FIELD_TYPE::TYPE_REAL2:
      case PDB_FIELD_TYPE::TYPE_REAL3:
      case PDB_FIELD_TYPE::TYPE_REAL4:
      case PDB_FIELD_TYPE::TYPE_REAL6:
        DBVAL_ELE_SET_INT64(pVals, fieldPos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_DATETIME:
        DBVAL_ELE_SET_DATETIME(pVals, fieldPos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_FLOAT:
        DBVAL_ELE_SET_FLOAT(pVals, fieldPos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_DOUBLE:
        DBVAL_ELE_SET_DOUBLE(pVals, fieldPos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_STRING:
        DBVAL_ELE_SET_STRING(pVals, fieldPos, nullptr, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_BLOB:
        DBVAL_ELE_SET_BLOB(pVals, fieldPos, nullptr, 0);
        break;
      }
    }
    else if (valIter_->dataType_ == typeVec_[idx])
    {
      pVals[fieldPos] = *valIter_;
    }
    else
    {
      val = *valIter_;
      if (ConvertDBVal(&val, typeVec_[idx]) != PdbE_OK)
        retVal = PdbE_VALUE_MISMATCH;

      pVals[fieldPos] = val;
    }

    valIter_++;
  }

  return retVal;
}

PdbErr_t InsertSql::GetNextRecBinary(std::string& buf, int64_t& devId, int64_t& tstamp)
{
  PdbErr_t retVal = PdbE_OK;
  DBVal val;

  buf.clear();
  buf.reserve(PDB_MAX_REC_LEN);
  buf.resize(fixedSize_);
  char* pBuf = &buf[0];
  memset(pBuf, 0, fixedSize_);

  devId = 0;
  tstamp = 0;

  for (size_t idx = 0; idx < fieldCnt_; idx++, valIter_++)
  {
    size_t storePos = storeVec_[idx];
    if (valIter_->dataType_ != PDB_VALUE_TYPE::VAL_NULL && retVal == PdbE_OK)
    {
      val = *valIter_;
      if (DBVAL_GET_TYPE(&val) != typeVec_[idx])
      {
        if (ConvertDBVal(&val, typeVec_[idx]) != PdbE_OK)
        {
          retVal = PdbE_VALUE_MISMATCH;
          continue;
        }
      }

      if (fieldVec_[idx] == PDB_DEVID_INDEX)
      {
        devId = DBVAL_GET_INT64(&val);
        continue;
      }

      if (fieldVec_[idx] == PDB_TSTAMP_INDEX)
      {
        tstamp = DBVAL_GET_DATETIME(&val);
      }

      switch (DBVAL_GET_TYPE(&val))
      {
      case PDB_VALUE_TYPE::VAL_BOOL:
        pBuf[storePos] = DBVAL_GET_BOOL(&val) ? PDB_BOOL_TRUE : PDB_BOOL_FALSE;
        break;
      case PDB_VALUE_TYPE::VAL_INT8:
        pBuf[storePos] = DBVAL_GET_INT8(&val);
        break;
      case PDB_VALUE_TYPE::VAL_INT16:
        Coding::FixedEncode16(pBuf + storePos, DBVAL_GET_INT16(&val));
        break;
      case PDB_VALUE_TYPE::VAL_INT32:
        Coding::FixedEncode32(pBuf + storePos, DBVAL_GET_INT32(&val));
        break;
      case PDB_VALUE_TYPE::VAL_INT64:
        Coding::FixedEncode64(pBuf + storePos, DBVAL_GET_INT64(&val));
        break;
      case PDB_VALUE_TYPE::VAL_DATETIME:
        Coding::FixedEncode64(pBuf + storePos, DBVAL_GET_DATETIME(&val));
        break;
      case PDB_VALUE_TYPE::VAL_FLOAT:
        Coding::FixedEncode32(pBuf + storePos, Coding::EncodeFloat(DBVAL_GET_FLOAT(&val)));
        break;
      case PDB_VALUE_TYPE::VAL_DOUBLE:
        Coding::FixedEncode64(pBuf + storePos, Coding::EncodeDouble(DBVAL_GET_DOUBLE(&val)));
        break;
      case PDB_VALUE_TYPE::VAL_STRING:
      case PDB_VALUE_TYPE::VAL_BLOB:
        if (DBVAL_GET_LEN(&val) > 0)
        {
          if (buf.size() + DBVAL_GET_LEN(&val) + 2 >= PDB_MAX_REC_LEN)
          {
            retVal = PdbE_RECORD_TOO_LONG;
          }
          else
          {
            Coding::FixedEncode16(pBuf + storePos, static_cast<uint16_t>(buf.size()));
            Coding::PutVarint64(&buf, DBVAL_GET_LEN(&val));
            buf.append(DBVAL_GET_STRING(&val), DBVAL_GET_LEN(&val));
          }
        }
        break;
      }
    }
  }

  Coding::FixedEncode16(pBuf, static_cast<uint16_t>(buf.size()));
  return retVal;
}

