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

#include "query/value_item.h"
#include "expr/parse.h"
#include "query/group_function.h"
#include <math.h>

#define GET_NUMBER_BY_DATA_TYPE(DataType, PValue, result) do { \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_INT8) { result = DBVAL_GET_INT8(PValue); } \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_INT16) { result = DBVAL_GET_INT16(PValue); } \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_INT32) { result = DBVAL_GET_INT32(PValue); } \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_INT64) { result = DBVAL_GET_INT64(PValue); } \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_DATETIME) { result = DBVAL_GET_DATETIME(PValue); } \
} while (false)

#define GET_DOUBLE_BY_DATA_TYPE(DataType, PValue, result) do { \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_INT8) { result = static_cast<double>(DBVAL_GET_INT8(PValue)); } \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_INT16) { result = static_cast<double>(DBVAL_GET_INT16(PValue)); } \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_INT32) { result = static_cast<double>(DBVAL_GET_INT32(PValue)); } \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_INT64) { result = static_cast<double>(DBVAL_GET_INT64(PValue)); } \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_DATETIME) { result = static_cast<double>(DBVAL_GET_DATETIME(PValue)); } \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_FLOAT) { result = static_cast<double>(DBVAL_GET_FLOAT(PValue)); } \
  if constexpr (DataType == PDB_VALUE_TYPE::VAL_DOUBLE) { result = DBVAL_GET_DOUBLE(PValue); } \
} while (false)


template<int Op, int LeftType, int RightType>
class CalculateFunction : public ValueItem
{
public:
  CalculateFunction(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~CalculateFunction()
  {
    if (pLeft_ != nullptr)
      delete pLeft_;
    if (pRight_ != nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override
  {
    if constexpr (PDB_TYPE_IS_NUMBER(LeftType) && PDB_TYPE_IS_NUMBER(RightType))
      return PDB_VALUE_TYPE::VAL_INT64;

    return PDB_VALUE_TYPE::VAL_DOUBLE;
  }

  PdbErr_t GetValue(const DBVal* pRecord, DBVal* pResult) const override
  {
    PdbErr_t retVal = PdbE_OK;
    DBVal leftValue, rightValue;
    retVal = pLeft_->GetValue(pRecord, &leftValue);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pRight_->GetValue(pRecord, &rightValue);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_GET_TYPE(&leftValue) != LeftType
      || DBVAL_GET_TYPE(&rightValue) != RightType)
    {
      DBVAL_SET_NULL(pResult);
      return PdbE_OK;
    }

    if constexpr (PDB_TYPE_IS_NUMBER(LeftType) && PDB_TYPE_IS_NUMBER(RightType))
    {
      int64_t lv, rv;
      GET_NUMBER_BY_DATA_TYPE(LeftType, &leftValue, lv);
      GET_NUMBER_BY_DATA_TYPE(RightType, &rightValue, rv);

      if constexpr (Op == PDB_SQL_FUNC::FUNC_ADD)
        DBVAL_SET_INT64(pResult, (lv + rv));
      else if constexpr (Op == PDB_SQL_FUNC::FUNC_SUB)
        DBVAL_SET_INT64(pResult, (lv - rv));
      else if constexpr (Op == PDB_SQL_FUNC::FUNC_MUL)
        DBVAL_SET_INT64(pResult, (lv * rv));
      else if constexpr (Op == PDB_SQL_FUNC::FUNC_DIV)
      {
        if (rv == 0)
          DBVAL_SET_NULL(pResult);
        else
          DBVAL_SET_INT64(pResult, (lv / rv));
      }
      else if constexpr (Op == PDB_SQL_FUNC::FUNC_MOD)
      {
        if (rv == 0)
          DBVAL_SET_NULL(pResult);
        else
          DBVAL_SET_INT64(pResult, (lv % rv));
      }
    }
    else
    {
      double lv, rv;
      GET_DOUBLE_BY_DATA_TYPE(LeftType, &leftValue, lv);
      GET_DOUBLE_BY_DATA_TYPE(RightType, &rightValue, rv);
      
      if constexpr (Op == PDB_SQL_FUNC::FUNC_ADD)
        DBVAL_SET_DOUBLE(pResult, (lv + rv));
      else if constexpr (Op == PDB_SQL_FUNC::FUNC_SUB)
        DBVAL_SET_DOUBLE(pResult, (lv - rv));
      else if constexpr (Op == PDB_SQL_FUNC::FUNC_MUL)
        DBVAL_SET_DOUBLE(pResult, (lv * rv));
      else if constexpr (Op == PDB_SQL_FUNC::FUNC_DIV)
      {
        if (DOUBLE_EQUAL_ZERO(rv))
          DBVAL_SET_NULL(pResult);
        else
          DBVAL_SET_DOUBLE(pResult, (lv / rv));
      }
    }

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> leftValVec;
    std::vector<DBVal> rightValVec;
    leftValVec.reserve(resultSize);
    rightValVec.reserve(resultSize);
    retVal = pLeft_->GetValueArray(blockValues, leftValVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (leftValVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    retVal = pRight_->GetValueArray(blockValues, rightValVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (rightValVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    DBVal tmpVal;
    DBVal nullVal;
    DBVAL_SET_NULL(&nullVal);
    const DBVal* pLeftVals = leftValVec.data();
    const DBVal* pRightVals = rightValVec.data();

    for (size_t idx = 0; idx < resultSize; idx++)
    {
      const DBVal* pLeftVal = pLeftVals + idx;
      const DBVal* pRightVal = pRightVals + idx;

      if (DBVAL_IS_NULL(pLeftVal) || DBVAL_IS_NULL(pRightVal))
      {
        resultVec.push_back(nullVal);
        continue;
      }

      if constexpr (PDB_TYPE_IS_NUMBER(LeftType) && PDB_TYPE_IS_NUMBER(RightType))
      {
        int64_t lv, rv;
        GET_NUMBER_BY_DATA_TYPE(LeftType, pLeftVal, lv);
        GET_NUMBER_BY_DATA_TYPE(RightType, pRightVal, rv);

        if constexpr (Op == PDB_SQL_FUNC::FUNC_ADD)
          DBVAL_SET_INT64(&tmpVal, (lv + rv));
        else if constexpr (Op == PDB_SQL_FUNC::FUNC_SUB)
          DBVAL_SET_INT64(&tmpVal, (lv - rv));
        else if constexpr (Op == PDB_SQL_FUNC::FUNC_MUL)
          DBVAL_SET_INT64(&tmpVal, (lv * rv));
        else if constexpr (Op == PDB_SQL_FUNC::FUNC_DIV)
        {
          if (rv == 0)
            DBVAL_SET_NULL(&tmpVal);
          else
            DBVAL_SET_INT64(&tmpVal, (lv / rv));
        }
        else if constexpr (Op == PDB_SQL_FUNC::FUNC_MOD)
        {
          if (rv == 0)
            DBVAL_SET_NULL(&tmpVal);
          else
            DBVAL_SET_INT64(&tmpVal, (lv % rv));
        }
      }
      else
      {
        double lv, rv;
        GET_DOUBLE_BY_DATA_TYPE(LeftType, pLeftVal, lv);
        GET_DOUBLE_BY_DATA_TYPE(RightType, pRightVal, rv);

        if constexpr (Op == PDB_SQL_FUNC::FUNC_ADD)
          DBVAL_SET_DOUBLE(&tmpVal, (lv + rv));
        else if constexpr (Op == PDB_SQL_FUNC::FUNC_SUB)
          DBVAL_SET_DOUBLE(&tmpVal, (lv + rv));
        else if constexpr (Op == PDB_SQL_FUNC::FUNC_MUL)
          DBVAL_SET_DOUBLE(&tmpVal, (lv * rv));
        else if constexpr (Op == PDB_SQL_FUNC::FUNC_DIV)
        {
          if (DOUBLE_EQUAL_ZERO(rv))
            DBVAL_SET_NULL(&tmpVal);
          else
            DBVAL_SET_DOUBLE(&tmpVal, (lv / rv));
        }
      }

      resultVec.push_back(tmpVal);
    }

    return PdbE_OK;
  }
  
  bool IsValid() const override
  {
    if (pLeft_ == nullptr || pRight_ == nullptr)
      return false;

    if (!pLeft_->IsValid())
      return false;

    if (!pRight_->IsValid())
      return false;

    if (LeftType != pLeft_->GetValueType())
      return false;

    if (RightType != pRight_->GetValueType())
      return false;

    if (Op == PDB_SQL_FUNC::FUNC_MOD)
    {
      return  PDB_TYPE_IS_NUMBER(LeftType) && PDB_TYPE_IS_NUMBER(RightType);
    }

    if (!PDB_TYPE_IS_NUMBER(LeftType) && !PDB_TYPE_IS_FLOAT_OR_DOUBLE(LeftType))
      return false;

    if (!PDB_TYPE_IS_NUMBER(RightType) && !PDB_TYPE_IS_FLOAT_OR_DOUBLE(RightType))
      return false;

    return true;
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pLeft_ != nullptr)
      pLeft_->GetUseFields(fieldSet);

    if (pRight_ != nullptr)
      pRight_->GetUseFields(fieldSet);
  }

protected:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

ValueItem* ConvertStringToDateTime(ValueItem* pValue)
{
  if (!(pValue->IsConstValue()))
    return nullptr;

  DBVal strVal;
  if (pValue->GetValue(nullptr, &strVal) != PdbE_OK)
    return nullptr;

  if (!DBVAL_IS_STRING(&strVal))
    return nullptr;

  DateTime dt;
  if (dt.Parse(DBVAL_GET_STRING(&strVal), DBVAL_GET_LEN(&strVal)))
  {
    return new ConstValue(dt.TotalMicrosecond(), true);
  }

  return nullptr;
}

//时间 加
class DateTimeAdd : public ValueItem
{
public:
  DateTimeAdd(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~DateTimeAdd()
  {
    if (pLeft_ == nullptr)
      delete pLeft_;
    if (pRight_ == nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override { return PDB_VALUE_TYPE::VAL_DATETIME; }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal leftValue, rightValue;
    retVal = pLeft_->GetValue(pVals, &leftValue);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pRight_->GetValue(pVals, &rightValue);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_GET_TYPE(&leftValue) != PDB_VALUE_TYPE::VAL_DATETIME
      || DBVAL_GET_TYPE(&rightValue) != PDB_VALUE_TYPE::VAL_INT64)
    {
      DBVAL_SET_NULL(pResult);
      return PdbE_OK;
    }

    int64_t dtVal = DBVAL_GET_DATETIME(&leftValue) + DBVAL_GET_INT64(&rightValue);
    if (dtVal >= DateTime::MinMicrosecond && dtVal <= DateTime::MaxMicrosecond)
    {
      DBVAL_SET_DATETIME(pResult, dtVal);
    }
    else
    {
      DBVAL_SET_NULL(pResult);
    }

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> leftValVec;
    std::vector<DBVal> rightValVec;
    leftValVec.reserve(resultSize);
    rightValVec.reserve(resultSize);
    retVal = pLeft_->GetValueArray(blockValues, leftValVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (leftValVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    retVal = pRight_->GetValueArray(blockValues, rightValVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (rightValVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    int64_t dtVal;
    DBVal tmpVal;
    DBVal nullVal;
    DBVAL_SET_NULL(&nullVal);
    const DBVal* pLeftVals = leftValVec.data();
    const DBVal* pRightVals = rightValVec.data();

    for (size_t idx = 0; idx < resultSize; idx++)
    {
      const DBVal* pLeftVal = pLeftVals + idx;
      const DBVal* pRightVal = pRightVals + idx;

      if (DBVAL_GET_TYPE(pLeftVal) != PDB_VALUE_TYPE::VAL_DATETIME
        || DBVAL_GET_TYPE(pRightVal) != PDB_VALUE_TYPE::VAL_INT64)
      {
        resultVec.push_back(nullVal);
      }
      else
      {
        dtVal = DBVAL_GET_DATETIME(pLeftVal) + DBVAL_GET_INT64(pRightVal);
        if (dtVal >= DateTime::MinMicrosecond && dtVal <= DateTime::MaxMicrosecond)
        {
          DBVAL_SET_DATETIME(&tmpVal, dtVal);
          resultVec.push_back(tmpVal);
        }
        else
        {
          resultVec.push_back(nullVal);
        }
      }
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pLeft_ == nullptr || pRight_ == nullptr)
      return false;

    if (!pLeft_->IsValid())
      return false;

    if (!pRight_->IsValid())
      return false;

    return (pLeft_->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME
      && pRight_->GetValueType() == PDB_VALUE_TYPE::VAL_INT64);
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pLeft_ != nullptr)
      pLeft_->GetUseFields(fieldSet);

    if (pRight_ != nullptr)
      pRight_->GetUseFields(fieldSet);
  }

protected:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

//时间 差
class DateTimeDiff : public ValueItem
{
public:
  DateTimeDiff(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~DateTimeDiff()
  {
    if (pLeft_ == nullptr)
      delete pLeft_;
    if (pRight_ == nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override { return PDB_VALUE_TYPE::VAL_INT64; }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal leftValue, rightValue;
    retVal = pLeft_->GetValue(pVals, &leftValue);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pRight_->GetValue(pVals, &rightValue);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_GET_TYPE(&leftValue) != PDB_VALUE_TYPE::VAL_DATETIME
      || DBVAL_GET_TYPE(&rightValue) != PDB_VALUE_TYPE::VAL_DATETIME)
    {
      DBVAL_SET_NULL(pResult);
      return PdbE_OK;
    }

    DBVAL_SET_INT64(pResult, (DBVAL_GET_DATETIME(&leftValue) - DBVAL_GET_DATETIME(&rightValue)));
    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> leftValVec;
    std::vector<DBVal> rightValVec;
    leftValVec.reserve(resultSize);
    rightValVec.reserve(resultSize);
    retVal = pLeft_->GetValueArray(blockValues, leftValVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (leftValVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    retVal = pRight_->GetValueArray(blockValues, rightValVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (rightValVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    DBVal tmpVal;
    DBVal nullVal;
    DBVAL_SET_NULL(&nullVal);
    const DBVal* pLeftVals = leftValVec.data();
    const DBVal* pRightVals = rightValVec.data();

    for (size_t idx = 0; idx < resultSize; idx++)
    {
      const DBVal* pLeftVal = pLeftVals + idx;
      const DBVal* pRightVal = pRightVals + idx;

      if (DBVAL_IS_DATETIME(pLeftVal) && DBVAL_IS_DATETIME(pRightVal))
      {
        DBVAL_SET_INT64(&tmpVal, (DBVAL_GET_DATETIME(pLeftVal) - DBVAL_GET_DATETIME(pRightVal)));
        resultVec.push_back(tmpVal);
      }
      else
      {
        resultVec.push_back(nullVal);
      }
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pLeft_ == nullptr || pRight_ == nullptr)
      return false;

    if (!pLeft_->IsValid())
      return false;

    if (!pRight_->IsValid())
      return false;

    return (pLeft_->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME
      && pRight_->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME);
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pLeft_ != nullptr)
      pLeft_->GetUseFields(fieldSet);

    if (pRight_ != nullptr)
      pRight_->GetUseFields(fieldSet);
  }

protected:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

template<bool IsCeil>
class DateTimeAlign : public ValueItem
{
public:
  DateTimeAlign(ValueItem* pValue, int64_t microseconds)
  {
    pTimeValue_ = pValue;
    microseconds_ = microseconds;
    timeZone_ = 0;
    if (microseconds_ == DateTime::MicrosecondPerDay)
    {
      timeZone_ = DateTime::GetSysTimeZone();
    }
  }

  virtual ~DateTimeAlign()
  {
    if (pTimeValue_ != nullptr)
      delete pTimeValue_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_DATETIME;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal = PdbE_OK;
    DBVal tmpVal;
    retVal = pTimeValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (!DBVAL_IS_DATETIME(&tmpVal))
    {
      DBVAL_SET_NULL(pResult);
      return PdbE_OK;
    }

    int64_t dtVal = DBVAL_GET_DATETIME(&tmpVal) - timeZone_;
    if constexpr (IsCeil)
    {
      dtVal -= (dtVal % microseconds_);
    }
    else
    {
      dtVal += (microseconds_ - 1);
      dtVal -= (dtVal % microseconds_);
    }
    dtVal += timeZone_;
    DBVAL_SET_DATETIME(pResult, dtVal);
    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> valVec;
    valVec.reserve(resultSize);
    retVal = pTimeValue_->GetValueArray(blockValues, valVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (valVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    int64_t dtVal;
    DBVal tmpVal;
    DBVal nullVal;
    DBVAL_SET_NULL(&nullVal);
    const DBVal* pVals = valVec.data();

    for (size_t idx = 0; idx < resultSize; idx++)
    {
      const DBVal* pVal = pVals + idx;
      if (DBVAL_IS_DATETIME(pVal))
      {
        dtVal = DBVAL_GET_DATETIME(pVal) - timeZone_;
        if constexpr (IsCeil)
        {
          dtVal -= (dtVal % microseconds_);
        }
        else
        {
          dtVal += (microseconds_ - 1);
          dtVal -= (dtVal % microseconds_);
        }
        dtVal += timeZone_;
        DBVAL_SET_DATETIME(&tmpVal, dtVal);
        resultVec.push_back(tmpVal);
      }
      else
      {
        resultVec.push_back(nullVal);
      }
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pTimeValue_ == nullptr)
      return false;

    if (!pTimeValue_->IsValid())
      return false;

    if (microseconds_ <= 0)
      return false;

    return pTimeValue_->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME;
  }

  bool IsConstValue() const override
  {
    return pTimeValue_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pTimeValue_ != nullptr)
      pTimeValue_->GetUseFields(fieldSet);
  }

private:
  ValueItem* pTimeValue_;
  int64_t microseconds_;
  int64_t timeZone_;
};

//绝对值函数
template<int ValueType>
class AbsFunction : public ValueItem
{
public:
  AbsFunction(ValueItem* pValue)
  {
    this->pValue_ = pValue;
  }

  virtual ~AbsFunction()
  {
    if (pValue_ != nullptr)
      delete pValue_;
  }

  int32_t GetValueType() const override
  {
    return ValueType;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_IS_NULL(&tmpVal))
    {
      DBVAL_SET_NULL(pResult);
      return PdbE_OK;
    }

#ifdef _WIN32
#define ABS_FUNC(val)  std::abs(val)
#else
#define ABS_FUNC(val)  abs(val)
#endif

    if constexpr (ValueType == PDB_VALUE_TYPE::VAL_INT8)
    {
      DBVAL_SET_INT8(pResult, ABS_FUNC(DBVAL_GET_INT8(&tmpVal)));
    }
    else if constexpr (ValueType == PDB_VALUE_TYPE::VAL_INT16)
    {
      DBVAL_SET_INT16(pResult, ABS_FUNC(DBVAL_GET_INT16(&tmpVal)));
    }
    else if constexpr (ValueType == PDB_VALUE_TYPE::VAL_INT32)
    {
      DBVAL_SET_INT32(pResult, ABS_FUNC(DBVAL_GET_INT32(&tmpVal)));
    }
    else if constexpr (ValueType == PDB_VALUE_TYPE::VAL_INT64)
    {
      DBVAL_SET_INT64(pResult, ABS_FUNC(DBVAL_GET_INT64(&tmpVal)));
    }
    else if constexpr (ValueType == PDB_VALUE_TYPE::VAL_FLOAT)
    {
      DBVAL_SET_FLOAT(pResult, ABS_FUNC(DBVAL_GET_FLOAT(&tmpVal)));
    }
    else if constexpr (ValueType == PDB_VALUE_TYPE::VAL_DOUBLE)
    {
      DBVAL_SET_DOUBLE(pResult, ABS_FUNC(DBVAL_GET_DOUBLE(&tmpVal)));
    }

#undef ABS_FUNC

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> valVec;
    valVec.reserve(resultSize);
    retVal = pValue_->GetValueArray(blockValues, valVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (valVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    DBVal tmpVal;
    DBVal nullVal;
    DBVAL_SET_NULL(&nullVal);
    const DBVal* pVals = valVec.data();

#ifdef _WIN32
#define ABS_FUNC(val)  std::abs(val)
#else
#define ABS_FUNC(val)  abs(val)
#endif

    for (size_t idx = 0; idx < resultSize; idx++)
    {
      if (DBVAL_ELE_IS_NULL(pVals, idx))
      {
        resultVec.push_back(nullVal);
      }
      else
      {
        if constexpr (ValueType == PDB_VALUE_TYPE::VAL_INT8)
        {
          DBVAL_SET_INT8(&tmpVal, ABS_FUNC(DBVAL_ELE_GET_INT8(pVals, idx)));
        }
        else if constexpr (ValueType == PDB_VALUE_TYPE::VAL_INT16)
        {
          DBVAL_SET_INT16(&tmpVal, ABS_FUNC(DBVAL_ELE_GET_INT16(pVals, idx)));
        }
        else if constexpr (ValueType == PDB_VALUE_TYPE::VAL_INT32)
        {
          DBVAL_SET_INT32(&tmpVal, ABS_FUNC(DBVAL_ELE_GET_INT32(pVals, idx)));
        }
        else if constexpr (ValueType == PDB_VALUE_TYPE::VAL_INT64)
        {
          DBVAL_SET_INT64(&tmpVal, ABS_FUNC(DBVAL_ELE_GET_INT64(pVals, idx)));
        }
        else if constexpr (ValueType == PDB_VALUE_TYPE::VAL_FLOAT)
        {
          DBVAL_SET_FLOAT(&tmpVal, ABS_FUNC(DBVAL_ELE_GET_FLOAT(pVals, idx)));
        }
        else if constexpr (ValueType == PDB_VALUE_TYPE::VAL_DOUBLE)
        {
          DBVAL_SET_DOUBLE(&tmpVal, ABS_FUNC(DBVAL_ELE_GET_DOUBLE(pVals, idx)));
        }
      
        resultVec.push_back(tmpVal);
      }
    }

#undef ABS_FUNC

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pValue_ == nullptr)
      return false;

    int32_t resultType = pValue_->GetValueType();
    if (resultType != ValueType)
      return false;

    if (PDB_TYPE_IS_NUMBER(ValueType))
      return true;

    if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(ValueType))
      return true;

    return false;
  }

  bool IsConstValue() const override
  {
    return pValue_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pValue_ != nullptr)
      pValue_->GetUseFields(fieldSet);
  }

private:
  ValueItem* pValue_;
};


template<int ResultType,int ParamType0, int ParamType1>
class IfFunction : public ValueItem
{
public:
  IfFunction(ValueItem* pCondition, ValueItem* pResult0, ValueItem* pResult1)
  {
    this->pCondition_ = pCondition;
    this->pResult0_ = pResult0;
    this->pResult1_ = pResult1;
  }

  virtual ~IfFunction()
  {
    if (pCondition_ != nullptr)
      delete pCondition_;

    if (pResult0_ != nullptr)
      delete pResult0_;

    if (pResult1_ != nullptr)
      delete pResult1_;
  }

  int32_t GetValueType() const override
  {
    return ResultType;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal condiVal, rst0Val, rst1Val;
    retVal = pCondition_->GetValue(pVals, &condiVal);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pResult0_->GetValue(pVals, &rst0Val);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pResult1_->GetValue(pVals, &rst1Val);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_IS_BOOL(&condiVal) && DBVAL_GET_BOOL(&condiVal))
    {
      if (DBVAL_IS_NULL(&rst0Val))
      {
        DBVAL_SET_NULL(pResult);
        return PdbE_OK;
      }

      if constexpr (ResultType == ParamType0)
      {
        *pResult = rst0Val;
      }
      else if constexpr (ResultType == PDB_VALUE_TYPE::VAL_INT64)
      {
        int64_t i64 = 0;
        GET_NUMBER_BY_DATA_TYPE(ParamType0, &rst0Val, i64);
        DBVAL_SET_INT64(pResult, i64);
      }
      else if constexpr (ResultType == PDB_VALUE_TYPE::VAL_DOUBLE)
      {
        double r64 = 0;
        GET_DOUBLE_BY_DATA_TYPE(ParamType0, &rst0Val, r64);
        DBVAL_SET_DOUBLE(pResult, r64);
      }
    }
    else
    {
      if (DBVAL_IS_NULL(&rst1Val))
      {
        DBVAL_SET_NULL(pResult);
        return PdbE_OK;
      }

      if constexpr (ResultType == ParamType1)
      {
        *pResult = rst1Val;
      }
      else if constexpr (ResultType == PDB_VALUE_TYPE::VAL_INT64)
      {
        int64_t i64 = 0;
        GET_NUMBER_BY_DATA_TYPE(ParamType1, &rst1Val, i64);
        DBVAL_SET_INT64(pResult, i64);
      }
      else if constexpr (ResultType == PDB_VALUE_TYPE::VAL_DOUBLE)
      {
        double r64 = 0;
        GET_DOUBLE_BY_DATA_TYPE(ParamType1, &rst1Val, r64);
        DBVAL_SET_DOUBLE(pResult, r64);
      }
    }

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> condiVec;
    std::vector<DBVal> param0ValVec;
    std::vector<DBVal> param1ValVec;
    condiVec.reserve(resultSize);
    param0ValVec.reserve(resultSize);
    param1ValVec.reserve(resultSize);

    retVal = pCondition_->GetValueArray(blockValues, condiVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (condiVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    retVal = pResult0_->GetValueArray(blockValues, param0ValVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (param0ValVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    retVal = pResult1_->GetValueArray(blockValues, param1ValVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (param1ValVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    const DBVal* pCondiVals = condiVec.data();
    const DBVal* pLeftVals = param0ValVec.data();
    const DBVal* pRightVals = param1ValVec.data();

    DBVal tmpVal;
    DBVal nullVal;
    DBVAL_SET_NULL(&nullVal);

    for (size_t idx = 0; idx < resultSize; idx++)
    {
      const DBVal* pCondiVal = pCondiVals + idx;
      const DBVal* pParam0Val = pLeftVals + idx;
      const DBVal* pParam1Val = pRightVals + idx;

      if (DBVAL_IS_BOOL(pCondiVal) && DBVAL_GET_BOOL(pCondiVal))
      {
        if (DBVAL_IS_NULL(pParam0Val))
        {
          resultVec.push_back(nullVal);
        }
        else
        {
          if constexpr (ResultType == ParamType0)
          {
            resultVec.push_back(*pParam0Val);
          }
          else if constexpr (ResultType == PDB_VALUE_TYPE::VAL_INT64)
          {
            int64_t i64 = 0;
            GET_NUMBER_BY_DATA_TYPE(ParamType0, pParam0Val, i64);
            DBVAL_SET_INT64(&tmpVal, i64);
            resultVec.push_back(tmpVal);
          }
          else if constexpr (ResultType == PDB_VALUE_TYPE::VAL_DOUBLE)
          {
            double r64 = 0;
            GET_DOUBLE_BY_DATA_TYPE(ParamType0, pParam0Val, r64);
            DBVAL_SET_DOUBLE(&tmpVal, r64);
            resultVec.push_back(tmpVal);
          }
        }
      }
      else
      {
        if (DBVAL_IS_NULL(pParam1Val))
        {
          resultVec.push_back(nullVal);
        }
        else
        {
          if constexpr (ResultType == ParamType1)
          {
            resultVec.push_back(*pParam1Val);
          }
          else if constexpr (ResultType == PDB_VALUE_TYPE::VAL_INT64)
          {
            int64_t i64 = 0;
            GET_NUMBER_BY_DATA_TYPE(ParamType1, pParam1Val, i64);
            DBVAL_SET_INT64(&tmpVal, i64);
            resultVec.push_back(tmpVal);
          }
          else if constexpr (ResultType == PDB_VALUE_TYPE::VAL_DOUBLE)
          {
            double r64 = 0;
            GET_DOUBLE_BY_DATA_TYPE(ParamType1, pParam1Val, r64);
            DBVAL_SET_DOUBLE(&tmpVal, r64);
            resultVec.push_back(tmpVal);
          }
        }
      }
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pCondition_ == nullptr || pResult0_ == nullptr || pResult1_ == nullptr)
      return false;

    if (!pCondition_->IsValid())
      return false;

    if (pCondition_->GetValueType() != PDB_VALUE_TYPE::VAL_BOOL)
      return false;

    if (!pResult0_->IsValid())
      return false;

    if (pResult0_->GetValueType() != ParamType0)
      return false;

    if (!pResult1_->IsValid())
      return false;

    if (pResult1_->GetValueType() != ParamType1)
      return false;

    //两个参数的类型相同，返回类型等于参数类型
    //两个参数都是数字类型，返回类型为 INT64
    //一个参数是数字型，一个是浮点型，返回类型为 DOUBLE
    //两个参数都是浮点型，返回类型为 DOUBLE
    if (ParamType0 == ParamType1)
    {
      return ResultType == ParamType0;
    }
    else if (PDB_TYPE_IS_NUMBER(ParamType0))
    {
      if (PDB_TYPE_IS_NUMBER(ParamType1))
        return ResultType == PDB_VALUE_TYPE::VAL_INT64;
      else if (PDB_TYPE_IS_REAL(ParamType1))
        return ResultType == PDB_VALUE_TYPE::VAL_DOUBLE;
    }
    else if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(ParamType0))
    {
      if (PDB_TYPE_IS_NUMBER(ParamType1) || PDB_TYPE_IS_FLOAT_OR_DOUBLE(ParamType1))
        return ResultType == PDB_VALUE_TYPE::VAL_DOUBLE;
    }

    return false;
  }

  bool IsConstValue() const override
  {
    return pCondition_->IsConstValue()
      && pResult0_->IsConstValue() && pResult1_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pCondition_ != nullptr)
      pCondition_->GetUseFields(fieldSet);

    if (pResult0_ != nullptr)
      pResult0_->GetUseFields(fieldSet);

    if (pResult1_ != nullptr)
      pResult1_->GetUseFields(fieldSet);
  }

private:
  ValueItem* pCondition_;
  ValueItem* pResult0_;
  ValueItem* pResult1_;
};

template<bool NotNull>
class NullFunction : public ValueItem
{
public:
  NullFunction(ValueItem* pValue)
  {
    pValue_ = pValue;
  }

  virtual ~NullFunction()
  {
    if (pValue_ != nullptr)
      delete pValue_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if constexpr (NotNull)
    {
      DBVAL_SET_BOOL(pResult, !DBVAL_IS_NULL(&tmpVal));
    }
    else
    {
      DBVAL_SET_BOOL(pResult, DBVAL_IS_NULL(&tmpVal));
    }
    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> valVec;
    valVec.reserve(resultSize);

    retVal = pValue_->GetValueArray(blockValues, valVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (valVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    const DBVal* pVals = valVec.data();

    DBVal tmpVal;
    for (size_t idx = 0; idx < resultSize; idx++)
    {
      if constexpr (NotNull)
      {
        DBVAL_SET_BOOL(&tmpVal, !DBVAL_ELE_IS_NULL(pVals, idx));
      }
      else
      {
        DBVAL_SET_BOOL(&tmpVal, DBVAL_ELE_IS_NULL(pVals, idx));
      }
      resultVec.push_back(tmpVal);
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pValue_ == nullptr)
      return false;

    return pValue_->IsValid();
  }

  bool IsConstValue() const override
  {
    return pValue_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pValue_ != nullptr)
      pValue_->GetUseFields(fieldSet);
  }

protected:
  ValueItem* pValue_;
};

class LikeFunction : public ValueItem
{
public:
  LikeFunction(ValueItem* pValue, const char* pPattern, size_t patternLen)
  {
    this->pValue_ = pValue;
    this->patternStr_ = std::string(pPattern, patternLen);
  }

  virtual ~LikeFunction()
  {
    if (pValue_ != nullptr)
      delete pValue_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (!DBVAL_IS_STRING(&tmpVal))
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    const char* pStrValue = DBVAL_GET_STRING(&tmpVal);
    size_t strLen = DBVAL_GET_LEN(&tmpVal);

    DBVAL_SET_BOOL(pResult, StringTool::Utf8LikeCompare(patternStr_.c_str(), pStrValue, strLen));
    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> valVec;
    valVec.reserve(resultSize);

    retVal = pValue_->GetValueArray(blockValues, valVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (valVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    const DBVal* pVals = valVec.data();

    DBVal tmpVal;
    for (size_t idx = 0; idx < resultSize; idx++)
    {
      if (DBVAL_ELE_IS_STRING(pVals, idx))
      {
        const char* pStr = DBVAL_ELE_GET_STRING(pVals, idx);
        size_t strLen = DBVAL_ELE_GET_LEN(pVals, idx);

        DBVAL_SET_BOOL(&tmpVal, StringTool::Utf8LikeCompare(patternStr_.c_str(), pStr, strLen));
      }
      else
      {
        DBVAL_SET_BOOL(&tmpVal, false);
      }
      resultVec.push_back(tmpVal);
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pValue_ == nullptr)
      return false;

    if (!pValue_->IsValid())
      return false;

    return pValue_->GetValueType() == PDB_VALUE_TYPE::VAL_STRING;
  }

  bool IsConstValue() const override
  {
    return pValue_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pValue_ != nullptr)
      pValue_->GetUseFields(fieldSet);
  }

private:
  ValueItem* pValue_;
  std::string patternStr_;
};

template<bool NotIn, int ValueType>
class InFunction : public ValueItem
{
public:
  InFunction(int32_t fieldPos, const std::list<int64_t>& valList)
  {
    fieldPos_ = fieldPos;
    for (auto valIt = valList.begin(); valIt != valList.end(); valIt++)
    {
      valSet_.insert(*valIt);
    }
  }

  virtual ~InFunction()
  {
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pResult == nullptr)
      return PdbE_OK;

    const DBVal* pFieldVal = pVals + fieldPos_;

    int64_t findVal = 0;
    if (DBVAL_GET_TYPE(pFieldVal) != ValueType)
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if constexpr (PDB_TYPE_IS_NUMBER(ValueType))
    {
      GET_NUMBER_BY_DATA_TYPE(ValueType, pFieldVal, findVal);
    }
    else if constexpr (ValueType == PDB_FIELD_TYPE::TYPE_STRING)
    {
      findVal = static_cast<int64_t>(StringTool::CRC64(DBVAL_GET_STRING(pFieldVal), DBVAL_GET_LEN(pFieldVal)));
    }

    if (valSet_.find(findVal) != valSet_.end())
    {
      DBVAL_SET_BOOL(pResult, !NotIn);
    }
    else
    {
      DBVAL_SET_BOOL(pResult, NotIn);
    }

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    size_t recordSize = blockValues.GetRecordSize();
    const DBVal* pFieldVals = blockValues.GetColumnValues(fieldPos_);
    const uint8_t* pFilter = blockValues.GetFilter();

    DBVal tmpVal;
    int64_t findVal = 0;
    for (size_t idx = 0; idx < recordSize; idx++)
    {
      const DBVal* pVal = pFieldVals + idx;

      if (pFilter != nullptr)
      {
        if (pFilter[idx] == 0)
          continue;
      }

      if (DBVAL_GET_TYPE(pVal) != ValueType)
      {
        DBVAL_SET_BOOL(&tmpVal, false);
      }
      else
      {
        if constexpr (PDB_TYPE_IS_NUMBER(ValueType))
        {
          GET_NUMBER_BY_DATA_TYPE(ValueType, pVal, findVal);
        }
        else if constexpr (ValueType == PDB_FIELD_TYPE::TYPE_STRING)
        {
          findVal = static_cast<int64_t>(StringTool::CRC64(DBVAL_GET_STRING(&tmpVal), DBVAL_GET_LEN(&tmpVal)));
        }

        if (valSet_.find(findVal) != valSet_.end())
        {
          DBVAL_SET_BOOL(&tmpVal, !NotIn);
        }
        else
        {
          DBVAL_SET_BOOL(&tmpVal, NotIn);
        }
      }
      resultVec.push_back(tmpVal);
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (valSet_.empty())
      return false;

    return PDB_TYPE_IS_NUMBER(ValueType) || ValueType == PDB_FIELD_TYPE::TYPE_STRING;
  }

  bool IsConstValue() const override
  {
    return false;
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(fieldPos_);
  }

  bool IsDevIdCondition() const override
  { 
    return fieldPos_ == PDB_DEVID_INDEX && ValueType == PDB_FIELD_TYPE::TYPE_INT64;
  }

  bool GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const override
  {
    int64_t minId = 0;
    int64_t maxId = INT64_MAX;
    if (!NotIn)
    {
      minId = -1;
      maxId = -1;
      for (auto valIt = valSet_.begin(); valIt != valSet_.end(); valIt++)
      {
        if (*valIt > 0)
        {
          if (minId < 0)
          {
            minId = *valIt;
            maxId = *valIt;
          }
          else
          {
            if (*valIt > maxId)
              maxId = *valIt;

            if (*valIt < minId)
              minId = *valIt;
          }
        }
      }

    }

    if (pMinDevId != nullptr)
      *pMinDevId = minId;

    if (pMaxDevId != nullptr)
      *pMaxDevId = maxId;
    return true;
  }

protected:
  int32_t fieldPos_;
  std::unordered_set<int64_t> valSet_;
};

template<int LeftType, int RightType, int CompOp>
class ValueCompareFunction : public ValueItem
{
public:
  ValueCompareFunction(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~ValueCompareFunction()
  {
    if (pLeft_ == nullptr)
      delete pLeft_;
    if (pRight_ == nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal leftValue, rightValue;
    bool result = false;

    retVal = pLeft_->GetValue(pVals, &leftValue);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pRight_->GetValue(pVals, &rightValue);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_GET_TYPE(&leftValue) != LeftType || DBVAL_GET_TYPE(&rightValue) != RightType)
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if constexpr (LeftType == PDB_FIELD_TYPE::TYPE_STRING && RightType == PDB_FIELD_TYPE::TYPE_STRING)
    {
      if constexpr (CompOp == TK_EQ)
      {
        if (DBVAL_GET_LEN(&leftValue) == DBVAL_GET_LEN(&rightValue))
          result = strncmp(DBVAL_GET_STRING(&leftValue), DBVAL_GET_STRING(&rightValue), DBVAL_GET_LEN(&leftValue)) == 0;
      }
      else if constexpr (CompOp == TK_NE)
      {
        if (DBVAL_GET_LEN(&leftValue) != DBVAL_GET_LEN(&rightValue))
          result = true;
        else
          result = strncmp(DBVAL_GET_STRING(&leftValue), DBVAL_GET_STRING(&rightValue), DBVAL_GET_LEN(&leftValue)) != 0;
      }
    }
    else if constexpr (LeftType == PDB_FIELD_TYPE::TYPE_BOOL && RightType == PDB_FIELD_TYPE::TYPE_BOOL)
    {
      if constexpr (CompOp == TK_EQ)
        result = DBVAL_GET_BOOL(&leftValue) == DBVAL_GET_BOOL(&rightValue);
      else if constexpr (CompOp == TK_NE)
        result = DBVAL_GET_BOOL(&leftValue) != DBVAL_GET_BOOL(&rightValue);
    }
    else if constexpr ((PDB_TYPE_IS_NUMBER(LeftType) && PDB_TYPE_IS_NUMBER(RightType)) 
      || (LeftType == PDB_FIELD_TYPE::TYPE_DATETIME && RightType == PDB_FIELD_TYPE::TYPE_DATETIME))
    {
      int64_t lv, rv;
      GET_NUMBER_BY_DATA_TYPE(LeftType, &leftValue, lv);
      GET_NUMBER_BY_DATA_TYPE(RightType, &rightValue, rv);

      if constexpr (CompOp == TK_LT)
        result = lv < rv;
      else if constexpr (CompOp == TK_LE)
        result = lv <= rv;
      else if constexpr (CompOp == TK_GT)
        result = lv > rv;
      else if constexpr (CompOp == TK_GE)
        result = lv >= rv;
      else if constexpr (CompOp == TK_EQ)
        result = lv == rv;
      else if constexpr (CompOp == TK_NE)
        result = lv != rv;
    }
    else
    {
      double lv, rv;
      GET_DOUBLE_BY_DATA_TYPE(LeftType, &leftValue, lv);
      GET_DOUBLE_BY_DATA_TYPE(RightType, &rightValue, rv);

      if constexpr (CompOp == TK_LT)
        result = lv < rv;
      else if constexpr (CompOp == TK_LE)
        result = lv <= rv;
      else if constexpr (CompOp == TK_GT)
        result = lv > rv;
      else if constexpr (CompOp == TK_GE)
        result = lv >= rv;
      else if constexpr (CompOp == TK_EQ)
        result = lv == rv;
      else if constexpr (CompOp == TK_NE)
        result = lv != rv;
    }

    DBVAL_SET_BOOL(pResult, result);
    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> leftValVec;
    std::vector<DBVal> rightValVec;
    leftValVec.reserve(resultSize);
    rightValVec.reserve(resultSize);

    retVal = pLeft_->GetValueArray(blockValues, leftValVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (leftValVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    retVal = pRight_->GetValueArray(blockValues, rightValVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (rightValVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    const DBVal* pLeftVals = leftValVec.data();
    const DBVal* pRightVals = rightValVec.data();
    DBVal tmpVal;
    for (size_t idx = 0; idx < resultSize; idx++)
    {
      const DBVal* pLeftVal = pLeftVals + idx;
      const DBVal* pRightVal = pRightVals + idx;

      if (DBVAL_GET_TYPE(pLeftVal) != LeftType || DBVAL_GET_TYPE(pRightVal) != RightType)
      {
        DBVAL_SET_BOOL(&tmpVal, false);
        resultVec.push_back(tmpVal);
      }
      else
      {
        if constexpr (LeftType == PDB_FIELD_TYPE::TYPE_STRING && RightType == PDB_FIELD_TYPE::TYPE_STRING)
        {
          if constexpr (CompOp == TK_EQ)
          {
            DBVAL_SET_BOOL(&tmpVal, false);
            if (DBVAL_GET_LEN(pLeftVal) == DBVAL_GET_LEN(pRightVal))
            {
              DBVAL_SET_BOOL(&tmpVal, (strncmp(DBVAL_GET_STRING(pLeftVal), DBVAL_GET_STRING(pRightVal), DBVAL_GET_LEN(pLeftVal)) == 0));
            }
          }
          else if constexpr (CompOp == TK_NE)
          {
            DBVAL_SET_BOOL(&tmpVal, true);
            if (DBVAL_GET_LEN(pLeftVal) == DBVAL_GET_LEN(pRightVal))
            {
              DBVAL_SET_BOOL(&tmpVal, (strncmp(DBVAL_GET_STRING(pLeftVal), DBVAL_GET_STRING(pRightVal), DBVAL_GET_LEN(pLeftVal)) != 0));
            }
          }
        }
        else if constexpr (LeftType == PDB_FIELD_TYPE::TYPE_BOOL && RightType == PDB_FIELD_TYPE::TYPE_BOOL)
        {
          if constexpr (CompOp == TK_EQ)
            DBVAL_SET_BOOL(&tmpVal, (DBVAL_GET_BOOL(pLeftVal) == DBVAL_GET_BOOL(pRightVal)));
          else if constexpr (CompOp == TK_NE)
            DBVAL_SET_BOOL(&tmpVal, (DBVAL_GET_BOOL(pLeftVal) != DBVAL_GET_BOOL(pRightVal)));
        }
        else if constexpr ((PDB_TYPE_IS_NUMBER(LeftType) && PDB_TYPE_IS_NUMBER(RightType))
          || (LeftType == PDB_FIELD_TYPE::TYPE_DATETIME && RightType == PDB_FIELD_TYPE::TYPE_DATETIME))
        {
          int64_t lv, rv;
          GET_NUMBER_BY_DATA_TYPE(LeftType, pLeftVal, lv);
          GET_NUMBER_BY_DATA_TYPE(RightType, pRightVal, rv);

          if constexpr (CompOp == TK_LT)
            DBVAL_SET_BOOL(&tmpVal, (lv < rv));
          else if constexpr (CompOp == TK_LE)
            DBVAL_SET_BOOL(&tmpVal, (lv <= rv));
          else if constexpr (CompOp == TK_GT)
            DBVAL_SET_BOOL(&tmpVal, (lv > rv));
          else if constexpr (CompOp == TK_GE)
            DBVAL_SET_BOOL(&tmpVal, (lv >= rv));
          else if constexpr (CompOp == TK_EQ)
            DBVAL_SET_BOOL(&tmpVal, (lv == rv));
          else if constexpr (CompOp == TK_NE)
            DBVAL_SET_BOOL(&tmpVal, (lv != rv));
        }
        else
        {
          double lv, rv;
          GET_DOUBLE_BY_DATA_TYPE(LeftType, pLeftVal, lv);
          GET_DOUBLE_BY_DATA_TYPE(RightType, pRightVal, rv);

          if constexpr (CompOp == TK_LT)
            DBVAL_SET_BOOL(&tmpVal, (lv < rv));
          else if constexpr (CompOp == TK_LE)
            DBVAL_SET_BOOL(&tmpVal, (lv <= rv));
          else if constexpr (CompOp == TK_GT)
            DBVAL_SET_BOOL(&tmpVal, (lv > rv));
          else if constexpr (CompOp == TK_GE)
            DBVAL_SET_BOOL(&tmpVal, (lv >= rv));
          else if constexpr (CompOp == TK_EQ)
            DBVAL_SET_BOOL(&tmpVal, (lv == rv));
          else if constexpr (CompOp == TK_NE)
            DBVAL_SET_BOOL(&tmpVal, (lv != rv));
        }

        resultVec.push_back(tmpVal);
      }
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pLeft_->GetValueType() != LeftType)
      return false;

    if (pRight_->GetValueType() != RightType)
      return false;

    if (CompOp != TK_LT && CompOp != TK_LE
      && CompOp != TK_GT && CompOp != TK_GE
      && CompOp != TK_EQ && CompOp != TK_NE)
      return false;

    if constexpr (LeftType == PDB_FIELD_TYPE::TYPE_BOOL && RightType == PDB_FIELD_TYPE::TYPE_BOOL)
    {
      return CompOp == TK_EQ || CompOp == TK_NE;
    }

    if constexpr (LeftType == PDB_FIELD_TYPE::TYPE_STRING && RightType == PDB_FIELD_TYPE::TYPE_STRING)
    {
      return CompOp == TK_EQ || CompOp == TK_NE;
    }

    if constexpr (LeftType == PDB_FIELD_TYPE::TYPE_DATETIME && RightType == PDB_FIELD_TYPE::TYPE_DATETIME)
      return true;

    if constexpr (!(PDB_TYPE_IS_NUMBER(LeftType) || PDB_TYPE_IS_FLOAT_OR_DOUBLE(LeftType)))
      return false;

    if constexpr (!(PDB_TYPE_IS_NUMBER(RightType) || PDB_TYPE_IS_FLOAT_OR_DOUBLE(RightType)))
      return false;

    return true;
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pLeft_ != nullptr)
      pLeft_->GetUseFields(fieldSet);

    if (pRight_ != nullptr)
      pRight_->GetUseFields(fieldSet);
  }

private:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

template<int FieldType, typename T, int CompOp>
class FieldCompareFunction : public ValueItem
{
public:
  FieldCompareFunction(size_t fieldPos, T val)
  {
    this->fieldPos_ = fieldPos;
    this->val_ = val;
  }

  virtual ~FieldCompareFunction() {}

  int32_t GetValueType() const override { return PDB_VALUE_TYPE::VAL_BOOL; }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pResult == nullptr)
      return PdbE_OK;

    bool result = false;
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, FieldType))
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_STRING)
    {
      if constexpr (CompOp == TK_EQ)
      {
        if ((size_t)DBVAL_ELE_GET_LEN(pVals, fieldPos_) == val_.size())
          result = strncmp(DBVAL_ELE_GET_STRING(pVals, fieldPos_), val_.c_str(), val_.size()) == 0;
      }
      else if constexpr (CompOp == TK_NE)
      {
        if ((size_t)DBVAL_ELE_GET_LEN(pVals, fieldPos_) != val_.size())
          result = true;
        else 
          result = strncmp(DBVAL_ELE_GET_STRING(pVals, fieldPos_), val_.c_str(), val_.size()) != 0;
      }
    }
    else if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_BOOL)
    {
      if constexpr (CompOp == TK_EQ)
        result = DBVAL_ELE_GET_BOOL(pVals, fieldPos_) == val_;
      else if constexpr (CompOp == TK_NE)
        result = DBVAL_ELE_GET_BOOL(pVals, fieldPos_) != val_;
    }
    else if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_DATETIME
      || PDB_TYPE_IS_NUMBER(FieldType))
    {
      int64_t fv;
      GET_NUMBER_BY_DATA_TYPE(FieldType, (pVals + fieldPos_), fv);

      if constexpr (CompOp == TK_LT)
        result = fv < val_;
      else if constexpr (CompOp == TK_LE)
        result = fv <= val_;
      else if constexpr (CompOp == TK_GT)
        result = fv > val_;
      else if constexpr (CompOp == TK_GE)
        result = fv >= val_;
      else if constexpr (CompOp == TK_EQ)
        result = fv == val_;
      else if constexpr (CompOp == TK_NE)
        result = fv != val_;
    }
    else
    {
      double fv;
      GET_DOUBLE_BY_DATA_TYPE(FieldType, (pVals + fieldPos_), fv);

      if constexpr (CompOp == TK_LT)
        result = fv < val_;
      else if constexpr (CompOp == TK_LE)
        result = fv <= val_;
      else if constexpr (CompOp == TK_GT)
        result = fv > val_;
      else if constexpr (CompOp == TK_GE)
        result = fv <= val_;
      else if constexpr (CompOp == TK_EQ)
        result = fv == val_;
      else if constexpr (CompOp == TK_NE)
        result = fv != val_;
    }

    DBVAL_SET_BOOL(pResult, result);
    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t recordSize = blockValues.GetRecordSize();
    size_t resultSize = blockValues.GetResultSize();
    const uint8_t* pFilter = blockValues.GetFilter();
    const DBVal* pFieldVals = blockValues.GetColumnValues(fieldPos_);

    if (recordSize == resultSize)
      pFilter = nullptr;

    if (pFieldVals == nullptr)
    {
      DBVal falseVal;
      DBVAL_SET_BOOL(&falseVal, false);
      for (size_t idx = 0; idx < resultSize; idx++)
      {
        resultVec.push_back(falseVal);
      }
    }
    else
    {
      if (pFilter == nullptr)
        retVal = GetValueArrayInner<false>(pFieldVals, pFilter, recordSize, resultVec);
      else
        retVal = GetValueArrayInner<true>(pFieldVals, pFilter, recordSize, resultVec);
    }

    return retVal;
  }

  bool IsValid() const override
  {
    if (FieldType == PDB_FIELD_TYPE::TYPE_BOOL
      || FieldType == PDB_FIELD_TYPE::TYPE_STRING)
    {
      return CompOp == TK_EQ || CompOp == TK_NE;
    }

    if (CompOp != TK_LT && CompOp != TK_LE
      && CompOp != TK_GT && CompOp != TK_GE
      && CompOp != TK_EQ && CompOp != TK_NE)
      return false;

    if (FieldType == PDB_FIELD_TYPE::TYPE_DATETIME)
      return true;

    if (PDB_TYPE_IS_NUMBER(FieldType))
      return true;

    if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(FieldType))
      return true;

    return false;
  }

  bool IsConstValue() const override { return false; }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(fieldPos_);
  }

  bool IsDevIdCondition() const override
  {
    return fieldPos_ == PDB_DEVID_INDEX && FieldType == PDB_VALUE_TYPE::VAL_INT64;
  }

  bool IsTstampCondition() const override
  {
    return fieldPos_ == PDB_TSTAMP_INDEX && FieldType == PDB_VALUE_TYPE::VAL_DATETIME;
  }

  bool GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const override
  {
    if (!IsDevIdCondition())
      return false;

    if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_INT64)
    {
      int64_t minId = 0;
      int64_t maxId = INT64_MAX;

      if constexpr (CompOp == TK_LT || CompOp == TK_LE)
      {
        maxId = val_;
      }
      else if constexpr (CompOp == TK_GT || CompOp == TK_GE)
      {
        minId = val_;
      }
      else if constexpr (CompOp == TK_EQ)
      {
        minId = val_;
        maxId = val_;
      }

      if (pMinDevId != nullptr)
        *pMinDevId = minId;

      if (pMaxDevId != nullptr)
        *pMaxDevId = maxId;

      return true;
    }

    return false;
  }

  bool GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const override
  {
    if (!IsTstampCondition())
      return false;

    if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_DATETIME)
    {
      int64_t minTs = 0;
      int64_t maxTs = INT64_MAX;

      if constexpr (CompOp == TK_LT || CompOp == TK_LE)
      {
        maxTs = val_;
      }
      else if constexpr (CompOp == TK_GT || CompOp == TK_GE)
      {
        minTs = val_;
      }
      else if constexpr (CompOp == TK_EQ)
      {
        minTs = val_;
        maxTs = val_;
      }

      if (pMinTstamp != nullptr)
        *pMinTstamp = minTs;

      if (pMaxTstamp != nullptr)
        *pMaxTstamp = maxTs;

      return true;
    }

    return false;
  }

private:
  template<bool HaveFilter>
  PdbErr_t GetValueArrayInner(const DBVal* pVals, const uint8_t* pFilter,
    size_t recordSize, std::vector<DBVal>& resultVec) const
  {
    DBVal tmpVal;
    for (size_t idx = 0; idx < recordSize; idx++)
    {
      if constexpr (HaveFilter)
      {
        if (pFilter[idx] != PDB_BOOL_TRUE)
          continue;
      }

      const DBVal* pVal = pVals + idx;

      if (DBVAL_GET_TYPE(pVal) != FieldType)
      {
        DBVAL_SET_BOOL(&tmpVal, false);
      }
      else 
      {
        if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_STRING)
        {
          if constexpr (CompOp == TK_EQ)
          {
            DBVAL_SET_BOOL(&tmpVal, false);
            if (DBVAL_GET_LEN(pVal) == val_.size())
            {
              DBVAL_SET_BOOL(&tmpVal, (strncmp(DBVAL_GET_STRING(pVal), val_.c_str(), val_.size()) == 0));
            }
          }
          else if constexpr (CompOp == TK_NE)
          {
            if (DBVAL_GET_LEN(pVal) != val_.size())
              DBVAL_SET_BOOL(&tmpVal, true);
            else
              DBVAL_SET_BOOL(&tmpVal, (strncmp(DBVAL_GET_STRING(pVal), val_.c_str(), val_.size()) != 0));
          }
        }
        else if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_BOOL)
        {
          if constexpr (CompOp == TK_EQ)
            DBVAL_SET_BOOL(&tmpVal, (DBVAL_GET_BOOL(pVal) == val_));
          else if constexpr (CompOp == TK_NE)
            DBVAL_SET_BOOL(&tmpVal, (DBVAL_GET_BOOL(pVal) != val_));
        }
        else if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_DATETIME
          || PDB_TYPE_IS_NUMBER(FieldType))
        {
          int64_t fv;
          GET_NUMBER_BY_DATA_TYPE(FieldType, pVal, fv);

          if constexpr (CompOp == TK_LT)
            DBVAL_SET_BOOL(&tmpVal, (fv < val_));
          else if constexpr (CompOp == TK_LE)
            DBVAL_SET_BOOL(&tmpVal, (fv <= val_));
          else if constexpr (CompOp == TK_GT)
            DBVAL_SET_BOOL(&tmpVal, (fv > val_));
          else if constexpr (CompOp == TK_GE)
            DBVAL_SET_BOOL(&tmpVal, (fv >= val_));
          else if constexpr (CompOp == TK_EQ)
            DBVAL_SET_BOOL(&tmpVal, (fv == val_));
          else if constexpr (CompOp == TK_NE)
            DBVAL_SET_BOOL(&tmpVal, (fv != val_));
        }
        else
        {
          double fv;
          GET_DOUBLE_BY_DATA_TYPE(FieldType, pVal, fv);

          if constexpr (CompOp == TK_LT)
            DBVAL_SET_BOOL(&tmpVal, (fv < val_));
          else if constexpr (CompOp == TK_LE)
            DBVAL_SET_BOOL(&tmpVal, (fv <= val_));
          else if constexpr (CompOp == TK_GT)
            DBVAL_SET_BOOL(&tmpVal, (fv > val_));
          else if constexpr (CompOp == TK_GE)
            DBVAL_SET_BOOL(&tmpVal, (fv >= val_));
          else if constexpr (CompOp == TK_EQ)
            DBVAL_SET_BOOL(&tmpVal, (fv == val_));
          else if constexpr (CompOp == TK_NE)
            DBVAL_SET_BOOL(&tmpVal, (fv != val_));
        }
      }
      
      resultVec.push_back(tmpVal);
    }
    

    return PdbE_OK;
  }

private:
  size_t fieldPos_;
  T val_;
};

class AndFunction : public ValueItem
{
public:
  AndFunction() { }

  virtual ~AndFunction()
  {
    for (size_t idx = 0; idx < condiVec_.size(); idx++)
    {
      delete condiVec_[idx];
    }
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    DBVal tmpRet;
    PdbErr_t retVal = PdbE_OK;
    DBVAL_SET_BOOL(&tmpRet, true);
    for (auto condiIt = condiVec_.begin(); condiIt != condiVec_.end(); condiIt++)
    {
      retVal = (*condiIt)->GetValue(pVals, &tmpRet);
      if (retVal != PdbE_OK)
        return retVal;

      if (!DBVAL_GET_BOOL(&tmpRet))
        break;
    }

    *pResult = tmpRet;
    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> val1Vec;
    std::vector<DBVal> val2Vec;
    val1Vec.reserve(resultSize);
    val2Vec.reserve(resultSize);

    retVal = condiVec_[0]->GetValueArray(blockValues, val1Vec);
    if (retVal != PdbE_OK)
      return retVal;

    for (size_t idx = 1; idx < condiVec_.size(); idx++)
    {
      val2Vec.clear();

      retVal = condiVec_[idx]->GetValueArray(blockValues, val2Vec);
      if (retVal != PdbE_OK)
        return retVal;

      //Merge Result
      DBVal* pVals1 = val1Vec.data();
      DBVal* pVals2 = val2Vec.data();

      for (size_t i = 0; i < resultSize; i++)
      {
        DBVAL_SET_BOOL(pVals1, (DBVAL_GET_BOOL(pVals1) && DBVAL_GET_BOOL(pVals2)));
        pVals1++;
        pVals2++;
      }
    }

    resultVec.insert(resultVec.end(), val1Vec.begin(), val1Vec.end());
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (condiVec_.size() == 0)
      return false;

    for (auto condiIt = condiVec_.begin(); condiIt != condiVec_.end(); condiIt++)
    {
      if (!(*condiIt)->IsValid())
        return false;

      if ((*condiIt)->GetValueType() != PDB_VALUE_TYPE::VAL_BOOL)
        return false;
    }

    return true;
  }

  bool IsConstValue() const override
  {
    for (auto condiIt = condiVec_.begin(); condiIt != condiVec_.end(); condiIt++)
    {
      if (!(*condiIt)->IsConstValue())
        return false;
    }

    return true;
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    for (auto valIt = condiVec_.begin(); valIt != condiVec_.end(); valIt++)
    {
      (*valIt)->GetUseFields(fieldSet);
    }
  }

  void AddValueItem(ValueItem* pItem)
  {
    if (pItem != nullptr)
    {
      condiVec_.push_back(pItem);
    }
  }

private:
  std::vector<ValueItem*> condiVec_;
};

int GetFunctionId(const char* pName, size_t nameLen)
{
  if (pName == nullptr || nameLen < 2 || nameLen > 20)
    return 0;

  if (pName[0] == 'A' || pName[0] == 'a')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "AVG", (sizeof("AVG") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_AVG;
    if (StringTool::ComparyNoCase(pName, nameLen, "AVGIF", (sizeof("AVGIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_AVG_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "ABS", (sizeof("ABS") - 1)))
      return PDB_SQL_FUNC::FUNC_ABS;
    else if (StringTool::ComparyNoCase(pName, nameLen, "ADD", (sizeof("ADD") - 1)))
      return PDB_SQL_FUNC::FUNC_ADD;
  }
  else if (pName[0] == 'D' || pName[0] == 'd')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "DIV", (sizeof("DIV") - 1)))
      return PDB_SQL_FUNC::FUNC_DIV;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMEADD", (sizeof("DATETIMEADD") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMEADD;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMEDIFF", (sizeof("DATETIMEDIFF") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMEDIFF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMEFLOOR", (sizeof("DATETIMEFLOOR") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMEFLOOR;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMECEIL", (sizeof("DATETIMECEIL") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMECEIL;
  }
  else if (pName[0] == 'M' || pName[0] == 'm')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "MIN", (sizeof("MIN") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MIN;
    if (StringTool::ComparyNoCase(pName, nameLen, "MINIF", (sizeof("MINIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MIN_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MAX", (sizeof("MAX") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MAX;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MAXIF", (sizeof("MAXIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MAX_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MUL", (sizeof("MUL") - 1)))
      return PDB_SQL_FUNC::FUNC_MUL;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MOD", (sizeof("MOD") - 1)))
      return PDB_SQL_FUNC::FUNC_MOD;
  }
  else if (pName[0] == 'S' || pName[0] == 's')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "SUM", (sizeof("SUM") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_SUM;
    if (StringTool::ComparyNoCase(pName, nameLen, "SUMIF", (sizeof("SUMIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_SUM_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "SUB", (sizeof("SUB") - 1)))
      return PDB_SQL_FUNC::FUNC_SUB;
  }
  else if (pName[0] == 'C' || pName[0] == 'c')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "COUNT", (sizeof("COUNT") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_COUNT;
    if (StringTool::ComparyNoCase(pName, nameLen, "COUNTIF", (sizeof("COUNTIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_COUNT_IF;
  }
  else if (pName[0] == 'F' || pName[0] == 'f')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "FIRST", (sizeof("FIRST") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_FIRST;
    else if (StringTool::ComparyNoCase(pName, nameLen, "FIRSTIF", (sizeof("FIRSTIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_FIRST_IF;
  }
  else if (pName[0] == 'L' || pName[0] == 'l')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "LAST", (sizeof("LAST") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_LAST;
    else if (StringTool::ComparyNoCase(pName, nameLen, "LASTIF", (sizeof("LASTIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_LAST_IF;
  }
  else
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "NOW", (sizeof("NOW") - 1)))
      return PDB_SQL_FUNC::FUNC_NOW;
    else if (StringTool::ComparyNoCase(pName, nameLen, "IF", (sizeof("IF") - 1)))
      return PDB_SQL_FUNC::FUNC_IF;
  }

  return 0;
}

void DeleteValueVec(std::vector<ValueItem*>* pValVec)
{
  for (size_t idx = 0; idx < pValVec->size(); idx++)
  {
    delete (*pValVec)[idx];
  }
}

bool ConvertExprToValue(const TableInfo* pTabInfo, int64_t nowMicroseconds,
  const ExprValueList* pArgList, std::vector<ValueItem*>* pValVec)
{
  if (pArgList == nullptr)
    return true;

  const std::vector<ExprValue*>* pExprVec = pArgList->GetValueList();
  for (auto argIt = pExprVec->begin(); argIt != pExprVec->end(); argIt++)
  {
    ValueItem* pTmpVal = BuildGeneralValueItem(pTabInfo, *argIt, nowMicroseconds);
    if (pTmpVal == nullptr)
      break;

    pValVec->push_back(pTmpVal);
  }

  if (pValVec->size() != pExprVec->size())
  {
    DeleteValueVec(pValVec);
    return false;
  }

  return true;
}

template<int LeftType, int RightType>
ValueItem* Create2ParamOpFunctionStep2(int op, ValueItem* pLeftValue, ValueItem* pRightValue)
{
  switch (op)
  {
  case PDB_SQL_FUNC::FUNC_ADD:
    return new CalculateFunction<PDB_SQL_FUNC::FUNC_ADD, LeftType, RightType>(pLeftValue, pRightValue);
  case PDB_SQL_FUNC::FUNC_SUB:
    return new CalculateFunction<PDB_SQL_FUNC::FUNC_SUB, LeftType, RightType>(pLeftValue, pRightValue);
  case PDB_SQL_FUNC::FUNC_MUL:
    return new CalculateFunction<PDB_SQL_FUNC::FUNC_MUL, LeftType, RightType>(pLeftValue, pRightValue);
  case PDB_SQL_FUNC::FUNC_DIV:
    return new CalculateFunction<PDB_SQL_FUNC::FUNC_DIV, LeftType, RightType>(pLeftValue, pRightValue);
  case PDB_SQL_FUNC::FUNC_MOD:
    return new CalculateFunction<PDB_SQL_FUNC::FUNC_MOD, LeftType, RightType>(pLeftValue, pRightValue);
  }

  return nullptr;
}

template<int LeftType>
ValueItem* Create2ParamOpFunctionStep1(int op, int rightType, ValueItem* pLeftValue, ValueItem* pRightValue)
{
  switch (rightType)
  {
  case PDB_VALUE_TYPE::VAL_INT8:
    return Create2ParamOpFunctionStep2<LeftType, PDB_VALUE_TYPE::VAL_INT8>(op, pLeftValue, pRightValue);
  case PDB_VALUE_TYPE::VAL_INT16:
    return Create2ParamOpFunctionStep2<LeftType, PDB_VALUE_TYPE::VAL_INT16>(op, pLeftValue, pRightValue);
  case PDB_VALUE_TYPE::VAL_INT32:
    return Create2ParamOpFunctionStep2<LeftType, PDB_VALUE_TYPE::VAL_INT32>(op, pLeftValue, pRightValue);
  case PDB_VALUE_TYPE::VAL_INT64:
    return Create2ParamOpFunctionStep2<LeftType, PDB_VALUE_TYPE::VAL_INT64>(op, pLeftValue, pRightValue);
  case PDB_VALUE_TYPE::VAL_FLOAT:
    return Create2ParamOpFunctionStep2<LeftType, PDB_VALUE_TYPE::VAL_FLOAT>(op, pLeftValue, pRightValue);
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return Create2ParamOpFunctionStep2<LeftType, PDB_VALUE_TYPE::VAL_DOUBLE>(op, pLeftValue, pRightValue);
  }

  return nullptr;
}

ValueItem* Create2ParamOpFunction(int op, const TableInfo* pTabInfo, int64_t nowMicroseconds, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
    return nullptr;

  if (!ConvertExprToValue(pTabInfo, nowMicroseconds, pArgList, &argVec))
    return nullptr;

  ValueItem* pResult = nullptr;

  do {
    if (argVec.size() != 2)
      break;

    int32_t leftType = argVec[0]->GetValueType();
    int32_t rightType = argVec[1]->GetValueType();

    switch (leftType)
    {
    case PDB_VALUE_TYPE::VAL_INT8:
      pResult = Create2ParamOpFunctionStep1<PDB_VALUE_TYPE::VAL_INT8>(op, rightType, argVec[0], argVec[1]);
      break;
    case PDB_VALUE_TYPE::VAL_INT16:
      pResult = Create2ParamOpFunctionStep1<PDB_VALUE_TYPE::VAL_INT16>(op, rightType, argVec[0], argVec[1]);
      break;
    case PDB_VALUE_TYPE::VAL_INT32:
      pResult = Create2ParamOpFunctionStep1<PDB_VALUE_TYPE::VAL_INT32>(op, rightType, argVec[0], argVec[1]);
      break;
    case PDB_VALUE_TYPE::VAL_INT64:
      pResult = Create2ParamOpFunctionStep1<PDB_VALUE_TYPE::VAL_INT64>(op, rightType, argVec[0], argVec[1]);
      break;
    case PDB_VALUE_TYPE::VAL_FLOAT:
      pResult = Create2ParamOpFunctionStep1<PDB_VALUE_TYPE::VAL_FLOAT>(op, rightType, argVec[0], argVec[1]);
      break;
    case PDB_VALUE_TYPE::VAL_DOUBLE:
      pResult = Create2ParamOpFunctionStep1<PDB_VALUE_TYPE::VAL_DOUBLE>(op, rightType, argVec[0], argVec[1]);
      break;
    }
  } while (false);

  if (pResult == nullptr)
  {
    DeleteValueVec(&argVec);
  }

  return pResult;
}

ValueItem* CreateDateTimeFunction(int op, const TableInfo* pTabInfo, int64_t nowMicroseconds, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
    return nullptr;

  if (!ConvertExprToValue(pTabInfo, nowMicroseconds, pArgList, &argVec))
    return nullptr;

  ValueItem* pResult = nullptr;

  do {
    if (argVec.size() != 2)
      break;

    if (argVec[0]->GetValueType() != PDB_VALUE_TYPE::VAL_DATETIME)
    {
      ValueItem* pTmpDt = ConvertStringToDateTime(argVec[0]);
      if (pTmpDt != nullptr)
      {
        delete argVec[0];
        argVec[0] = pTmpDt;
      }
      else
      {
        break;
      }
    }

    if (op == PDB_SQL_FUNC::FUNC_DATETIMEADD)
    {
      DateTimeAdd* pTimeAdd = new DateTimeAdd(argVec[0], argVec[1]);
      pResult = pTimeAdd;
    }
    else if (op == PDB_SQL_FUNC::FUNC_DATETIMEDIFF)
    {
      DateTimeDiff* pTimeDiff = new DateTimeDiff(argVec[0], argVec[1]);
      pResult = pTimeDiff;
    }
    else if (op == PDB_SQL_FUNC::FUNC_DATETIMEFLOOR || op == PDB_SQL_FUNC::FUNC_DATETIMECEIL)
    {
      DBVal tmpUnit;
      if (!argVec[1]->IsConstValue())
        break;

      PdbErr_t retVal = argVec[1]->GetValue(nullptr, &tmpUnit);
      if (retVal != PdbE_OK)
        break;

      delete argVec[1];
      argVec.erase((argVec.begin() + 1));

      if (!DBVAL_IS_STRING(&tmpUnit))
        break;

      int64_t microOffset = 0;
      if (StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "s", (sizeof("s") - 1)),
        StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "second", (sizeof("second") - 1)))
      {
        microOffset = DateTime::MicrosecondPerSecond;
      }
      else if (StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "m", (sizeof("m") - 1)),
        StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "minute", (sizeof("minute") - 1)))
      {
        microOffset = DateTime::MicrosecondPerMinute;
      }
      else if (StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "h", (sizeof("h") - 1)),
        StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "hour", (sizeof("hour") - 1)))
      {
        microOffset = DateTime::MicrosecondPerHour;
      }
      else if (StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "d", (sizeof("d") - 1)),
        StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "day", (sizeof("day") - 1)))
      {
        microOffset = DateTime::MicrosecondPerDay;
      }
      else
      {
        break;
      }

      if (op == PDB_SQL_FUNC::FUNC_DATETIMEFLOOR)
      {
        pResult = new DateTimeAlign<false>(argVec[0], microOffset);
      }
      else if (op == PDB_SQL_FUNC::FUNC_DATETIMECEIL)
      {
        pResult = new DateTimeAlign<true>(argVec[0], microOffset);
      }
    }

  } while (false);

  if (pResult == nullptr)
  {
    DeleteValueVec(&argVec);
  }

  return pResult;
}

ValueItem* CreateNowFunction(const TableInfo* pTabInfo, int64_t nowMicroseconds, const ExprValueList* pArgList)
{
  if (pArgList != nullptr)
  {
    const std::vector<ExprValue*>* pExprVec = pArgList->GetValueList();
    if (pExprVec->size() > 0)
      return nullptr;
  }

  return new ConstValue(nowMicroseconds, true);
}

ValueItem* CreateAbsFunction(const TableInfo* pTabInfo, int64_t nowMicroseconds, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
    return nullptr;

  if (!ConvertExprToValue(pTabInfo, nowMicroseconds, pArgList, &argVec))
    return nullptr;

  if (argVec.size() == 1)
  {
    int32_t valueType = argVec[0]->GetValueType();
    switch (valueType)
    {
    case PDB_FIELD_TYPE::TYPE_INT8:
      return new AbsFunction<PDB_FIELD_TYPE::TYPE_INT8>(argVec[0]);
    case PDB_FIELD_TYPE::TYPE_INT16:
      return new AbsFunction<PDB_FIELD_TYPE::TYPE_INT16>(argVec[0]);
    case PDB_FIELD_TYPE::TYPE_INT32:
      return new AbsFunction<PDB_FIELD_TYPE::TYPE_INT32>(argVec[0]);
    case PDB_FIELD_TYPE::TYPE_INT64:
      return new AbsFunction<PDB_FIELD_TYPE::TYPE_INT64>(argVec[0]);
    case PDB_FIELD_TYPE::TYPE_FLOAT:
      return new AbsFunction<PDB_FIELD_TYPE::TYPE_FLOAT>(argVec[0]);
    case PDB_FIELD_TYPE::TYPE_DOUBLE:
      return new AbsFunction<PDB_FIELD_TYPE::TYPE_DOUBLE>(argVec[0]);
    }
  }

  DeleteValueVec(&argVec);
  return nullptr;
}

template<int ResultType, int Param1Type>
ValueItem* CreateIfFunctionStep3(std::vector<ValueItem*>& argVec)
{
  int32_t param2Type = argVec[2]->GetValueType();

  switch (param2Type)
  {
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new IfFunction<ResultType, Param1Type, PDB_FIELD_TYPE::TYPE_INT8>(argVec[0], argVec[1], argVec[2]);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new IfFunction<ResultType, Param1Type, PDB_FIELD_TYPE::TYPE_INT16>(argVec[0], argVec[1], argVec[2]);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new IfFunction<ResultType, Param1Type, PDB_FIELD_TYPE::TYPE_INT32>(argVec[0], argVec[1], argVec[2]);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new IfFunction<ResultType, Param1Type, PDB_FIELD_TYPE::TYPE_INT64>(argVec[0], argVec[1], argVec[2]);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return new IfFunction<ResultType, Param1Type, PDB_FIELD_TYPE::TYPE_FLOAT>(argVec[0], argVec[1], argVec[2]);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new IfFunction<ResultType, Param1Type, PDB_FIELD_TYPE::TYPE_DOUBLE>(argVec[0], argVec[1], argVec[2]);
  }

  return nullptr;
}

template<int ResultType>
ValueItem* CreateIfFunctionStep2(std::vector<ValueItem*>& argVec)
{
  int32_t param1Type = argVec[1]->GetValueType();

  switch (param1Type)
  {
  case PDB_FIELD_TYPE::TYPE_INT8:
    return CreateIfFunctionStep3<ResultType, PDB_FIELD_TYPE::TYPE_INT8>(argVec);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return CreateIfFunctionStep3<ResultType, PDB_FIELD_TYPE::TYPE_INT16>(argVec);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return CreateIfFunctionStep3<ResultType, PDB_FIELD_TYPE::TYPE_INT32>(argVec);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return CreateIfFunctionStep3<ResultType, PDB_FIELD_TYPE::TYPE_INT64>(argVec);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return CreateIfFunctionStep3<ResultType, PDB_FIELD_TYPE::TYPE_FLOAT>(argVec);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return CreateIfFunctionStep3<ResultType, PDB_FIELD_TYPE::TYPE_DOUBLE>(argVec);
  }

  return nullptr;
}

ValueItem* CreateIfFunction(const TableInfo* pTabInfo, int64_t nowMicroseconds, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
    return nullptr;

  if (!ConvertExprToValue(pTabInfo, nowMicroseconds, pArgList, &argVec))
    return nullptr;

  if (argVec.size() == 3)
  {
    if (argVec[0]->GetValueType() == PDB_VALUE_TYPE::VAL_BOOL)
    {
      int32_t param1Type = argVec[1]->GetValueType();
      int32_t param2Type = argVec[2]->GetValueType();

      if (param1Type != param2Type)
      {
        if (param1Type == PDB_VALUE_TYPE::VAL_STRING && param2Type == PDB_VALUE_TYPE::VAL_DATETIME)
        {
          ValueItem* pTmpItem = ConvertStringToDateTime(argVec[1]);
          if (pTmpItem != nullptr)
          {
            delete argVec[1];
            argVec[1] = pTmpItem;
            param1Type = argVec[1]->GetValueType();
          }
        }

        if (param1Type == PDB_VALUE_TYPE::VAL_DATETIME && param2Type == PDB_VALUE_TYPE::VAL_STRING)
        {
          ValueItem* pTmpItem = ConvertStringToDateTime(argVec[2]);
          if (pTmpItem != nullptr)
          {
            delete argVec[2];
            argVec[2] = pTmpItem;
            param2Type = argVec[2]->GetValueType();
          }
        }
      }

      if (param1Type == param2Type)
      {
        switch (param1Type)
        {
        case PDB_FIELD_TYPE::TYPE_BOOL:
          return new IfFunction<PDB_FIELD_TYPE::TYPE_BOOL, PDB_FIELD_TYPE::TYPE_BOOL, PDB_FIELD_TYPE::TYPE_BOOL>(argVec[0], argVec[1], argVec[2]);
        case PDB_FIELD_TYPE::TYPE_INT8:
          return new IfFunction<PDB_FIELD_TYPE::TYPE_INT8, PDB_FIELD_TYPE::TYPE_INT8, PDB_FIELD_TYPE::TYPE_INT8>(argVec[0], argVec[1], argVec[2]);
        case PDB_FIELD_TYPE::TYPE_INT16:
          return new IfFunction<PDB_FIELD_TYPE::TYPE_INT16, PDB_FIELD_TYPE::TYPE_INT16, PDB_FIELD_TYPE::TYPE_INT16>(argVec[0], argVec[1], argVec[2]);
        case PDB_FIELD_TYPE::TYPE_INT32:
          return new IfFunction<PDB_FIELD_TYPE::TYPE_INT32, PDB_FIELD_TYPE::TYPE_INT32, PDB_FIELD_TYPE::TYPE_INT32>(argVec[0], argVec[1], argVec[2]);
        case PDB_FIELD_TYPE::TYPE_INT64:
          return new IfFunction<PDB_FIELD_TYPE::TYPE_INT64, PDB_FIELD_TYPE::TYPE_INT64, PDB_FIELD_TYPE::TYPE_INT64>(argVec[0], argVec[1], argVec[2]);
        case PDB_FIELD_TYPE::TYPE_DATETIME:
          return new IfFunction<PDB_FIELD_TYPE::TYPE_DATETIME, PDB_FIELD_TYPE::TYPE_DATETIME, PDB_FIELD_TYPE::TYPE_DATETIME>(argVec[0], argVec[1], argVec[2]);
        case PDB_FIELD_TYPE::TYPE_FLOAT:
          return new IfFunction<PDB_FIELD_TYPE::TYPE_FLOAT, PDB_FIELD_TYPE::TYPE_FLOAT, PDB_FIELD_TYPE::TYPE_FLOAT>(argVec[0], argVec[1], argVec[2]);
        case PDB_FIELD_TYPE::TYPE_DOUBLE:
          return new IfFunction<PDB_FIELD_TYPE::TYPE_DOUBLE, PDB_FIELD_TYPE::TYPE_DOUBLE, PDB_FIELD_TYPE::TYPE_DOUBLE>(argVec[0], argVec[1], argVec[2]);
        case PDB_FIELD_TYPE::TYPE_STRING:
          return new IfFunction<PDB_FIELD_TYPE::TYPE_STRING, PDB_FIELD_TYPE::TYPE_STRING, PDB_FIELD_TYPE::TYPE_STRING>(argVec[0], argVec[1], argVec[2]);
        }
      }
      else if (PDB_TYPE_IS_NUMBER(param1Type) && PDB_TYPE_IS_NUMBER(param2Type))
      {
        ValueItem* pResult = CreateIfFunctionStep2<PDB_FIELD_TYPE::TYPE_INT64>(argVec);
        if (pResult != nullptr)
          return pResult;
      }
      else if ((PDB_TYPE_IS_NUMBER(param1Type) || PDB_TYPE_IS_FLOAT_OR_DOUBLE(param1Type))
        && (PDB_TYPE_IS_NUMBER(param2Type) || PDB_TYPE_IS_FLOAT_OR_DOUBLE(param2Type)))
      {
        ValueItem* pResult = CreateIfFunctionStep2<PDB_FIELD_TYPE::TYPE_DOUBLE>(argVec);
        if (pResult != nullptr)
          return pResult;
      }
    }
  }

  DeleteValueVec(&argVec);
  return nullptr;
}

ValueItem* CreateFieldValue(int fieldType, size_t fieldPos)
{
  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_BOOL:
    return new FieldValue<PDB_FIELD_TYPE::TYPE_BOOL>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new FieldValue<PDB_FIELD_TYPE::TYPE_INT8>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new FieldValue<PDB_FIELD_TYPE::TYPE_INT16>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new FieldValue<PDB_FIELD_TYPE::TYPE_INT32>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new FieldValue<PDB_FIELD_TYPE::TYPE_INT64>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    return new FieldValue<PDB_FIELD_TYPE::TYPE_DATETIME>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return new FieldValue<PDB_FIELD_TYPE::TYPE_FLOAT>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new FieldValue<PDB_FIELD_TYPE::TYPE_DOUBLE>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_STRING:
    return new FieldValue<PDB_FIELD_TYPE::TYPE_STRING>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_BLOB:
    return new FieldValue<PDB_FIELD_TYPE::TYPE_BLOB>(fieldPos);
  }

  return nullptr;
}


ValueItem* CreateFieldValueByExpr(const TableInfo* pTableInfo, const ExprValue* pExpr)
{
  if (pTableInfo == nullptr || pExpr == nullptr)
    return nullptr;

  DBVal fieldVal = pExpr->GetValue();
  if (!DBVAL_IS_STRING(&fieldVal))
    return nullptr;

  PdbErr_t retVal = PdbE_OK;
  size_t fieldPos = 0;
  int32_t fieldType = 0;
  std::string fieldName = std::string(DBVAL_GET_STRING(&fieldVal), DBVAL_GET_LEN(&fieldVal));
  retVal = pTableInfo->GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
  if (retVal != PdbE_OK)
    return nullptr;

  return CreateFieldValue(fieldType, fieldPos);
}

ValueItem* CreateIsNullOrNullFunction(bool notNull, const TableInfo* pTabInfo, const ExprValue* pExpr)
{
  ValueItem* pFieldValue = CreateFieldValueByExpr(pTabInfo, pExpr);
  if (pFieldValue == nullptr)
    return nullptr;

  if (notNull)
    return new NullFunction<true>(pFieldValue);
  else
    return new NullFunction<false>(pFieldValue);
}

ValueItem* CreateLikeFunction(const TableInfo* pTableInfo, const ExprValue* pExpr)
{
  if (pTableInfo == nullptr || pExpr == nullptr)
    return nullptr;

  const ExprValue* pPatVal = pExpr->GetLeftParam();
  if (pPatVal == nullptr)
    return nullptr;

  if (pPatVal->GetValueType() != TK_STRING)
    return nullptr;

  DBVal patStr = pPatVal->GetValue();
  if (!DBVAL_IS_STRING(&patStr))
    return nullptr;

  ValueItem* pFieldValue = CreateFieldValueByExpr(pTableInfo, pExpr);
  if (pFieldValue == nullptr)
    return nullptr;

  return new LikeFunction(pFieldValue, DBVAL_GET_STRING(&patStr), DBVAL_GET_LEN(&patStr));
}


template<bool NotIn>
ValueItem* CreateInOrNotInFunction(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMicroseconds)
{
  if (pTableInfo == nullptr || pExpr == nullptr)
    return nullptr;

  DBVal fieldVal = pExpr->GetValue();
  if (!DBVAL_IS_STRING(&fieldVal))
    return nullptr;

  PdbErr_t retVal = PdbE_OK;
  size_t fieldPos = 0;
  int32_t fieldType = 0;
  std::string fieldName = std::string(DBVAL_GET_STRING(&fieldVal), DBVAL_GET_LEN(&fieldVal));
  retVal = pTableInfo->GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
  if (retVal != PdbE_OK)
    return nullptr;

  int32_t cmpType = 0;
  if (PDB_TYPE_IS_NUMBER(fieldType))
    cmpType = PDB_FIELD_TYPE::TYPE_INT64;
  else if (fieldType == PDB_FIELD_TYPE::TYPE_STRING)
    cmpType = PDB_FIELD_TYPE::TYPE_STRING;
  else
    return nullptr;

  bool errFlag = false;
  DBVal tmpVal;
  const ExprValueList* pArgList = pExpr->GetArgList();
  const std::vector<ExprValue*>* pValVec = pArgList->GetValueList();
  std::list<int64_t> argList;
  for (auto argIt = pValVec->begin(); argIt != pValVec->end(); argIt++)
  {
    ValueItem* pTmpVal = BuildGeneralValueItem(pTableInfo, (*argIt), nowMicroseconds);
    if (pTmpVal == nullptr)
    {
      errFlag = true;
      break;
    }

    do {
      if (!pTmpVal->IsValid())
      {
        errFlag = true;
        break;
      }

      if (!pTmpVal->IsConstValue())
      {
        errFlag = true;
        break;
      }

      if (pTmpVal->GetValue(nullptr, &tmpVal) != PdbE_OK)
      {
        errFlag = true;
        break;
      }

      if (DBVAL_GET_TYPE(&tmpVal) != cmpType)
      {
        errFlag = true;
        break;
      }

      if (cmpType == PDB_FIELD_TYPE::TYPE_STRING)
      {
        argList.push_back(static_cast<int64_t>(StringTool::CRC64(DBVAL_GET_STRING(&tmpVal), DBVAL_GET_LEN(&tmpVal))));
      }
      else
      {
        argList.push_back(DBVAL_GET_INT64(&tmpVal));
      }
    } while (false);

    delete pTmpVal;

    if (errFlag)
      break;
  }

  if (errFlag || argList.empty())
    return nullptr;

  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new InFunction<NotIn, PDB_FIELD_TYPE::TYPE_INT8>(fieldPos, argList);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new InFunction<NotIn, PDB_FIELD_TYPE::TYPE_INT16>(fieldPos, argList);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new InFunction<NotIn, PDB_FIELD_TYPE::TYPE_INT32>(fieldPos, argList);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new InFunction<NotIn, PDB_FIELD_TYPE::TYPE_INT64>(fieldPos, argList);
  case PDB_FIELD_TYPE::TYPE_STRING:
    return new InFunction<NotIn, PDB_FIELD_TYPE::TYPE_STRING>(fieldPos, argList);
  }

  return nullptr;
}

ValueItem* CreateGeneralFunction(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMicroseconds)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  size_t fieldPos = 0;
  char uniqueName[32];
  if (pTableInfo == nullptr || pExpr == nullptr)
    return nullptr;

  if (pExpr->GetValueType() != TK_FUNCTION)
    return nullptr;

  DBVal nameVal = pExpr->GetValue();
  if (!DBVAL_IS_STRING(&nameVal))
    return nullptr;

  if (DBVAL_GET_LEN(&nameVal) < 2 || DBVAL_GET_LEN(&nameVal) > 20)
    return nullptr;

  int funcId = GetFunctionId(DBVAL_GET_STRING(&nameVal), DBVAL_GET_LEN(&nameVal));
  if (funcId <= 0)
    return nullptr;

  ValueItem* pResultVal = nullptr;

  do {
    if (funcId == PDB_SQL_FUNC::FUNC_AGG_COUNT
      || funcId == PDB_SQL_FUNC::FUNC_AGG_FIRST
      || funcId == PDB_SQL_FUNC::FUNC_AGG_LAST
      || funcId == PDB_SQL_FUNC::FUNC_AGG_AVG
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MIN
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MAX
      || funcId == PDB_SQL_FUNC::FUNC_AGG_SUM
      || funcId == PDB_SQL_FUNC::FUNC_AGG_COUNT_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_FIRST_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_LAST_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_AVG_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MIN_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MAX_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_SUM_IF)
    {
#ifdef _WIN32
      sprintf(uniqueName, "agg_%llu", reinterpret_cast<uintptr_t>(pExpr));
#else
      sprintf(uniqueName, "agg_%lu", reinterpret_cast<uintptr_t>(pExpr));
#endif
      retVal = pTableInfo->GetFieldInfo(uniqueName, &fieldPos, &fieldType);
      if (retVal != PdbE_OK)
        return nullptr;

      return CreateFieldValue(fieldType, fieldPos);
    }

    switch (funcId)
    {
    case PDB_SQL_FUNC::FUNC_ADD:
    case PDB_SQL_FUNC::FUNC_SUB:
    case PDB_SQL_FUNC::FUNC_MUL:
    case PDB_SQL_FUNC::FUNC_DIV:
    case PDB_SQL_FUNC::FUNC_MOD:
      pResultVal = Create2ParamOpFunction(funcId, pTableInfo, nowMicroseconds, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_DATETIMEADD:
    case PDB_SQL_FUNC::FUNC_DATETIMEDIFF:
    case PDB_SQL_FUNC::FUNC_DATETIMEFLOOR:
    case PDB_SQL_FUNC::FUNC_DATETIMECEIL:
      pResultVal = CreateDateTimeFunction(funcId, pTableInfo, nowMicroseconds, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_IF:
      pResultVal = CreateIfFunction(pTableInfo, nowMicroseconds, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_ABS:
      pResultVal = CreateAbsFunction(pTableInfo, nowMicroseconds, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_NOW:
      pResultVal = CreateNowFunction(pTableInfo, nowMicroseconds, pExpr->GetArgList());
      break;
    }

  } while (false);

  if (pResultVal != nullptr)
  {
    if (pResultVal->IsValid())
      return pResultVal;

    delete pResultVal;
  }

  return nullptr;
}

template<int FieldType, typename T>
ValueItem* CreateFieldCompare(int op, size_t fieldPos, T val)
{
  switch (op)
  {
  case TK_LT:
    return new FieldCompareFunction<FieldType, T, TK_LT>(fieldPos, val);
  case TK_LE:
    return new FieldCompareFunction<FieldType, T, TK_LE>(fieldPos, val);
  case TK_GT:
    return new FieldCompareFunction<FieldType, T, TK_GT>(fieldPos, val);
  case TK_GE:
    return new FieldCompareFunction<FieldType, T, TK_GE>(fieldPos, val);
  case TK_EQ:
    return new FieldCompareFunction<FieldType, T, TK_EQ>(fieldPos, val);
  case TK_NE:
    return new FieldCompareFunction<FieldType, T, TK_NE>(fieldPos, val);
  }

  return nullptr;
}

template<int LeftType, int RightType, typename T>
ValueItem* CreateValueCompareLeaf(int op, ValueItem* pLeft, ValueItem* pRight)
{
  switch (op)
  {
  case TK_LT:
    return new ValueCompareFunction<LeftType, RightType, TK_LT>(pLeft, pRight);
  case TK_LE:
    return new ValueCompareFunction<LeftType, RightType, TK_LE>(pLeft, pRight);
  case TK_GT:
    return new ValueCompareFunction<LeftType, RightType, TK_GT>(pLeft, pRight);
  case TK_GE:
    return new ValueCompareFunction<LeftType, RightType, TK_GE>(pLeft, pRight);
  case TK_EQ:
    return new ValueCompareFunction<LeftType, RightType, TK_EQ>(pLeft, pRight);
  case TK_NE:
    return new ValueCompareFunction<LeftType, RightType, TK_NE>(pLeft, pRight);
  }
  return nullptr;
}

template<int LeftType, typename T>
ValueItem* CreateValueCompareStep(int op, ValueItem* pLeft, ValueItem* pRight)
{
  int rightType = pRight->GetValueType();

  switch (rightType)
  {
  case PDB_FIELD_TYPE::TYPE_INT8:
    return CreateValueCompareLeaf<LeftType, PDB_FIELD_TYPE::TYPE_INT8, T>(op, pLeft, pRight);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return CreateValueCompareLeaf<LeftType, PDB_FIELD_TYPE::TYPE_INT16, T>(op, pLeft, pRight);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return CreateValueCompareLeaf<LeftType, PDB_FIELD_TYPE::TYPE_INT32, T>(op, pLeft, pRight);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return CreateValueCompareLeaf<LeftType, PDB_FIELD_TYPE::TYPE_INT64, T>(op, pLeft, pRight);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return CreateValueCompareLeaf<LeftType, PDB_FIELD_TYPE::TYPE_FLOAT, T>(op, pLeft, pRight);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return CreateValueCompareLeaf<LeftType, PDB_FIELD_TYPE::TYPE_DOUBLE, T>(op, pLeft, pRight);
  }

  return nullptr;
}

ValueItem* CreateValueCompare(int op, ValueItem* pLeft, ValueItem* pRight)
{
  int leftType = pLeft->GetValueType();
  int rightType = pRight->GetValueType();

  if (PDB_TYPE_IS_NUMBER(leftType) && PDB_TYPE_IS_NUMBER(rightType))
  {
    switch (leftType)
    {
    case PDB_FIELD_TYPE::TYPE_INT8:
      return CreateValueCompareStep<PDB_FIELD_TYPE::TYPE_INT8, int64_t>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_INT16:
      return CreateValueCompareStep<PDB_FIELD_TYPE::TYPE_INT16, int64_t>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_INT32:
      return CreateValueCompareStep<PDB_FIELD_TYPE::TYPE_INT32, int64_t>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_INT64:
      return CreateValueCompareStep<PDB_FIELD_TYPE::TYPE_INT64, int64_t>(op, pLeft, pRight);
    }
  }
  else if ((PDB_TYPE_IS_NUMBER(leftType) && PDB_TYPE_IS_FLOAT_OR_DOUBLE(rightType))
    || (PDB_TYPE_IS_FLOAT_OR_DOUBLE(leftType) && PDB_TYPE_IS_NUMBER(rightType)))
  {
    switch (leftType)
    {
    case PDB_FIELD_TYPE::TYPE_INT8:
      return CreateValueCompareStep< PDB_FIELD_TYPE::TYPE_INT8, double>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_INT16:
      return CreateValueCompareStep< PDB_FIELD_TYPE::TYPE_INT16, double>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_INT32:
      return CreateValueCompareStep< PDB_FIELD_TYPE::TYPE_INT32, double>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_INT64:
      return CreateValueCompareStep< PDB_FIELD_TYPE::TYPE_INT64, double>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_FLOAT:
      return CreateValueCompareStep< PDB_FIELD_TYPE::TYPE_FLOAT, double>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_DOUBLE:
      return CreateValueCompareStep< PDB_FIELD_TYPE::TYPE_DOUBLE, double>(op, pLeft, pRight);
    }
  }
  else if (leftType == PDB_FIELD_TYPE::TYPE_BOOL && rightType == PDB_FIELD_TYPE::TYPE_BOOL)
  {
    return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_BOOL, PDB_FIELD_TYPE::TYPE_BOOL, bool>(op, pLeft, pRight);
  }
  else if (leftType == PDB_FIELD_TYPE::TYPE_DATETIME && rightType == PDB_FIELD_TYPE::TYPE_DATETIME)
  {
    return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_DATETIME, PDB_FIELD_TYPE::TYPE_DATETIME, int64_t>(op, pLeft, pRight);
  }
  else if (leftType == PDB_FIELD_TYPE::TYPE_STRING && rightType == PDB_FIELD_TYPE::TYPE_STRING)
  {
    return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_STRING, PDB_FIELD_TYPE::TYPE_STRING, std::string>(op, pLeft, pRight);
  }

  return nullptr;
}

ValueItem* CreateOperator(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMicroseconds)
{
  if (pExpr == nullptr)
    return nullptr;

  int op = pExpr->GetValueType();
  const ExprValue* pLeftExpr = pExpr->GetLeftParam();
  const ExprValue* pRightExpr = pExpr->GetRightParam();
  if (pLeftExpr == nullptr || pRightExpr == nullptr)
    return nullptr;

  PdbErr_t retVal = PdbE_OK;
  ValueItem* pLeftValue = nullptr;
  ValueItem* pRightValue = BuildGeneralValueItem(pTableInfo, pRightExpr, nowMicroseconds);
  if (pRightValue == nullptr)
    return nullptr;

  if (!pRightValue->IsValid())
  {
    delete pRightValue;
    return nullptr;
  }

  ValueItem* pResultValue = nullptr;
  DBVal oldVal;
  DateTime dt;

  if (pLeftExpr->GetValueType() == TK_ID && pRightValue->IsConstValue())
  {
    do {
      DBVal fieldVal = pLeftExpr->GetValue();
      if (!DBVAL_IS_STRING(&fieldVal))
        break;

      size_t fieldPos = 0;
      int32_t fieldType = 0;
      std::string fieldName = std::string(DBVAL_GET_STRING(&fieldVal), DBVAL_GET_LEN(&fieldVal));
      retVal = pTableInfo->GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
      if (retVal != PdbE_OK)
        break;

      DBVal rightVal;
      retVal = pRightValue->GetValue(nullptr, &rightVal);
      if (retVal != PdbE_OK)
        break;

      if (fieldType == PDB_VALUE_TYPE::VAL_DATETIME && DBVAL_IS_STRING(&rightVal))
      {
        if (dt.Parse(DBVAL_GET_STRING(&rightVal), DBVAL_GET_LEN(&rightVal)))
        {
          DBVAL_SET_DATETIME(&rightVal, dt.TotalMicrosecond());
        }
      }

      if (fieldType != DBVAL_GET_TYPE(&rightVal))
      {
        //表达式两边的类型不一样
        if (DBVAL_IS_INT64(&rightVal))
        {
          //int64 可以转成 datetime， double
          int64_t tmpI64 = DBVAL_GET_INT64(&rightVal);
          if (fieldType == PDB_FIELD_TYPE::TYPE_DATETIME)
          {
            if (tmpI64 >= DateTime::MinMicrosecond && tmpI64 < DateTime::MaxMicrosecond)
            {
              DBVAL_SET_DATETIME(&rightVal, tmpI64);
            }
          }
          else if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(fieldType))
          {
            DBVAL_SET_DOUBLE(&rightVal, static_cast<double>(tmpI64));
          }
        }

        int32_t tmpTypeLeft = fieldType;
        int32_t tmpTypeRight = DBVAL_GET_TYPE(&rightVal);

        if (PDB_TYPE_IS_NUMBER(tmpTypeLeft))
          tmpTypeLeft = PDB_FIELD_TYPE::TYPE_INT64;

        if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(tmpTypeLeft))
          tmpTypeLeft = PDB_FIELD_TYPE::TYPE_DOUBLE;

        if (PDB_TYPE_IS_NUMBER(tmpTypeRight))
          tmpTypeRight = PDB_FIELD_TYPE::TYPE_INT64;

        if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(tmpTypeRight))
          tmpTypeRight = PDB_FIELD_TYPE::TYPE_DOUBLE;

        if (tmpTypeLeft != tmpTypeRight)
        {
          break;
        }
      }

      switch (fieldType)
      {
      case PDB_FIELD_TYPE::TYPE_BOOL:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_BOOL, bool>(op, fieldPos, DBVAL_GET_BOOL(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_INT8:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_INT8, int64_t>(op, fieldPos, DBVAL_GET_INT64(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_INT16:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_INT16, int64_t>(op, fieldPos, DBVAL_GET_INT64(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_INT32:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_INT32, int64_t>(op, fieldPos, DBVAL_GET_INT64(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_INT64:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_INT64, int64_t>(op, fieldPos, DBVAL_GET_INT64(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_DATETIME:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_DATETIME, int64_t>(op, fieldPos, DBVAL_GET_DATETIME(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_FLOAT:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_FLOAT, double>(op, fieldPos, DBVAL_GET_DOUBLE(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_DOUBLE:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_DOUBLE, double>(op, fieldPos, DBVAL_GET_DOUBLE(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_STRING:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_STRING, std::string>(op, fieldPos, std::string(DBVAL_GET_STRING(&rightVal), DBVAL_GET_LEN(&rightVal)));
        break;
      }

    } while (false);

    delete pRightValue;
    return pResultValue;
  }
  else
  {
    do {
      pLeftValue = BuildGeneralValueItem(pTableInfo, pLeftExpr, nowMicroseconds);
      if (pLeftValue == nullptr)
        break;

      if (!pLeftValue->IsValid())
        break;

      //1. 尝试 string 转 datetime
      //2. 尝试 int 转 datetime 或 double
      auto convertParamFunc = [&](ValueItem* pParam1, ValueItem* pParam2)->ValueItem* {
        if (pParam1->IsConstValue()) {
          if (pParam1->GetValue(nullptr, &oldVal) != PdbE_OK)
            return nullptr;

          if (DBVAL_IS_STRING(&oldVal) && pParam2->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME)
          {
            if (dt.Parse(DBVAL_GET_STRING(&oldVal), DBVAL_GET_LEN(&oldVal)))
            {
              return new ConstValue(dt.TotalMicrosecond(), true);
            }
          }
          else if (DBVAL_IS_INT64(&oldVal))
          {
            if (pParam2->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME)
            {
              if (DBVAL_GET_INT64(&oldVal) >= DateTime::MinMicrosecond && DBVAL_GET_INT64(&oldVal) < DateTime::MaxMicrosecond)
              {
                return new ConstValue(DBVAL_GET_INT64(&oldVal), true);
              }
            }
            else if (pParam2->GetValueType() == PDB_VALUE_TYPE::VAL_DOUBLE)
            {
              return new ConstValue(static_cast<double>(DBVAL_GET_INT64(&oldVal)));
            }
          }
        }
        return nullptr;
      };

      if (pLeftValue->GetValueType() != pRightValue->GetValueType())
      {
        ValueItem* pNewValue = convertParamFunc(pLeftValue, pRightValue);
        if (pNewValue != nullptr)
        {
          delete pLeftValue;
          pLeftValue = pNewValue;
        }
      }

      if (pLeftValue->GetValueType() != pRightValue->GetValueType())
      {
        ValueItem* pNewValue = convertParamFunc(pRightValue, pLeftValue);
        if (pNewValue != nullptr)
        {
          delete pRightValue;
          pRightValue = pNewValue;
        }
      }

      pResultValue = CreateValueCompare(op, pLeftValue, pRightValue);
    } while (false);

    if (pResultValue == nullptr)
    {
      if (pLeftValue != nullptr)
        delete pLeftValue;

      if (pRightValue != nullptr)
        delete pRightValue;
    }

    return pResultValue;
  }
}

PdbErr_t CreateAndFunctionStep(const TableInfo* pTableInfo, const ExprValue* pExpr,
  int64_t nowMicroseconds, AndFunction* pAndFunc)
{
  PdbErr_t retVal = PdbE_OK;
  int opType = pExpr->GetValueType();
  if (opType == TK_AND)
  {
    const ExprValue* pLeftExpr = pExpr->GetLeftParam();
    const ExprValue* pRightExpr = pExpr->GetRightParam();
    if (pLeftExpr == nullptr || pRightExpr == nullptr)
      return PdbE_SQL_ERROR;

    retVal = CreateAndFunctionStep(pTableInfo, pLeftExpr, nowMicroseconds, pAndFunc);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = CreateAndFunctionStep(pTableInfo, pRightExpr, nowMicroseconds, pAndFunc);
    if (retVal != PdbE_OK)
      return retVal;
  }
  else
  {
    ValueItem* pValue = BuildGeneralValueItem(pTableInfo, pExpr, nowMicroseconds);
    if (pValue == nullptr)
      return PdbE_SQL_ERROR;

    pAndFunc->AddValueItem(pValue);
  }

  return PdbE_OK;
}

ValueItem* CreateAndFunction(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMicroseconds)
{
  AndFunction* pAndFunc = new AndFunction();
  if (CreateAndFunctionStep(pTableInfo, pExpr, nowMicroseconds, pAndFunc) != PdbE_OK)
  {
    delete pAndFunc;
    return nullptr;
  }

  return pAndFunc;
}

GroupField* CreateAggCount(size_t fieldPos)
{
  return new CountFunc(fieldPos);
}

template<bool IsFirst>
GroupField* CreateAggFirstOrLast(size_t fieldPos, int fieldType)
{
  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_BOOL:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_BOOL, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_INT8, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_INT16, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_INT32, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_INT64, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_DATETIME, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_FLOAT, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_DOUBLE, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_STRING:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_STRING, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_BLOB:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_BLOB, IsFirst>(fieldPos);
  }

  return nullptr;
}

GroupField* CreateAggAvg(size_t fieldPos, int fieldType)
{
  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_INT8, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_INT16, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_INT32, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_INT64, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_FLOAT, double>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_DOUBLE, double>(fieldPos);
  }

  return nullptr;
}

GroupField* CreateAggSum(size_t fieldPos, int fieldType)
{
  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_INT8, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_INT16, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_INT32, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_INT64, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_FLOAT, double>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_DOUBLE, double>(fieldPos);
  }

  return nullptr;
}

template<int CompareType, bool IsMax>
GroupField* CreateAggExtremeLeaf(int targetType, size_t comparePos, size_t targetPos)
{
  switch (targetType)
  {
  case PDB_FIELD_TYPE::TYPE_BOOL:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_BOOL, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_INT8, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_INT16, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_INT32, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_INT64, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_DATETIME, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_FLOAT, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_DOUBLE, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_STRING:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_STRING, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_BLOB:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_BLOB, IsMax>(comparePos, targetPos);
  }

  return nullptr;
}

template<bool IsMax>
GroupField* CreateAggExtreme(int compareType, int targetType, size_t comparePos, size_t targetPos)
{
  switch (compareType)
  {
  case PDB_FIELD_TYPE::TYPE_INT8:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_INT8, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_INT16, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_INT32, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_INT64, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_DATETIME, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return CreateAggExtremeLeaf< PDB_FIELD_TYPE::TYPE_FLOAT, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_DOUBLE, IsMax>(targetType, comparePos, targetPos);
  }

  return nullptr;
}


ValueItem* BuildGeneralValueItem(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMicroseconds)
{
  if (pExpr == nullptr)
    return nullptr;

  ValueItem* pNewValue = nullptr;
  DBVal exprVal = pExpr->GetValue();
  int exprType = pExpr->GetValueType();
  switch (exprType)
  {
  case TK_ID:
    pNewValue = CreateFieldValueByExpr(pTableInfo, pExpr);
    break;
  case TK_TRUE:
  case TK_FALSE:
  case TK_INTEGER:
  case TK_DOUBLE:
  case TK_STRING:
  case TK_BLOB:
  case TK_TIMEVAL:
    pNewValue = new ConstValue(exprVal);
    break;
  case TK_FUNCTION:
    pNewValue = CreateGeneralFunction(pTableInfo, pExpr, nowMicroseconds);
    break;
  case TK_LT:
  case TK_LE:
  case TK_GT:
  case TK_GE:
  case TK_EQ:
  case TK_NE:
    pNewValue = CreateOperator(pTableInfo, pExpr, nowMicroseconds);
    break;
  case TK_AND:
    pNewValue = CreateAndFunction(pTableInfo, pExpr, nowMicroseconds);
    break;
  case TK_LIKE:
    pNewValue = CreateLikeFunction(pTableInfo, pExpr);
    break;
  case TK_ISNOTNULL:
    pNewValue = CreateIsNullOrNullFunction(true, pTableInfo, pExpr);
    break;
  case TK_ISNULL:
    pNewValue = CreateIsNullOrNullFunction(false, pTableInfo, pExpr);
    break;
  case TK_IN:
    pNewValue = CreateInOrNotInFunction<false>(pTableInfo, pExpr, nowMicroseconds);
    break;
  case TK_NOTIN:
    pNewValue = CreateInOrNotInFunction<true>(pTableInfo, pExpr, nowMicroseconds);
    break;
  }

  if (pNewValue != nullptr)
  {
    if (!pNewValue->IsValid())
    {
      delete pNewValue;
      pNewValue = nullptr;
    }
  }

  return pNewValue;
}

GroupField* BuildTargetGroupStep(int funcId, const TableInfo* pTableInfo, int64_t nowMicroseconds,
  const ExprValue* pParam1, const ExprValue* pParam2)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType1 = 0;
  int32_t fieldType2 = 0;
  size_t fieldPos1 = 0;
  size_t fieldPos2 = 0;

  if (pParam1 != nullptr)
  {
    if (pParam1->GetValueType() == TK_ID)
    {
      DBVal nameVal1 = pParam1->GetValue();
      if (!DBVAL_IS_STRING(&nameVal1))
        return nullptr;

      std::string fieldName1 = std::string(DBVAL_GET_STRING(&nameVal1), DBVAL_GET_LEN(&nameVal1));
      retVal = pTableInfo->GetFieldInfo(fieldName1.c_str(), &fieldPos1, &fieldType1);
      if (retVal != PdbE_OK)
        return nullptr;
    }
  }

  if (pParam2 != nullptr)
  {
    if (pParam2->GetValueType() != TK_ID)
      return nullptr;

    DBVal nameVal2 = pParam2->GetValue();
    if (!DBVAL_IS_STRING(&nameVal2))
      return nullptr;

    std::string fieldName2 = std::string(DBVAL_GET_STRING(&nameVal2), DBVAL_GET_LEN(&nameVal2));
    retVal = pTableInfo->GetFieldInfo(fieldName2.c_str(), &fieldPos2, &fieldType2);
    if (retVal != PdbE_OK)
      return nullptr;
  }

  if (pParam1 == nullptr && pParam2 == nullptr)
  {
    if (funcId == PDB_SQL_FUNC::FUNC_AGG_COUNT)
      return new CountFunc(PDB_DEVID_INDEX);
    else
      return nullptr;
  }
  else if (pParam1 != nullptr && pParam2 == nullptr)
  {
    if (pParam1->GetValueType() == TK_STAR)
    {
      if (funcId == PDB_SQL_FUNC::FUNC_AGG_COUNT)
        return new CountFunc(PDB_DEVID_INDEX);
      else
        return nullptr;
    }

    if (pParam1->GetValueType() != TK_ID)
      return nullptr;

    switch (funcId)
    {
    case PDB_SQL_FUNC::FUNC_AGG_COUNT:
      return CreateAggCount(fieldPos1);
    case PDB_SQL_FUNC::FUNC_AGG_FIRST:
      return CreateAggFirstOrLast<true>(fieldPos1, fieldType1);
    case PDB_SQL_FUNC::FUNC_AGG_LAST:
      return CreateAggFirstOrLast<false>(fieldPos1, fieldType1);
    case PDB_SQL_FUNC::FUNC_AGG_AVG:
      return CreateAggAvg(fieldPos1, fieldType1);
    case PDB_SQL_FUNC::FUNC_AGG_MIN:
      return CreateAggExtreme<false>(fieldType1, fieldType1, fieldPos1, fieldPos1);
    case PDB_SQL_FUNC::FUNC_AGG_MAX:
      return CreateAggExtreme<true>(fieldType1, fieldType1, fieldPos1, fieldPos1);
    case PDB_SQL_FUNC::FUNC_AGG_SUM:
      return CreateAggSum(fieldPos1, fieldType1);
    }
  }
  else if (pParam1 != nullptr && pParam2 != nullptr)
  {
    if (pParam1->GetValueType() != TK_ID)
      return nullptr;

    if (funcId == PDB_SQL_FUNC::FUNC_AGG_MIN)
    {
      return CreateAggExtreme<false>(fieldType1, fieldType2, fieldPos1, fieldPos2);
    }
    else if (funcId == PDB_SQL_FUNC::FUNC_AGG_MAX)
    {
      return CreateAggExtreme<true>(fieldType1, fieldType2, fieldPos1, fieldPos2);
    }
  }

  return nullptr;
}

PdbErr_t BuildTargetGroupItem(const TableInfo* pTableInfo, const ExprValue* pExpr,
  TableInfo* pGroupInfo, std::vector<GroupField*>& fieldVec, int64_t nowMicroseconds)
{
  PdbErr_t retVal = PdbE_OK;
  char uniqueName[32];
  if (pTableInfo == nullptr || pExpr == nullptr || pGroupInfo == nullptr)
    return PdbE_INVALID_PARAM;

#ifdef _WIN32
  sprintf(uniqueName, "agg_%llu", reinterpret_cast<uintptr_t>(pExpr));
#else
  sprintf(uniqueName, "agg_%lu", reinterpret_cast<uintptr_t>(pExpr));
#endif

  if (pExpr->GetValueType() == TK_FUNCTION)
  {
    DBVal nameVal = pExpr->GetValue();
    if (!DBVAL_IS_STRING(&nameVal))
      return PdbE_SQL_ERROR;

    if (DBVAL_GET_LEN(&nameVal) < 2 || DBVAL_GET_LEN(&nameVal) > 20)
      return PdbE_SQL_ERROR;

    int functionId = GetFunctionId(DBVAL_GET_STRING(&nameVal), DBVAL_GET_LEN(&nameVal));
    if (functionId <= 0)
      return PdbE_SQL_ERROR;

    const ExprValue* pParam1 = nullptr;
    const ExprValue* pParam2 = nullptr;
    const ExprValueList* pArgList = pExpr->GetArgList();

    if (functionId == PDB_SQL_FUNC::FUNC_AGG_COUNT
      || functionId == PDB_SQL_FUNC::FUNC_AGG_FIRST
      || functionId == PDB_SQL_FUNC::FUNC_AGG_LAST
      || functionId == PDB_SQL_FUNC::FUNC_AGG_AVG
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MIN
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MAX
      || functionId == PDB_SQL_FUNC::FUNC_AGG_SUM)
    {
      if (pArgList != nullptr)
      {
        const std::vector<ExprValue*>* pArgVec = pArgList->GetValueList();
        if (pArgVec != nullptr)
        {
          if (pArgVec->size() > 2)
            return PdbE_SQL_ERROR;

          if (pArgVec->size() >= 1)
            pParam1 = pArgVec->at(0);

          if (pArgVec->size() == 2)
            pParam2 = pArgVec->at(1);
        }
      }

      GroupField* pAggFunc = BuildTargetGroupStep(functionId, pTableInfo, nowMicroseconds, pParam1, pParam2);
      if (pAggFunc == nullptr)
        return PdbE_SQL_ERROR;

      pGroupInfo->AddField(uniqueName, pAggFunc->FieldType());
      fieldVec.push_back(pAggFunc);
      return PdbE_OK;
    }
    else if (functionId == PDB_SQL_FUNC::FUNC_AGG_COUNT_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_FIRST_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_LAST_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_AVG_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MIN_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MAX_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_SUM_IF)
    {
      if (pArgList == nullptr)
        return PdbE_SQL_ERROR;

      const std::vector<ExprValue*>* pArgVec = pArgList->GetValueList();
      if (pArgVec == nullptr)
        return PdbE_SQL_ERROR;

      if (pArgVec->size() < 1 || pArgVec->size() > 3)
        return PdbE_SQL_ERROR;

      if (pArgVec->size() >= 2)
        pParam1 = pArgVec->at(1);

      if (pArgVec->size() == 3)
        pParam2 = pArgVec->at(2);

      ValueItem* pCondi = BuildGeneralValueItem(pTableInfo, pArgVec->at(0), nowMicroseconds);
      if (pCondi == nullptr)
        return PdbE_SQL_ERROR;

      int subFunc = 0;
      switch (functionId)
      {
      case PDB_SQL_FUNC::FUNC_AGG_COUNT_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_COUNT; break;
      case PDB_SQL_FUNC::FUNC_AGG_FIRST_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_FIRST; break;
      case PDB_SQL_FUNC::FUNC_AGG_LAST_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_LAST; break;
      case PDB_SQL_FUNC::FUNC_AGG_AVG_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_AVG; break;
      case PDB_SQL_FUNC::FUNC_AGG_MIN_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_MIN; break;
      case PDB_SQL_FUNC::FUNC_AGG_MAX_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_MAX; break;
      case PDB_SQL_FUNC::FUNC_AGG_SUM_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_SUM; break;
      }

      GroupField* pSubAggFunc = BuildTargetGroupStep(subFunc, pTableInfo, nowMicroseconds, pParam1, pParam2);
      if (pSubAggFunc == nullptr)
      {
        delete pCondi;
        return PdbE_SQL_ERROR;
      }

      GroupField* pAggFunc = new AggIfExtendFunc(pCondi, pSubAggFunc, true);
      pGroupInfo->AddField(uniqueName, pAggFunc->FieldType());
      fieldVec.push_back(pAggFunc);
      return PdbE_OK;
    }
  }

  const ExprValue* pLeftParam = pExpr->GetLeftParam();
  if (pLeftParam != nullptr)
  {
    retVal = BuildTargetGroupItem(pTableInfo, pLeftParam, pGroupInfo, fieldVec, nowMicroseconds);
    if (retVal != PdbE_OK)
      return retVal;
  }

  const ExprValue* pRightParam = pExpr->GetRightParam();
  if (pRightParam != nullptr)
  {
    retVal = BuildTargetGroupItem(pTableInfo, pRightParam, pGroupInfo, fieldVec, nowMicroseconds);
    if (retVal != PdbE_OK)
      return retVal;
  }

  const ExprValueList* pArgList = pExpr->GetArgList();
  if (pArgList != nullptr)
  {
    const std::vector<ExprValue*>* pValVec = pArgList->GetValueList();
    for (auto argIt = pValVec->begin(); argIt != pValVec->end(); argIt++)
    {
      retVal = BuildTargetGroupItem(pTableInfo, (*argIt), pGroupInfo, fieldVec, nowMicroseconds);
      if (retVal != PdbE_OK)
        return retVal;
    }
  }

  return PdbE_OK;
}

bool IncludeAggFunction(const ExprValue* pExpr)
{
  if (pExpr == nullptr)
    return false;

  bool haveAgg = false;

  if (pExpr->GetValueType() == TK_FUNCTION)
  {
    DBVal nameVal = pExpr->GetValue();
    if (!DBVAL_IS_STRING(&nameVal))
      return false;

    if (DBVAL_GET_LEN(&nameVal) < 2 || DBVAL_GET_LEN(&nameVal) > 20)
      return false;

    int functionId = GetFunctionId(DBVAL_GET_STRING(&nameVal), DBVAL_GET_LEN(&nameVal));
    if (functionId <= 0)
      return false;

    if (functionId == PDB_SQL_FUNC::FUNC_AGG_COUNT
      || functionId == PDB_SQL_FUNC::FUNC_AGG_FIRST
      || functionId == PDB_SQL_FUNC::FUNC_AGG_LAST
      || functionId == PDB_SQL_FUNC::FUNC_AGG_AVG
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MIN
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MAX
      || functionId == PDB_SQL_FUNC::FUNC_AGG_SUM
      || functionId == PDB_SQL_FUNC::FUNC_AGG_COUNT_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_FIRST_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_LAST_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_AVG_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MIN_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MAX_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_SUM_IF
      )
    {
      return true;
    }
  }

  const ExprValue* pLeftParam = pExpr->GetLeftParam();
  if (pLeftParam != nullptr)
  {
    haveAgg = IncludeAggFunction(pLeftParam);
    if (haveAgg)
      return true;
  }

  const ExprValue* pRightParam = pExpr->GetRightParam();
  if (pRightParam != nullptr)
  {
    haveAgg = IncludeAggFunction(pRightParam);
    if (haveAgg)
      return true;
  }

  const ExprValueList* pArgList = pExpr->GetArgList();
  if (pArgList != nullptr)
  {
    const std::vector<ExprValue*>* pArgVec = pArgList->GetValueList();
    for (auto argIt = pArgVec->begin(); argIt != pArgVec->end(); argIt++)
    {
      haveAgg = IncludeAggFunction(*argIt);
      if (haveAgg)
        return true;
    }
  }

  return false;
}


