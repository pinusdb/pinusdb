/*
* Copyright (c) 2020 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
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
#include "internal.h"
#include "util/string_tool.h"
#include "table/db_value.h"
#include "util/date_time.h"
#include "expr/expr_value.h"
#include "table/table_info.h"
#include "query/group_field.h"
#include "query/block_values.h"
#include <list>

class ValueItem
{
public:
  ValueItem() {}
  virtual ~ValueItem() {}

  //获取值
  virtual PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const = 0;
  virtual PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const = 0;
  //获取值类型
  virtual int32_t GetValueType() const = 0;
  //是否符合规则
  virtual bool IsValid() const = 0;
  //是否是常数值
  virtual bool IsConstValue() const = 0;
  virtual void GetUseFields(std::unordered_set<size_t>& fieldSet) const = 0;

  virtual bool IsDevIdCondition() const { return false; }
  virtual bool IsTstampCondition() const { return false; }
  virtual bool GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const { return false; }
  virtual bool GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const { return false; }
};

//常数
class ConstValue : public ValueItem
{
public:
  ConstValue(int valType)
  {
    this->valType_ = valType;
    DBVAL_SET_NULL(&val_);
  }

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
    valType_ = DBVAL_GET_TYPE(&val_);
  }

  int32_t GetValueType() const override
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
    size_t recordSize = blockValues.GetRecordSize();
    const uint8_t* pFilter = blockValues.GetFilter();
    if (pFilter == nullptr)
    {
      for (size_t idx = 0; idx < recordSize; idx++)
      {
        resultVec.push_back(val_);
      }
    }
    else
    {
      for (size_t idx = 0; idx < recordSize; idx++)
      {
        if (pFilter[idx] == PDB_BOOL_TRUE)
        {
          resultVec.push_back(val_);
        }
      }
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
  int32_t valType_;
  DBVal val_;
};


//字段值
template<int FieldType>
class FieldValue : public ValueItem
{
public:
  FieldValue(size_t fieldPos)
  {
    fieldPos_ = fieldPos;
  }

  virtual ~FieldValue()
  {}

  int32_t GetValueType() const override
  {
    return FieldType;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pVals == nullptr)
      return PdbE_INVALID_PARAM;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_ELE_GET_TYPE(pVals, fieldPos_) == FieldType)
    {
      *pResult = pVals[fieldPos_];
    }
    else
    {
      DBVAL_SET_NULL(pResult);
    }

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    size_t recordSize = blockValues.GetRecordSize();
    size_t resultSize = blockValues.GetResultSize();
    const uint8_t* pFilter = blockValues.GetFilter();
    const DBVal* pVals = blockValues.GetColumnValues(fieldPos_);

    if (resultSize == recordSize)
      pFilter = nullptr;

    DBVal nullVal;
    DBVAL_SET_NULL(&nullVal);

    if (pVals == nullptr)
    {
      for (size_t idx = 0; idx < resultSize; idx++)
      {
        resultVec.push_back(nullVal);
      }
    }
    else 
    {
      if (pFilter == nullptr)
      {
        for (size_t idx = 0; idx < recordSize; idx++)
        {
          if (DBVAL_ELE_GET_TYPE(pVals, idx) == FieldType)
            resultVec.push_back(pVals[idx]);
          else
            resultVec.push_back(nullVal);
        }
      }
      else
      {
        for (size_t idx = 0; idx < recordSize; idx++)
        {
          if (pFilter[idx] != PDB_BOOL_FALSE)
          {
            if (DBVAL_ELE_GET_TYPE(pVals, idx) == FieldType)
              resultVec.push_back(pVals[idx]);
            else
              resultVec.push_back(nullVal);
          }
        }
      }
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    return true;
  }

  bool IsConstValue() const override
  {
    return false;
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(fieldPos_);
  }

private:
  size_t fieldPos_;
};

ValueItem* CreateFieldValue(int fieldType, size_t fieldPos);

ValueItem* BuildGeneralValueItem(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMicroseconds);
PdbErr_t BuildTargetGroupItem(const TableInfo* pTableInfo, const ExprValue* pExpr,
  TableInfo* pGroupInfo, std::vector<GroupField*>& fieldVec, int64_t nowMicroseconds);

bool IncludeAggFunction(const ExprValue* pExpr);


