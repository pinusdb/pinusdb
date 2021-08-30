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
#include "util/date_time.h"

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
    {
      delete pLeft_;
    }
    if (pRight_ == nullptr)
    {
      delete pRight_;
    }
  }

  PDB_VALUE_TYPE GetValueType() const override { return PDB_VALUE_TYPE::VAL_DATETIME; }

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

  PDB_VALUE_TYPE GetValueType() const override { return PDB_VALUE_TYPE::VAL_INT64; }

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

  PDB_VALUE_TYPE GetValueType() const override
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
    if PDB_CONSTEXPR(IsCeil)
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
        if PDB_CONSTEXPR(IsCeil)
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

/////////////////////////////////////////////////////////

ValueItem* CreateDateTimeAdd(ValueItem* pLeft, ValueItem* pRight)
{
  if (pLeft == nullptr || pRight == nullptr)
    return nullptr;

  if (pLeft->GetValueType() != PDB_VALUE_TYPE::VAL_DATETIME
    || pRight->GetValueType() != PDB_VALUE_TYPE::VAL_INT64)
  {
    return nullptr;
  }

  return new DateTimeAdd(pLeft, pRight);
}

ValueItem* CreateDateTimeDiff(ValueItem* pLeft, ValueItem* pRight)
{
  if (pLeft == nullptr || pRight == nullptr)
    return nullptr;

  if (pLeft->GetValueType() != PDB_VALUE_TYPE::VAL_DATETIME
    || pRight->GetValueType() != PDB_VALUE_TYPE::VAL_DATETIME)
  {
    return nullptr;
  }

  return new DateTimeDiff(pLeft, pRight);
}

ValueItem* CreateDateTimeAlign(bool isCeil, ValueItem* pValue, int64_t microseconds)
{
  if (isCeil)
  {
    return new DateTimeAlign<true>(pValue, microseconds);
  }
  else
  {
    return new DateTimeAlign<false>(pValue, microseconds);
  }
}

