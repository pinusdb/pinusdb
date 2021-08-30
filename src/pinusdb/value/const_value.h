/*
* Copyright (c) 2021 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
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

#pragma once
#include "pdb.h"
#include "pdb_error.h"
#include "internal.h"
#include "expr/parse.h"
#include "value/value_item.h"
#include "table/db_value.h"

//³£Êý
class ConstValue : public ValueItem
{
public:
  //ConstValue(int valType)
  //{
  //  this->valType_ = valType;
  //  DBVAL_SET_NULL(&val_);
  //}

  ConstValue(bool value)
  {
    this->valType_ = PDB_VALUE_TYPE::VAL_BOOL;
    DBVAL_SET_BOOL(&val_, value);
  }

  ConstValue(int64_t value, bool isTime)
  {
    if (isTime)
    {
      this->valType_ = PDB_VALUE_TYPE::VAL_DATETIME;
      if (value >= DateTime::MinMicrosecond && value < DateTime::MaxMicrosecond)
      {
        DBVAL_SET_DATETIME(&val_, value);
      }
      else
      {
        DBVAL_SET_NULL(&val_);
      }
    }
    else
    {
      this->valType_ = PDB_VALUE_TYPE::VAL_INT64;
      DBVAL_SET_INT64(&val_, value);
    }
  }

  ConstValue(double value)
  {
    this->valType_ = PDB_VALUE_TYPE::VAL_DOUBLE;
    DBVAL_SET_DOUBLE(&val_, value);
  }

  ConstValue(const char* pStr, size_t len, bool isBlob)
  {
    if (isBlob)
    {
      this->valType_ = PDB_VALUE_TYPE::VAL_BLOB;
      DBVAL_SET_BLOB(&val_, pStr, len);
    }
    else
    {
      this->valType_ = PDB_VALUE_TYPE::VAL_STRING;
      DBVAL_SET_STRING(&val_, pStr, len);
    }
  }

  ConstValue(DBVal val)
  {
    val_ = val;
    valType_ = (PDB_VALUE_TYPE)DBVAL_GET_TYPE(&val_);
  }

  PDB_VALUE_TYPE GetValueType() const override
  {
    return valType_;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pResult != nullptr)
    {
      *pResult = val_;
    }

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    size_t resultSize = blockValues.GetResultSize();
    const uint8_t* pFilter = blockValues.GetFilter();
    for (size_t idx = 0; idx < resultSize; idx++)
    {
      resultVec.push_back(val_);
    }
    return PdbE_OK;
  }


  bool IsValid() const override
  {
    return true;
  }

  bool IsConstValue() const override
  {
    return true;
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    return;
  }

private:
  PDB_VALUE_TYPE valType_;
  DBVal val_;
};










