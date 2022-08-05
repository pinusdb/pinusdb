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

#define _CONDI_GET_COMP_RESULT(pV, v1, v2) do { \
  if PDB_CONSTEXPR(CompOp == TK_LT) { DBVAL_SET_BOOL(pV, (v1 < v2)); } \
  else if PDB_CONSTEXPR(CompOp == TK_LE) { DBVAL_SET_BOOL(pV, (v1 <= v2)); } \
  else if PDB_CONSTEXPR(CompOp == TK_GT) { DBVAL_SET_BOOL(pV, (v1 > v2)); } \
  else if PDB_CONSTEXPR(CompOp == TK_GE) { DBVAL_SET_BOOL(pV, (v1 >= v2)); } \
  else if PDB_CONSTEXPR(CompOp == TK_EQ) { DBVAL_SET_BOOL(pV, (v1 == v2)); } \
  else if PDB_CONSTEXPR(CompOp == TK_NE) { DBVAL_SET_BOOL(pV, (v1 != v2)); } \
} while (false)

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
    {
      delete pValue_;
    }
  }

  PDB_VALUE_TYPE GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal = PdbE_OK;
    DBVal tmpVal;
    retVal = pValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if PDB_CONSTEXPR(NotNull)
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

#ifdef _DEBUG
    if (valVec.size() != resultSize)
      return PdbE_INVALID_PARAM;
#endif

    const DBVal* pVals = valVec.data();

    DBVal tmpVal;
    for (size_t idx = 0; idx < resultSize; idx++)
    {
      if PDB_CONSTEXPR(NotNull)
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
    {
      return false;
    }

    return pValue_->IsValid();
  }

  bool IsConstValue() const override
  {
    return pValue_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pValue_ != nullptr)
    {
      pValue_->GetUseFields(fieldSet);
    }
  }

private:
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
    {
      delete pValue_;
    }
  }

  PDB_VALUE_TYPE GetValueType() const override
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

#ifdef _DEBUG
    if (valVec.size() != resultSize)
      return PdbE_INVALID_PARAM;
#endif

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
    {
      pValue_->GetUseFields(fieldSet);
    }
  }

private:
  ValueItem* pValue_;
  std::string patternStr_;
};

template<bool NotIn, PDB_VALUE_TYPE ValueType>
class InFunction : public ValueItem
{
public:
  InFunction(size_t fieldPos, const std::list<int64_t>& valList)
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

  PDB_VALUE_TYPE GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pResult == nullptr)
      return PdbE_OK;

    int64_t findVal = 0;
    if (DBVAL_ELE_GET_TYPE(pVals, fieldPos_) != ValueType)
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if PDB_CONSTEXPR(PDB_TYPE_IS_NUMBER(ValueType))
    {
      _GET_INT64_BY_DATATYPE_ARRAY(pVals, fieldPos_, findVal, ValueType);
    }
    else if PDB_CONSTEXPR(ValueType == PDB_VALUE_TYPE::VAL_STRING)
    {
      findVal = static_cast<int64_t>(StringTool::CRC64(DBVAL_ELE_GET_STRING(pVals, fieldPos_), DBVAL_ELE_GET_LEN(pVals, fieldPos_)));
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
      if (pFilter != nullptr)
      {
        if (pFilter[idx] == 0)
        {
          continue;
        }
      }

      if (DBVAL_ELE_GET_TYPE(pFieldVals, idx) != ValueType)
      {
        DBVAL_SET_BOOL(&tmpVal, false);
      }
      else
      {
        if PDB_CONSTEXPR(PDB_TYPE_IS_NUMBER(ValueType))
        {
          _GET_INT64_BY_DATATYPE_ARRAY(pFieldVals, idx, findVal, ValueType);
        }
        else if PDB_CONSTEXPR(ValueType == PDB_VALUE_TYPE::VAL_STRING)
        {
          findVal = static_cast<int64_t>(StringTool::CRC64(DBVAL_ELE_GET_STRING(pFieldVals, idx), DBVAL_ELE_GET_LEN(pFieldVals, idx)));
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

    return PDB_TYPE_IS_NUMBER(ValueType) || ValueType == PDB_VALUE_TYPE::VAL_STRING;
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
    return fieldPos_ == PDB_DEVID_INDEX && ValueType == PDB_VALUE_TYPE::VAL_INT64;
  }

  bool GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const override
  {
    int64_t minId = 0;
    int64_t maxId = INT64_MAX;
    if (!NotIn && ValueType == PDB_VALUE_TYPE::VAL_INT64 && fieldPos_ == PDB_DEVID_INDEX)
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
  size_t fieldPos_;
  std::unordered_set<int64_t> valSet_;
};

template<int CompOp, PDB_VALUE_TYPE CommonType, PDB_VALUE_TYPE LeftType, PDB_VALUE_TYPE RightType>
class CompareFunction : public ValueItem
{
public:
  CompareFunction(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~CompareFunction()
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

  PDB_VALUE_TYPE GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal leftValue, rightValue;

    retVal = pLeft_->GetValue(pVals, &leftValue);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

    retVal = pRight_->GetValue(pVals, &rightValue);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

    if (pResult == nullptr)
    {
      return PdbE_OK;
    }

    if (DBVAL_GET_TYPE(&leftValue) != LeftType || DBVAL_GET_TYPE(&rightValue) != RightType)
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if PDB_CONSTEXPR(CommonType == PDB_VALUE_TYPE::VAL_BOOL)
    {
      if PDB_CONSTEXPR(CompOp == TK_EQ)
      {
        DBVAL_SET_BOOL(pResult, (DBVAL_GET_BOOL(&leftValue) == DBVAL_GET_BOOL(&rightValue)));
      }
      else if PDB_CONSTEXPR(CompOp == TK_NE)
      {
        DBVAL_SET_BOOL(pResult, (DBVAL_GET_BOOL(&leftValue) != DBVAL_GET_BOOL(&rightValue)));
      }
    }
    else if PDB_CONSTEXPR(CommonType == PDB_VALUE_TYPE::VAL_INT64
      || CommonType == PDB_VALUE_TYPE::VAL_DATETIME)
    {
      int64_t i64V1 = 0;
      int64_t i64V2 = 0;
      _GET_INT64_BY_DATATYPE_SINGLE(&leftValue, i64V1, LeftType);
      _GET_INT64_BY_DATATYPE_SINGLE(&rightValue, i64V2, RightType);
      _CONDI_GET_COMP_RESULT(pResult, i64V1, i64V2);
    }
    else if PDB_CONSTEXPR(CommonType == PDB_VALUE_TYPE::VAL_DOUBLE)
    {
      double dV1 = 0;
      double dV2 = 0;
      _GET_DOUBLE_BY_DATATYPE_SINGLE(&leftValue, dV1, LeftType);
      _GET_DOUBLE_BY_DATATYPE_SINGLE(&rightValue, dV2, RightType);
      _CONDI_GET_COMP_RESULT(pResult, dV1, dV2);
    }
    else if PDB_CONSTEXPR(CommonType == PDB_VALUE_TYPE::VAL_STRING)
    {
      DBVAL_SET_BOOL(pResult, (CompOp == TK_NE));
      if (DBVAL_GET_LEN(&leftValue) == DBVAL_GET_LEN(&rightValue))
      {
        if (strncmp(DBVAL_GET_STRING(&leftValue), DBVAL_GET_STRING(&rightValue), DBVAL_GET_LEN(&leftValue)) == 0)
        {
          DBVAL_SET_BOOL(pResult, (CompOp == TK_EQ));
        }
      }
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

    retVal = pRight_->GetValueArray(blockValues, rightValVec);
    if (retVal != PdbE_OK)
      return retVal;

#ifdef _DEBUG
    if (leftValVec.size() != resultSize || rightValVec.size() != resultSize)
      return PdbE_INVALID_PARAM;
#endif

    int64_t i64V1 = 0;
    int64_t i64V2 = 0;
    double dV1 = 0;
    double dV2 = 0;

    const DBVal* pLeftVals = leftValVec.data();
    const DBVal* pRightVals = rightValVec.data();
    DBVal tmpVal;
    for (size_t idx = 0; idx < resultSize; idx++)
    {
      if (DBVAL_ELE_GET_TYPE(pLeftVals, idx) != LeftType || DBVAL_ELE_GET_TYPE(pRightVals, idx) != RightType)
      {
        DBVAL_SET_BOOL(&tmpVal, false);
      }
      else
      {
        if PDB_CONSTEXPR(CommonType == PDB_VALUE_TYPE::VAL_BOOL)
        {
          if PDB_CONSTEXPR(CompOp == TK_EQ)
          {
            DBVAL_SET_BOOL(&tmpVal, (DBVAL_ELE_GET_BOOL(pLeftVals, idx) == DBVAL_ELE_GET_BOOL(pRightVals, idx)));
          }
          else if PDB_CONSTEXPR(CompOp == TK_NE)
          {
            DBVAL_SET_BOOL(&tmpVal, (DBVAL_ELE_GET_BOOL(pLeftVals, idx) != DBVAL_ELE_GET_BOOL(pRightVals, idx)));
          }
        }
        else if PDB_CONSTEXPR(CommonType == PDB_VALUE_TYPE::VAL_INT64
          || CommonType == PDB_VALUE_TYPE::VAL_DATETIME)
        {
          _GET_INT64_BY_DATATYPE_ARRAY(pLeftVals, idx, i64V1, LeftType);
          _GET_INT64_BY_DATATYPE_ARRAY(pRightVals, idx, i64V2, RightType);
          _CONDI_GET_COMP_RESULT(&tmpVal, i64V1, i64V2);
        }
        else if PDB_CONSTEXPR(CommonType == PDB_VALUE_TYPE::VAL_DOUBLE)
        {
          _GET_DOUBLE_BY_DATATYPE_ARRAY(pLeftVals, idx, dV1, LeftType);
          _GET_DOUBLE_BY_DATATYPE_ARRAY(pRightVals, idx, dV2, RightType);
          _CONDI_GET_COMP_RESULT(&tmpVal, dV1, dV2);
        }
        else if PDB_CONSTEXPR(CommonType == PDB_VALUE_TYPE::VAL_STRING)
        {
          DBVAL_SET_BOOL(&tmpVal, (CompOp == TK_NE));
          if (DBVAL_ELE_GET_LEN(pLeftVals, idx) == DBVAL_ELE_GET_LEN(pRightVals, idx))
          {
            if (strncmp(DBVAL_ELE_GET_STRING(pLeftVals, idx), DBVAL_ELE_GET_STRING(pRightVals, idx), DBVAL_ELE_GET_LEN(pLeftVals, idx)) == 0)
            {
              DBVAL_SET_BOOL(&tmpVal, (CompOp == TK_EQ));
            }
          }
        }
      }
      resultVec.push_back(tmpVal);
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pLeft_->GetValueType() != LeftType)
    {
      return false;
    }

    if (pRight_->GetValueType() != RightType)
    {
      return false;
    }

    if (CompOp != TK_LT && CompOp != TK_LE
      && CompOp != TK_GT && CompOp != TK_GE
      && CompOp != TK_EQ && CompOp != TK_NE)
    {
      return false;
    }

    if PDB_CONSTEXPR(CommonType == PDB_VALUE_TYPE::VAL_BOOL
     || CommonType == PDB_VALUE_TYPE::VAL_STRING)
    {
      return CompOp == TK_EQ || CompOp == TK_NE;
    }

    if (CommonType != PDB_VALUE_TYPE::VAL_INT64
      && CommonType != PDB_VALUE_TYPE::VAL_DOUBLE
      && CommonType != PDB_VALUE_TYPE::VAL_DATETIME)
    {
      return false;
    }

    return true;
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pLeft_ != nullptr)
    {
      pLeft_->GetUseFields(fieldSet);
    }

    if (pRight_ != nullptr)
    {
      pRight_->GetUseFields(fieldSet);
    }
  }

private:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

template<int CompOp>
class StringFieldCompareFunction : public ValueItem
{
public:
  StringFieldCompareFunction(size_t fieldPos, const std::string& val)
  {
    this->fieldPos_ = fieldPos;
    this->val_ = val;
  }

  virtual ~StringFieldCompareFunction()
  {}

  PDB_VALUE_TYPE GetValueType() const override { return PDB_VALUE_TYPE::VAL_BOOL; }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pResult == nullptr)
    {
      return PdbE_OK;
    }

    if (!DBVAL_ELE_IS_STRING(pVals, fieldPos_))
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if PDB_CONSTEXPR(CompOp == TK_EQ)
    {
      if ((size_t)DBVAL_ELE_GET_LEN(pVals, fieldPos_) == val_.size())
      {
        DBVAL_SET_BOOL(pResult, (strncmp(DBVAL_ELE_GET_STRING(pVals, fieldPos_), val_.c_str(), val_.size()) == 0));
      }
      else
      {
        DBVAL_SET_BOOL(pResult, false);
      }
    }
    else if PDB_CONSTEXPR(CompOp == TK_NE)
    {
      if ((size_t)DBVAL_ELE_GET_LEN(pVals, fieldPos_) != val_.size())
      {
        DBVAL_SET_BOOL(pResult, true);
      }
      else
      {
        DBVAL_SET_BOOL(pResult, (strncmp(DBVAL_ELE_GET_STRING(pVals, fieldPos_), val_.c_str(), val_.size()) != 0));
      }
    }

    return PdbE_OK;
  }


  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t recordSize = blockValues.GetRecordSize();
    size_t resultSize = blockValues.GetResultSize();
    const uint8_t* pFilter = blockValues.GetFilter();
    const DBVal* pVals = blockValues.GetColumnValues(fieldPos_);

    if (recordSize == resultSize)
      pFilter = nullptr;

    DBVal tmpVal;
    if (pVals == nullptr)
    {
      DBVAL_SET_BOOL(&tmpVal, false);
      for (size_t idx = 0; idx < resultSize; idx++)
      {
        resultVec.push_back(tmpVal);
      }
    }
    else
    {
      for (size_t idx = 0; idx < recordSize; idx++)
      {
        if (pFilter != nullptr && pFilter[idx] != PDB_BOOL_TRUE)
        {
          continue;
        }

        if (DBVAL_ELE_GET_TYPE(pVals, idx) != PDB_VALUE_TYPE::VAL_STRING)
        {
          DBVAL_SET_BOOL(&tmpVal, false);
        }
        else if PDB_CONSTEXPR(CompOp == TK_EQ)
        {
          DBVAL_SET_BOOL(&tmpVal, false);
          if (DBVAL_ELE_GET_LEN(pVals, idx) == val_.size())
          {
            DBVAL_SET_BOOL(&tmpVal, (strncmp(DBVAL_ELE_GET_STRING(pVals, idx), val_.c_str(), val_.size()) == 0));
          }
        }
        else if PDB_CONSTEXPR(CompOp == TK_NE)
        {
          DBVAL_SET_BOOL(&tmpVal, true);
          if (DBVAL_ELE_GET_LEN(pVals, idx) == val_.size())
          {
            DBVAL_SET_BOOL(&tmpVal, (strncmp(DBVAL_ELE_GET_STRING(pVals, idx), val_.c_str(), val_.size()) != 0));
          }
        }
      }
    }

    return retVal;
  }

  bool IsValid() const override
  {
    return CompOp == TK_EQ || CompOp == TK_NE;
  }

  bool IsConstValue() const override { return false; }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(fieldPos_);
  }

private:
  size_t fieldPos_;
  std::string val_;
};

template<int CompOp>
class BoolFieldCompareFunction : public ValueItem
{
public:
  BoolFieldCompareFunction(size_t fieldPos, bool val)
  {
    fieldPos_ = fieldPos;
    val_ = val;
  }

  virtual ~BoolFieldCompareFunction() {}

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pResult == nullptr)
      return PdbE_OK;

    if (!DBVAL_ELE_IS_BOOL(pVals, fieldPos_))
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if PDB_CONSTEXPR(CompOp == TK_EQ)
    {
      DBVAL_SET_BOOL(pResult, (DBVAL_ELE_GET_BOOL(pVals, fieldPos_) == val_));
    }
    else if PDB_CONSTEXPR(CompOp == TK_NE)
    {
      DBVAL_SET_BOOL(pResult, (DBVAL_ELE_GET_BOOL(pVals, fieldPos_) != val_));
    }

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    size_t recordSize = blockValues.GetRecordSize();
    size_t resultSize = blockValues.GetResultSize();
    const uint8_t* pFilter = blockValues.GetFilter();
    const DBVal* pFieldVals = blockValues.GetColumnValues(fieldPos_);

    DBVal tmpVal;
    if (recordSize == resultSize)
    {
      pFilter = nullptr;
    }

    if (pFieldVals == nullptr)
    {
      DBVAL_SET_BOOL(&tmpVal, false);
      for (size_t idx = 0; idx < resultSize; idx++)
      {
        resultVec.push_back(tmpVal);
      }
    }
    else
    {
      for (size_t idx = 0; idx < recordSize; idx++)
      {
        if (pFilter != nullptr)
        {
          if (pFilter[idx] != PDB_BOOL_TRUE)
          {
            continue;
          }
        }

        if (DBVAL_ELE_IS_BOOL(pFieldVals, idx))
        {
          DBVAL_SET_BOOL(&tmpVal, false);
        }
        else
        {
          if PDB_CONSTEXPR(CompOp == TK_EQ)
          {
            DBVAL_SET_BOOL(&tmpVal, (DBVAL_ELE_GET_BOOL(pFieldVals, idx) == val_));
          }
          else if PDB_CONSTEXPR(CompOp == TK_NE)
          {
            DBVAL_SET_BOOL(&tmpVal, (DBVAL_ELE_GET_BOOL(pFieldVals, idx) != val_));
          }
        }
        resultVec.push_back(tmpVal);
      }
    }

    return PdbE_OK;
  }

  PDB_VALUE_TYPE GetValueType() const override { return PDB_VALUE_TYPE::VAL_BOOL; }

  bool IsValid() const override
  {
    return CompOp == TK_EQ || CompOp == TK_NE;
  }

  bool IsConstValue() const override { return false; }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(fieldPos_);
  }

private:
  bool val_;
  size_t fieldPos_;
};

template<int CompOp, PDB_VALUE_TYPE DataType, typename T>
class NumFieldCompareFunction : public ValueItem
{
public:
  NumFieldCompareFunction(size_t fieldPos, T val)
  {
    this->fieldPos_ = fieldPos;
    this->val_ = val;
  }

  virtual ~NumFieldCompareFunction() {}

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pResult == nullptr)
      return PdbE_OK;

    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, DataType))
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if PDB_CONSTEXPR(PDB_TYPE_IS_NUMBER((PDB_FIELD_TYPE)DataType))
    {
      int64_t fieldValue = 0;
      _GET_INT64_BY_DATATYPE_ARRAY(pVals, fieldPos_, fieldValue, DataType);
      _CONDI_GET_COMP_RESULT(pResult, fieldValue, val_);
    }
    else if PDB_CONSTEXPR(PDB_TYPE_IS_FLOAT_OR_DOUBLE((PDB_FIELD_TYPE)DataType))
    {
      double fieldValue = 0;
      _GET_DOUBLE_BY_DATATYPE_ARRAY(pVals, fieldPos_, fieldValue, DataType);
      _CONDI_GET_COMP_RESULT(pResult, fieldValue, val_);
    }

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    size_t recordSize = blockValues.GetRecordSize();
    size_t resultSize = blockValues.GetResultSize();
    const uint8_t* pFilter = blockValues.GetFilter();
    const DBVal* pFieldVals = blockValues.GetColumnValues(fieldPos_);

    DBVal tmpVal;
    if (recordSize == resultSize)
    {
      pFilter = nullptr;
    }

    if (pFieldVals == nullptr)
    {
      DBVAL_SET_BOOL(&tmpVal, false);
      for (size_t idx = 0; idx < resultSize; idx++)
      {
        resultVec.push_back(tmpVal);
      }
    }
    else
    {
      for (size_t idx = 0; idx < recordSize; idx++)
      {
        if (pFilter != nullptr)
        {
          if (pFilter[idx] != PDB_BOOL_TRUE)
          {
            continue;
          }
        }

        if (DBVAL_ELE_GET_TYPE(pFieldVals, idx) != DataType)
        {
          DBVAL_SET_BOOL(&tmpVal, false);
        }
        else
        {
          if PDB_CONSTEXPR(PDB_TYPE_IS_NUMBER((PDB_FIELD_TYPE)DataType))
          {
            int64_t fieldValue = 0;
            _GET_INT64_BY_DATATYPE_ARRAY(pFieldVals, idx, fieldValue, DataType);
            _CONDI_GET_COMP_RESULT(&tmpVal, fieldValue, val_);
          }
          else if PDB_CONSTEXPR(PDB_TYPE_IS_FLOAT_OR_DOUBLE((PDB_FIELD_TYPE)DataType))
          {
            double fieldValue = 0;
            _GET_DOUBLE_BY_DATATYPE_ARRAY(pFieldVals, idx, fieldValue, DataType);
            _CONDI_GET_COMP_RESULT(&tmpVal, fieldValue, val_);
          }
        }
        resultVec.push_back(tmpVal);
      }
    }

    return PdbE_OK;
  }

  PDB_VALUE_TYPE GetValueType() const override { return PDB_VALUE_TYPE::VAL_BOOL; }

  bool IsValid() const override
  {
    if (PDB_TYPE_IS_NUMBER(DataType) || PDB_TYPE_IS_FLOAT_OR_DOUBLE(DataType))
    {
      return (CompOp == TK_LT || CompOp == TK_LE
        || CompOp == TK_GT || CompOp == TK_GE
        || CompOp == TK_EQ || CompOp == TK_NE);
    }

    return false;
  }

  bool IsConstValue() const override { return false; }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(fieldPos_);
  }

protected:
  size_t fieldPos_;
  T val_;
};

template<int CompOp>
class DevIdCompareFunction : public ValueItem
{
public:
  DevIdCompareFunction(int64_t val)
  {
    val_ = val;
  }

  virtual ~DevIdCompareFunction() {}

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pResult == nullptr)
      return PdbE_OK;

    if (!DBVAL_ELE_IS_INT64(pVals, PDB_DEVID_INDEX))
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    _CONDI_GET_COMP_RESULT(pResult, DBVAL_ELE_GET_INT64(pVals, PDB_DEVID_INDEX), val_);
    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    size_t recordSize = blockValues.GetRecordSize();
    size_t resultSize = blockValues.GetResultSize();
    const uint8_t* pFilter = blockValues.GetFilter();
    const DBVal* pFieldVals = blockValues.GetColumnValues(PDB_DEVID_INDEX);

    DBVal tmpVal;
    if (recordSize == resultSize)
    {
      pFilter = nullptr;
    }

    for (size_t idx = 0; idx < recordSize; idx++)
    {
      if (pFilter != nullptr)
      {
        if (pFilter[idx] != PDB_BOOL_TRUE)
        {
          continue;
        }
      }

      if (DBVAL_ELE_IS_INT64(pFieldVals, idx))
      {
        _CONDI_GET_COMP_RESULT(&tmpVal, DBVAL_ELE_GET_INT64(pFieldVals, idx), val_);
      }
      else
      {
        DBVAL_SET_BOOL(&tmpVal, false);
      }

      resultVec.push_back(tmpVal);
    }

    return PdbE_OK;
  }

  PDB_VALUE_TYPE GetValueType() const override { return PDB_VALUE_TYPE::VAL_BOOL; }

  bool IsValid() const override { return true; }

  bool IsConstValue() const override { return false; }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(PDB_DEVID_INDEX);
  }

  bool IsDevIdCondition() const override { return true; }

  bool GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const override
  {
    int64_t minId = 0;
    int64_t maxId = INT64_MAX;

    if PDB_CONSTEXPR(CompOp == TK_LT || CompOp == TK_LE)
    {
      maxId = val_;
    }
    else if PDB_CONSTEXPR(CompOp == TK_GT || CompOp == TK_GE)
    {
      minId = val_;
    }
    else if PDB_CONSTEXPR(CompOp == TK_EQ)
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

private:
  int64_t val_;
};

template<int CompOp>
class TsCompareFunction : public ValueItem
{
public:
  TsCompareFunction(int64_t val)
  {
    val_ = val;
  }

  virtual ~TsCompareFunction() {}

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pResult == nullptr)
      return PdbE_OK;

    if (!DBVAL_ELE_IS_INT64(pVals, PDB_TSTAMP_INDEX))
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    _CONDI_GET_COMP_RESULT(pResult, DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX), val_);
    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    size_t recordSize = blockValues.GetRecordSize();
    size_t resultSize = blockValues.GetResultSize();
    const uint8_t* pFilter = blockValues.GetFilter();
    const DBVal* pFieldVals = blockValues.GetColumnValues(PDB_TSTAMP_INDEX);

    DBVal tmpVal;
    if (recordSize == resultSize)
    {
      pFilter = nullptr;
    }

    for (size_t idx = 0; idx < recordSize; idx++)
    {
      if (pFilter != nullptr)
      {
        if (pFilter[idx] != PDB_BOOL_TRUE)
        {
          continue;
        }
      }

      if (DBVAL_ELE_IS_INT64(pFieldVals, idx))
      {
        DBVAL_SET_BOOL(&tmpVal, false);
      }
      else
      {
        _CONDI_GET_COMP_RESULT(&tmpVal, DBVAL_ELE_GET_INT64(pFieldVals, idx), val_);
      }

      resultVec.push_back(tmpVal);
    }

    return PdbE_OK;
  }

  PDB_VALUE_TYPE GetValueType() const override { return PDB_VALUE_TYPE::VAL_BOOL; }

  bool IsValid() const override { return true; }

  bool IsConstValue() const override { return false; }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(PDB_TSTAMP_INDEX);
  }

  bool IsTstampCondition() const override { return true; }

  bool GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const override
  {
    int64_t minTs = 0;
    int64_t maxTs = INT64_MAX;
    if PDB_CONSTEXPR(CompOp == TK_LT || CompOp == TK_LE)
    {
      maxTs = val_;
    }
    else if PDB_CONSTEXPR(CompOp == TK_GT || CompOp == TK_GE)
    {
      minTs = val_;
    }
    else if PDB_CONSTEXPR(CompOp == TK_EQ)
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

private:
  int64_t val_;
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

  PDB_VALUE_TYPE GetValueType() const override
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

////////////////////////////////////////////////////////////////////////////////

ValueItem* CreateNullFunction(bool notNull, ValueItem* pValue)
{
  if (notNull)
  {
    return new NullFunction<true>(pValue);
  }
  else
  {
    return new NullFunction<false>(pValue);
  }
}

////////////////////////////////////////////////////////////////////////////////

template<int CompOp, PDB_VALUE_TYPE CommonType, PDB_VALUE_TYPE LeftType>
ValueItem* _CreateCompareFunction3(ValueItem* pLeft, ValueItem* pRight)
{
  PDB_VALUE_TYPE rightType = pRight->GetValueType();
  switch (rightType)
  {
  case PDB_VALUE_TYPE::VAL_BOOL:
    return new CompareFunction<CompOp, CommonType, LeftType, PDB_VALUE_TYPE::VAL_BOOL>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT8:
    return new CompareFunction<CompOp, CommonType, LeftType, PDB_VALUE_TYPE::VAL_INT8>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT16:
    return new CompareFunction<CompOp, CommonType, LeftType, PDB_VALUE_TYPE::VAL_INT16>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT32:
    return new CompareFunction<CompOp, CommonType, LeftType, PDB_VALUE_TYPE::VAL_INT32>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT64:
    return new CompareFunction<CompOp, CommonType, LeftType, PDB_VALUE_TYPE::VAL_INT64>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return new CompareFunction<CompOp, CommonType, LeftType, PDB_VALUE_TYPE::VAL_DATETIME>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_FLOAT:
    return new CompareFunction<CompOp, CommonType, LeftType, PDB_VALUE_TYPE::VAL_FLOAT>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return new CompareFunction<CompOp, CommonType, LeftType, PDB_VALUE_TYPE::VAL_DOUBLE>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_STRING:
    return new CompareFunction<CompOp, CommonType, LeftType, PDB_VALUE_TYPE::VAL_STRING>(pLeft, pRight);
  }

  return nullptr;
}

template<int CompOp, PDB_VALUE_TYPE CommonType>
ValueItem* _CreateCompareFunction2(ValueItem* pLeft, ValueItem* pRight)
{
  PDB_VALUE_TYPE leftType = pLeft->GetValueType();
  switch (leftType)
  {
  case PDB_VALUE_TYPE::VAL_BOOL:
    return _CreateCompareFunction3<CompOp, CommonType, PDB_VALUE_TYPE::VAL_BOOL>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT8:
    return _CreateCompareFunction3<CompOp, CommonType, PDB_VALUE_TYPE::VAL_INT8>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT16:
    return _CreateCompareFunction3<CompOp, CommonType, PDB_VALUE_TYPE::VAL_INT16>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT32:
    return _CreateCompareFunction3<CompOp, CommonType, PDB_VALUE_TYPE::VAL_INT32>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT64:
    return _CreateCompareFunction3<CompOp, CommonType, PDB_VALUE_TYPE::VAL_INT64>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return _CreateCompareFunction3<CompOp, CommonType, PDB_VALUE_TYPE::VAL_DATETIME>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_FLOAT:
    return _CreateCompareFunction3<CompOp, CommonType, PDB_VALUE_TYPE::VAL_FLOAT>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return _CreateCompareFunction3<CompOp, CommonType, PDB_VALUE_TYPE::VAL_DOUBLE>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_STRING:
    return _CreateCompareFunction3<CompOp, CommonType, PDB_VALUE_TYPE::VAL_STRING>(pLeft, pRight);
  }

  return nullptr;
}

template<int CompOp>
ValueItem* _CreateCompareFunction1(ValueItem* pLeft, ValueItem* pRight)
{
  PDB_VALUE_TYPE leftType = pLeft->GetValueType();
  PDB_VALUE_TYPE rightType = pRight->GetValueType();
  PDB_VALUE_TYPE commonType = GetCommonType(leftType, rightType);
  
  switch (commonType)
  {
  case PDB_VALUE_TYPE::VAL_BOOL:
    return _CreateCompareFunction2<CompOp, PDB_VALUE_TYPE::VAL_BOOL>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT64:
    return _CreateCompareFunction2<CompOp, PDB_VALUE_TYPE::VAL_INT64>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return _CreateCompareFunction2<CompOp, PDB_VALUE_TYPE::VAL_DOUBLE>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return _CreateCompareFunction2<CompOp, PDB_VALUE_TYPE::VAL_DATETIME>(pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_STRING:
    return _CreateCompareFunction2<CompOp, PDB_VALUE_TYPE::VAL_STRING>(pLeft, pRight);
  }

  return nullptr;
}

ValueItem* CreateCompareFunction(int compOp, ValueItem* pLeft, ValueItem* pRight)
{
  switch (compOp)
  {
  case TK_LT: return _CreateCompareFunction1<TK_LT>(pLeft, pRight);
  case TK_LE: return _CreateCompareFunction1<TK_LE>(pLeft, pRight);
  case TK_GT: return _CreateCompareFunction1<TK_GT>(pLeft, pRight);
  case TK_GE: return _CreateCompareFunction1<TK_GE>(pLeft, pRight);
  case TK_EQ: return _CreateCompareFunction1<TK_EQ>(pLeft, pRight);
  case TK_NE: return _CreateCompareFunction1<TK_NE>(pLeft, pRight);
  }
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

ValueItem* CreateStringFieldCompareFunction(int compOp, size_t fieldPos, const std::string& val)
{
  if (compOp == TK_EQ)
  {
    return new StringFieldCompareFunction<TK_EQ>(fieldPos, val);
  }
  else if (compOp == TK_NE)
  {
    return new StringFieldCompareFunction<TK_NE>(fieldPos, val);
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

template<int CompOp>
ValueItem* _CreateNumFieldCompareFunction1(size_t fieldPos, PDB_VALUE_TYPE fieldType, DBVal val)
{
  if (PDB_TYPE_IS_NUMBER(fieldType) || fieldType == PDB_VALUE_TYPE::VAL_DATETIME)
  {
    int64_t ival = 0;
    switch (DBVAL_GET_TYPE(&val))
    {
    case PDB_VALUE_TYPE::VAL_INT8: ival = DBVAL_GET_INT8(&val); break;
    case PDB_VALUE_TYPE::VAL_INT16: ival = DBVAL_GET_INT16(&val); break;
    case PDB_VALUE_TYPE::VAL_INT32: ival = DBVAL_GET_INT32(&val); break;
    case PDB_VALUE_TYPE::VAL_INT64: ival = DBVAL_GET_INT64(&val); break;
    case PDB_VALUE_TYPE::VAL_DATETIME: ival = DBVAL_GET_DATETIME(&val); break;
    default: return nullptr;
    }

    switch (fieldType)
    {
    case PDB_VALUE_TYPE::VAL_INT8: 
      return new NumFieldCompareFunction<CompOp, PDB_VALUE_TYPE::VAL_INT8, int64_t>(fieldPos, ival);
    case PDB_VALUE_TYPE::VAL_INT16:
      return new NumFieldCompareFunction<CompOp, PDB_VALUE_TYPE::VAL_INT16, int64_t>(fieldPos, ival);
    case PDB_VALUE_TYPE::VAL_INT32:
      return new NumFieldCompareFunction<CompOp, PDB_VALUE_TYPE::VAL_INT32, int64_t>(fieldPos, ival);
    case PDB_VALUE_TYPE::VAL_INT64:
      return new NumFieldCompareFunction<CompOp, PDB_VALUE_TYPE::VAL_INT64, int64_t>(fieldPos, ival);
    case PDB_VALUE_TYPE::VAL_DATETIME:
      return new NumFieldCompareFunction<CompOp, PDB_VALUE_TYPE::VAL_DATETIME, int64_t>(fieldPos, ival);
    }
  }
  else if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(fieldType))
  {
    double dval = 0;
    switch (DBVAL_GET_TYPE(&val))
    {
    case PDB_VALUE_TYPE::VAL_INT8: dval = static_cast<double>(DBVAL_GET_INT8(&val)); break;
    case PDB_VALUE_TYPE::VAL_INT16: dval = static_cast<double>(DBVAL_GET_INT16(&val)); break;
    case PDB_VALUE_TYPE::VAL_INT32: dval = static_cast<double>(DBVAL_GET_INT32(&val)); break;
    case PDB_VALUE_TYPE::VAL_INT64: dval = static_cast<double>(DBVAL_GET_INT64(&val)); break;
    case PDB_VALUE_TYPE::VAL_DATETIME: dval = static_cast<double>(DBVAL_GET_DATETIME(&val)); break;
    case PDB_VALUE_TYPE::VAL_FLOAT: dval = static_cast<double>(DBVAL_GET_FLOAT(&val)); break;
    case PDB_VALUE_TYPE::VAL_DOUBLE: dval = DBVAL_GET_DOUBLE(&val); break;
    default: return nullptr;
    }

    if (PDB_VALUE_TYPE::VAL_FLOAT == fieldType)
    {
      return new NumFieldCompareFunction<CompOp, PDB_VALUE_TYPE::VAL_FLOAT, double>(fieldPos, dval);
    }
    else if (PDB_VALUE_TYPE::VAL_DOUBLE == fieldType)
    {
      return new NumFieldCompareFunction<CompOp, PDB_VALUE_TYPE::VAL_DOUBLE, double>(fieldPos, dval);
    }
  }
  else if (fieldType == PDB_VALUE_TYPE::VAL_BOOL)
  {
    if (DBVAL_IS_BOOL(&val))
    {
      return new BoolFieldCompareFunction<CompOp>(fieldPos, DBVAL_GET_BOOL(&val));
    }
  }

  return nullptr;
}

ValueItem* CreateNumFieldCompareFunction(int compOp, size_t fieldPos, PDB_VALUE_TYPE fieldType, DBVal val)
{
  switch (compOp)
  {
  case TK_LT: return _CreateNumFieldCompareFunction1<TK_LT>(fieldPos, fieldType, val);
  case TK_LE: return _CreateNumFieldCompareFunction1<TK_LE>(fieldPos, fieldType, val);
  case TK_GT: return _CreateNumFieldCompareFunction1<TK_GT>(fieldPos, fieldType, val);
  case TK_GE: return _CreateNumFieldCompareFunction1<TK_GE>(fieldPos, fieldType, val);
  case TK_EQ: return _CreateNumFieldCompareFunction1<TK_EQ>(fieldPos, fieldType, val);
  case TK_NE: return _CreateNumFieldCompareFunction1<TK_NE>(fieldPos, fieldType, val);
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

ValueItem* CreateDevIdCompareFunction(int compOp, int64_t val)
{
  switch (compOp)
  {
  case TK_LT: return new DevIdCompareFunction<TK_LT>(val);
  case TK_LE: return new DevIdCompareFunction<TK_LE>(val);
  case TK_GT: return new DevIdCompareFunction<TK_GT>(val);
  case TK_GE: return new DevIdCompareFunction<TK_GE>(val);
  case TK_EQ: return new DevIdCompareFunction<TK_EQ>(val);
  case TK_NE: return new DevIdCompareFunction<TK_NE>(val);
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

ValueItem* CreateTsCompareFunction(int compOp, int64_t val)
{
  switch (compOp)
  {
  case TK_LT: return new TsCompareFunction<TK_LT>(val);
  case TK_LE: return new TsCompareFunction<TK_LE>(val);
  case TK_GT: return new TsCompareFunction<TK_GT>(val);
  case TK_GE: return new TsCompareFunction<TK_GE>(val);
  case TK_EQ: return new TsCompareFunction<TK_EQ>(val);
  case TK_NE: return new TsCompareFunction<TK_NE>(val);
  }

  return nullptr;
}




