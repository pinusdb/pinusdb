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
#include "query/group_field.h"
#include "query/value_item.h"

template<int ValType, typename T>
class AvgFunc : public GroupField
{
public:
  AvgFunc(size_t fieldPos)
  {
    this->fieldPos_ = fieldPos;
    this->totalVal_ = 0;
    this->dataCnt_ = 0;
  }

  virtual ~AvgFunc() { }

  int32_t FieldType() override { return ValType; }

  PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt) override
  {
    if (DBVAL_ELE_IS_NULL(pVals, fieldPos_))
      return PdbE_OK;

    if (DBVAL_ELE_GET_TYPE(pVals, fieldPos_) != ValType)
      return PdbE_INVALID_PARAM;

    if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT8)
    {
      totalVal_ += DBVAL_ELE_GET_INT8(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT16)
    {
      totalVal_ += DBVAL_ELE_GET_INT16(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT32)
    {
      totalVal_ += DBVAL_ELE_GET_INT32(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT64)
    {
      totalVal_ += DBVAL_ELE_GET_INT64(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_FLOAT)
    {
      totalVal_ += DBVAL_ELE_GET_FLOAT(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_DOUBLE)
    {
      totalVal_ += DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_);
    }
    else
    {
      return PdbE_INVALID_PARAM;
    }

    dataCnt_++;
    return PdbE_OK;
  }

  PdbErr_t AppendArray(BlockValues& blockValues,
    uint64_t groupId, const std::vector<size_t>& groupIdVec) override
  {
    const uint8_t* pFilter = blockValues.GetFilter();

    if (groupIdVec.empty())
    {
      if (pFilter == nullptr)
        return AppendArrayInner<false, false>(blockValues, 0, nullptr);
      else
        return AppendArrayInner<false, true>(blockValues, 0, nullptr);
    }
    else
    {
      if (groupIdVec.size() != blockValues.GetRecordSize())
        return PdbE_INVALID_PARAM;

      if (pFilter == nullptr)
        return AppendArrayInner<true, false>(blockValues, groupId, groupIdVec.data());
      else
        return AppendArrayInner<true, true>(blockValues, groupId, groupIdVec.data());
    }
  }

  PdbErr_t GetResult(DBVal* pVal) override
  {
    if (dataCnt_ > 0)
    {
      if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT8)
      {
        DBVAL_SET_INT8(pVal, static_cast<int8_t>(totalVal_ / dataCnt_));
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT16)
      {
        DBVAL_SET_INT16(pVal, static_cast<int16_t>(totalVal_ / dataCnt_));
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT32)
      {
        DBVAL_SET_INT32(pVal, static_cast<int32_t>(totalVal_ / dataCnt_));
      }
      if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT64)
      {
        DBVAL_SET_INT64(pVal, (totalVal_ / dataCnt_));
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_FLOAT)
      {
        DBVAL_SET_FLOAT(pVal, static_cast<float>(totalVal_ / dataCnt_));
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_DOUBLE)
      {
        DBVAL_SET_DOUBLE(pVal, (totalVal_ / dataCnt_));
      }
      else
      {
        return PdbE_INVALID_PARAM;
      }
    }
    else
    {
      DBVAL_SET_NULL(pVal);
    }
    return PdbE_OK;
  }

  GroupField* NewField(int64_t devId, int64_t tstamp) override
  {
    return new AvgFunc<ValType, T>(fieldPos_);
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(fieldPos_);
  }

protected:
  template<bool HaveGroup, bool HaveFilter>
  PdbErr_t AppendArrayInner(const BlockValues& blockValues,
    uint64_t groupId, const uint64_t* pGroupIds)
  {
    size_t recCnt = blockValues.GetRecordSize();
    const DBVal* pVals = blockValues.GetColumnValues(fieldPos_);
    const uint8_t* pFilter = blockValues.GetFilter();

    for (size_t idx = 0; idx < recCnt; idx++)
    {
      if constexpr (HaveFilter)
      {
        if (pFilter[idx] == PDB_BOOL_FALSE)
          continue;
      }

      if constexpr (HaveGroup)
      {
        if (pGroupIds[idx] != groupId)
          continue;
      }

      if (DBVAL_ELE_IS_NULL(pVals, idx))
        continue;

      if (DBVAL_ELE_GET_TYPE(pVals, idx) != ValType)
        return PdbE_INVALID_PARAM;

      if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT8)
      {
        totalVal_ += DBVAL_ELE_GET_INT8(pVals, idx);
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT16)
      {
        totalVal_ += DBVAL_ELE_GET_INT16(pVals, idx);
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT32)
      {
        totalVal_ += DBVAL_ELE_GET_INT32(pVals, idx);
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT64)
      {
        totalVal_ += DBVAL_ELE_GET_INT64(pVals, idx);
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_FLOAT)
      {
        totalVal_ += DBVAL_ELE_GET_FLOAT(pVals, idx);
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_DOUBLE)
      {
        totalVal_ += DBVAL_ELE_GET_DOUBLE(pVals, idx);
      }
      else
      {
        return PdbE_INVALID_PARAM;
      }

      dataCnt_++;
    }

    return PdbE_OK;
  }

private:
  size_t fieldPos_;
  T totalVal_;
  int64_t dataCnt_;
};

///////////////////////////////////////////////////////////////////////////////

class CountFunc : public GroupField
{
public:
  CountFunc(size_t fieldPos)
  {
    this->fieldPos_ = fieldPos;
    this->dataCnt_ = 0;
  }

  virtual ~CountFunc() {}

  int32_t FieldType() override { return PDB_FIELD_TYPE::TYPE_INT64; }

  PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt) override
  {
    if (!DBVAL_ELE_IS_NULL(pVals, fieldPos_))
    {
      this->dataCnt_++;
    }

    return PdbE_OK;
  }

  PdbErr_t AppendArray(BlockValues& blockValues,
    uint64_t groupId, const std::vector<size_t>& groupIdVec) override
  {
    const uint8_t* pFilter = blockValues.GetFilter();
    if (groupIdVec.empty())
    {
      if (fieldPos_ <= PDB_TSTAMP_INDEX)
      {
        dataCnt_ += blockValues.GetResultSize();
        return PdbE_OK;
      }

      if (pFilter == nullptr)
        return AppendArrayInner<false, false>(blockValues, 0, nullptr);
      else
        return AppendArrayInner<false, true>(blockValues, 0, nullptr);
    }
    else
    {
      if (groupIdVec.size() != blockValues.GetRecordSize())
        return PdbE_INVALID_PARAM;

      if (pFilter == nullptr)
        return AppendArrayInner<true, false>(blockValues, groupId, groupIdVec.data());
      else
        return AppendArrayInner<true, true>(blockValues, groupId, groupIdVec.data());
    }
  }

  PdbErr_t GetResult(DBVal* pVal) override
  {
    DBVAL_SET_INT64(pVal, dataCnt_);
    return PdbE_OK;
  }

  GroupField* NewField(int64_t devId, int64_t tstamp) override
  {
    return new CountFunc(fieldPos_);
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(fieldPos_);
  }

protected:
  template<bool HaveGroup, bool HaveFilter>
  PdbErr_t AppendArrayInner(const BlockValues& blockValues,
    uint64_t groupId, const uint64_t* pGroupIds)
  {
    size_t recCnt = blockValues.GetRecordSize();
    const DBVal* pVals = blockValues.GetColumnValues(fieldPos_);
    const uint8_t* pFilter = blockValues.GetFilter();

    for (size_t idx = 0; idx < recCnt; idx++)
    {
      if constexpr (HaveFilter)
      {
        if (pFilter[idx] == PDB_BOOL_FALSE)
          continue;
      }

      if constexpr (HaveGroup)
      {
        if (pGroupIds[idx] != groupId)
          continue;
      }

      if (DBVAL_ELE_IS_NULL(pVals, idx))
        continue;

      dataCnt_++;
    }

    return PdbE_OK;
  }

private:
  size_t fieldPos_;
  int64_t dataCnt_;
};

/////////////////////////////////////////////////////////////////////////////////////

template<int ValType, bool IsFirst>
class FirstOrLastValueFunc : public GroupField
{
  static constexpr bool IsBlockValue = (ValType == PDB_FIELD_TYPE::TYPE_STRING || ValType == PDB_FIELD_TYPE::TYPE_BLOB);

public:
  FirstOrLastValueFunc(size_t fieldPos)
  {
    fieldPos_ = fieldPos;
    curTs_ = IsFirst ? DateTime::MaxMicrosecond : DateTime::MinMicrosecond;
    DBVAL_SET_NULL(&curVal_);

    if constexpr (IsBlockValue)
    {
      pValBuf_ = new std::string();
    }
    else
    {
      pValBuf_ = nullptr;
    }
  }

  virtual ~FirstOrLastValueFunc()
  {
    if (pValBuf_ != nullptr)
    {
      delete pValBuf_;
    }
  }

  int32_t FieldType() override { return ValType; }

  PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt) override
  {
    if (DBVAL_ELE_GET_TYPE(pVals, fieldPos_) != PDB_VALUE_TYPE::VAL_NULL
      && DBVAL_ELE_GET_TYPE(pVals, fieldPos_) != ValType)
    {
      return PdbE_INVALID_PARAM;
    }

    if constexpr (IsFirst)
    {
      if (DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX) >= curTs_)
        return PdbE_OK;
    }

    if constexpr (!IsFirst)
    {
      if (DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX) <= curTs_)
        return PdbE_OK;
    }

    if constexpr (IsBlockValue)
    {
      if (DBVAL_ELE_IS_NULL(pVals, fieldPos_))
      {
        DBVAL_SET_NULL(&curVal_);
      }
      else
      {
        size_t dataLen = DBVAL_ELE_GET_LEN(pVals, fieldPos_);
        if (dataLen > pValBuf_->capacity())
        {
          pValBuf_->resize(dataLen);
        }

        pValBuf_->resize(dataLen);
        memcpy(&((*pValBuf_)[0]), DBVAL_ELE_GET_BLOB(pVals, fieldPos_), dataLen);
        DBVAL_SET_BLOCK_VALUE(&curVal_, ValType, &((*pValBuf_)[0]), dataLen);
      }
    }
    else
    {
      curVal_ = pVals[fieldPos_];
    }

    curTs_ = DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX);
    return PdbE_OK;
  }

  PdbErr_t AppendArray(BlockValues& blockValues,
    uint64_t groupId, const std::vector<size_t>& groupIdVec) override
  {
    const uint8_t* pFilter = blockValues.GetFilter();

    if (groupIdVec.empty())
    {
      if (pFilter == nullptr)
        return AppendArrayInner<false, false>(blockValues, 0, nullptr);
      else
        return AppendArrayInner<false, true>(blockValues, 0, nullptr);
    }
    else
    {
      if (groupIdVec.size() != blockValues.GetRecordSize())
        return PdbE_INVALID_PARAM;

      if (pFilter == nullptr)
        return AppendArrayInner<true, false>(blockValues, groupId, groupIdVec.data());
      else
        return AppendArrayInner<true, true>(blockValues, groupId, groupIdVec.data());
    }
  }

  PdbErr_t GetResult(DBVal* pVal) override
  {
    *pVal = curVal_;
    return PdbE_OK;
  }

  GroupField* NewField(int64_t devId, int64_t tstamp) override
  {
    return new FirstOrLastValueFunc<ValType, IsFirst>(fieldPos_);
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(PDB_TSTAMP_INDEX);
    fieldSet.insert(fieldPos_);
  }

  bool IsFirstFunc() override { return IsFirst; }
  bool IsLastFunc() override { return !IsFirst; }

protected:
  template<bool HaveGroup, bool HaveFilter>
  PdbErr_t AppendArrayInner(const BlockValues& blockValues,
    uint64_t groupId, const uint64_t* pGroupIds)
  {
    size_t recCnt = blockValues.GetRecordSize();
    const DBVal* pTsVals = blockValues.GetColumnValues(PDB_TSTAMP_INDEX);
    const DBVal* pVals = blockValues.GetColumnValues(fieldPos_);
    const uint8_t* pFilter = blockValues.GetFilter();

    if (pTsVals == nullptr || pVals == nullptr)
    {
      return PdbE_INVALID_PARAM;
    }

    for (size_t idx = 0; idx < recCnt; idx++)
    {
      if constexpr (HaveFilter)
      {
        if (pFilter[idx] == PDB_BOOL_FALSE)
          continue;
      }

      if constexpr (HaveGroup)
      {
        if (pGroupIds[idx] != groupId)
          continue;
      }

      if (DBVAL_ELE_GET_TYPE(pTsVals, idx) != PDB_VALUE_TYPE::VAL_DATETIME)
        return PdbE_INVALID_PARAM;

      if (DBVAL_ELE_GET_TYPE(pVals, idx) != PDB_VALUE_TYPE::VAL_NULL
        && DBVAL_ELE_GET_TYPE(pVals, idx) != ValType)
      {
        return PdbE_INVALID_PARAM;
      }

      if constexpr (IsFirst)
      {
        if (DBVAL_ELE_GET_DATETIME(pTsVals, idx) >= curTs_)
          continue;
      }
      else
      {
        if (DBVAL_ELE_GET_DATETIME(pTsVals, idx) <= curTs_)
          continue;
      }

      curTs_ = DBVAL_ELE_GET_DATETIME(pTsVals, idx);

      if constexpr (IsBlockValue)
      {
        if (DBVAL_ELE_IS_NULL(pVals, idx))
        {
          DBVAL_SET_NULL(&curVal_);
        }
        else
        {
          size_t dataLen = DBVAL_ELE_GET_LEN(pVals, idx);
          if (dataLen > pValBuf_->capacity())
          {
            pValBuf_->resize(dataLen);
          }

          pValBuf_->resize(dataLen);
          memcpy(&((*pValBuf_)[0]), DBVAL_ELE_GET_BLOB(pVals, idx), dataLen);
          DBVAL_SET_BLOCK_VALUE(&curVal_, ValType, &((*pValBuf_)[0]), dataLen);
        }
      }
      else
      {
        curVal_ = pVals[idx];
      }

    }

    return PdbE_OK;
  }

private:
  size_t fieldPos_;
  int64_t curTs_;
  DBVal curVal_;
  std::string* pValBuf_;
};

/////////////////////////////////////////////////////////////////////////////////////

template<int CompareType, int TargetType, bool IsMax>
class ExtremeValueFunc : public GroupField
{
public:
  ExtremeValueFunc(size_t comparePos, size_t targetPos)
  {
    comparePos_ = comparePos;
    targetPos_ = targetPos;
    DBVAL_SET_NULL(&targetVal_);
    DBVAL_SET_NULL(&compareVal_);
    if constexpr (TargetType == PDB_FIELD_TYPE::TYPE_STRING || TargetType == PDB_FIELD_TYPE::TYPE_BLOB)
    {
      pTargetBuf_ = new std::string();
    }
    else
    {
      pTargetBuf_ = nullptr;
    }
  }

  virtual ~ExtremeValueFunc() { if (pTargetBuf_ != nullptr) { delete pTargetBuf_; } }
  int32_t FieldType() override { return TargetType; }
  PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt) override
  {
    if (DBVAL_ELE_IS_NULL(pVals, comparePos_))
      return PdbE_OK;

    if (DBVAL_ELE_GET_TYPE(pVals, targetPos_) != PDB_VALUE_TYPE::VAL_NULL
      && DBVAL_ELE_GET_TYPE(pVals, targetPos_) != TargetType)
      return PdbE_INVALID_PARAM;

    if (DBVAL_ELE_GET_TYPE(pVals, comparePos_) != CompareType)
      return PdbE_INVALID_PARAM;

    if (!DBVAL_IS_NULL(&compareVal_))
    {
      if constexpr (IsMax)
      {
        if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT8)
        {
          if (DBVAL_ELE_GET_INT8(pVals, comparePos_) <= DBVAL_GET_INT8(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT16)
        {
          if (DBVAL_ELE_GET_INT16(pVals, comparePos_) <= DBVAL_GET_INT16(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT32)
        {
          if (DBVAL_ELE_GET_INT32(pVals, comparePos_) <= DBVAL_GET_INT32(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT64)
        {
          if (DBVAL_ELE_GET_INT64(pVals, comparePos_) <= DBVAL_GET_INT64(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_FLOAT)
        {
          if (DBVAL_ELE_GET_INT64(pVals, comparePos_) <= DBVAL_GET_FLOAT(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_DOUBLE)
        {
          if (DBVAL_ELE_GET_DOUBLE(pVals, comparePos_) <= DBVAL_GET_DOUBLE(&compareVal_))
            return PdbE_OK;
        }
      }
      else
      {
        if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT8)
        {
          if (DBVAL_ELE_GET_INT8(pVals, comparePos_) >= DBVAL_GET_INT8(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT16)
        {
          if (DBVAL_ELE_GET_INT16(pVals, comparePos_) >= DBVAL_GET_INT16(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT32)
        {
          if (DBVAL_ELE_GET_INT32(pVals, comparePos_) >= DBVAL_GET_INT32(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT64)
        {
          if (DBVAL_ELE_GET_INT64(pVals, comparePos_) >= DBVAL_GET_INT64(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_FLOAT)
        {
          if (DBVAL_ELE_GET_INT64(pVals, comparePos_) >= DBVAL_GET_FLOAT(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_DOUBLE)
        {
          if (DBVAL_ELE_GET_DOUBLE(pVals, comparePos_) >= DBVAL_GET_DOUBLE(&compareVal_))
            return PdbE_OK;
        }
      }
    }

    compareVal_ = pVals[comparePos_];
    if constexpr (TargetType == PDB_FIELD_TYPE::TYPE_STRING || TargetType == PDB_FIELD_TYPE::TYPE_BLOB)
    {
      if (DBVAL_ELE_IS_NULL(pVals, targetPos_))
      {
        DBVAL_SET_NULL(&targetVal_);
      }
      else
      {
        size_t dataLen = DBVAL_ELE_GET_LEN(pVals, targetPos_);
        if (dataLen > pTargetBuf_->capacity())
        {
          pTargetBuf_->resize(dataLen);
        }

        memcpy(&((*pTargetBuf_)[0]), DBVAL_ELE_GET_BLOB(pVals, targetPos_), dataLen);
        pTargetBuf_->resize(dataLen);
        DBVAL_SET_BLOCK_VALUE(&targetVal_, TargetType, &((*pTargetBuf_)[0]), dataLen);
      }
    }
    else
    {
      targetVal_ = pVals[targetPos_];
    }
    return PdbE_OK;
  }

  PdbErr_t AppendArray(BlockValues& blockValues,
    uint64_t groupId, const std::vector<size_t>& groupIdVec) override
  {
    const uint8_t* pFilter = blockValues.GetFilter();

    if (groupIdVec.empty())
    {
      if (pFilter == nullptr)
        return AppendArrayInner<false, false>(blockValues, 0, nullptr);
      else
        return AppendArrayInner<false, true>(blockValues, 0, nullptr);
    }
    else
    {
      if (groupIdVec.size() != blockValues.GetRecordSize())
        return PdbE_INVALID_PARAM;

      if (pFilter == nullptr)
        return AppendArrayInner<true, false>(blockValues, groupId, groupIdVec.data());
      else
        return AppendArrayInner<true, true>(blockValues, groupId, groupIdVec.data());
    }
  }

  PdbErr_t GetResult(DBVal* pVal) override
  {
    *pVal = targetVal_;
    return PdbE_OK;
  }

  GroupField* NewField(int64_t devId, int64_t tstamp) override
  {
    return new ExtremeValueFunc<CompareType, TargetType, IsMax>(comparePos_, targetPos_);
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(comparePos_);
    fieldSet.insert(targetPos_);
  }

protected:
  template<bool HaveGroup, bool HaveFilter>
  PdbErr_t AppendArrayInner(const BlockValues& blockValues,
    uint64_t groupId, const uint64_t* pGroupIds)
  {
    size_t recCnt = blockValues.GetRecordSize();
    const DBVal* pTargetVals = blockValues.GetColumnValues(targetPos_);
    const DBVal* pCompareVals = blockValues.GetColumnValues(comparePos_);
    const uint8_t* pFilter = blockValues.GetFilter();

    for (size_t idx = 0; idx < recCnt; idx++)
    {
      if constexpr (HaveFilter)
      {
        if (pFilter[idx] == PDB_BOOL_FALSE)
          continue;
      }

      if constexpr (HaveGroup)
      {
        if (pGroupIds[idx] != groupId)
          continue;
      }

      if (DBVAL_ELE_IS_NULL(pCompareVals, idx))
        continue;

      if (DBVAL_ELE_GET_TYPE(pTargetVals, idx) != PDB_VALUE_TYPE::VAL_NULL
        && DBVAL_ELE_GET_TYPE(pTargetVals, idx) != TargetType)
        return PdbE_INVALID_PARAM;

      if (DBVAL_ELE_GET_TYPE(pCompareVals, idx) != CompareType)
        return PdbE_INVALID_PARAM;

      if (!DBVAL_IS_NULL(&compareVal_))
      {
        if constexpr (IsMax)
        {
          if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT8)
          {
            if (DBVAL_ELE_GET_INT8(pCompareVals, idx) <= DBVAL_GET_INT8(&compareVal_))
              continue;
          }
          else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT16)
          {
            if (DBVAL_ELE_GET_INT16(pCompareVals, idx) <= DBVAL_GET_INT16(&compareVal_))
              continue;
          }
          else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT32)
          {
            if (DBVAL_ELE_GET_INT32(pCompareVals, idx) <= DBVAL_GET_INT32(&compareVal_))
              continue;
          }
          else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT64)
          {
            if (DBVAL_ELE_GET_INT64(pCompareVals, idx) <= DBVAL_GET_INT32(&compareVal_))
              continue;
          }
          else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_FLOAT)
          {
            if (DBVAL_ELE_GET_FLOAT(pCompareVals, idx) <= DBVAL_GET_FLOAT(&compareVal_))
              continue;
          }
          else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_DOUBLE)
          {
            if (DBVAL_ELE_GET_DOUBLE(pCompareVals, idx) <= DBVAL_GET_DOUBLE(&compareVal_))
              continue;
          }
        }
        else
        {
          if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT8)
          {
            if (DBVAL_ELE_GET_INT8(pCompareVals, idx) >= DBVAL_GET_INT8(&compareVal_))
              continue;
          }
          else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT16)
          {
            if (DBVAL_ELE_GET_INT16(pCompareVals, idx) >= DBVAL_GET_INT16(&compareVal_))
              continue;
          }
          else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT32)
          {
            if (DBVAL_ELE_GET_INT32(pCompareVals, idx) >= DBVAL_GET_INT32(&compareVal_))
              continue;
          }
          else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT64)
          {
            if (DBVAL_ELE_GET_INT64(pCompareVals, idx) >= DBVAL_GET_INT32(&compareVal_))
              continue;
          }
          else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_FLOAT)
          {
            if (DBVAL_ELE_GET_FLOAT(pCompareVals, idx) >= DBVAL_GET_FLOAT(&compareVal_))
              continue;
          }
          else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_DOUBLE)
          {
            if (DBVAL_ELE_GET_DOUBLE(pCompareVals, idx) >= DBVAL_GET_DOUBLE(&compareVal_))
              continue;
          }
        }
      }

      compareVal_ = pCompareVals[idx];
      if constexpr (TargetType == PDB_FIELD_TYPE::TYPE_STRING || TargetType == PDB_FIELD_TYPE::TYPE_BLOB)
      {
        if (DBVAL_ELE_IS_NULL(pTargetVals, idx))
        {
          DBVAL_SET_NULL(&targetVal_);
        }
        else
        {
          size_t dataLen = DBVAL_ELE_GET_LEN(pTargetVals, idx);
          pTargetBuf_->resize(dataLen);
          memcpy(&((*pTargetBuf_)[0]), DBVAL_ELE_GET_BLOB(pTargetVals, idx), dataLen);
          DBVAL_SET_BLOCK_VALUE(&targetVal_, TargetType, &((*pTargetBuf_)[0]), dataLen);
        }
      }
      else
      {
        targetVal_ = pTargetVals[idx];
      }
    }

    return PdbE_OK;
  }

private:
  size_t comparePos_;
  size_t targetPos_;
  DBVal compareVal_;
  DBVal targetVal_;
  std::string* pTargetBuf_;
};

/////////////////////////////////////////////////////////////////////////////////////

template<int ValType, typename T>
class SumFunc : public GroupField
{
public:
  SumFunc(size_t fieldPos)
  {
    haveVal_ = false;
    fieldPos_ = fieldPos;
    sumVal_ = 0;
  }

  virtual ~SumFunc() {}

  int32_t FieldType() override { return ValType; }

  PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt) override
  {
    if (DBVAL_ELE_IS_NULL(pVals, fieldPos_))
      return PdbE_OK;

    if (DBVAL_ELE_GET_TYPE(pVals, fieldPos_) != ValType)
      return PdbE_INVALID_PARAM;

    haveVal_ = true;
    if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT8)
    {
      sumVal_ += DBVAL_ELE_GET_INT8(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT16)
    {
      sumVal_ += DBVAL_ELE_GET_INT16(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT32)
    {
      sumVal_ += DBVAL_ELE_GET_INT32(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT64)
    {
      sumVal_ += DBVAL_ELE_GET_INT64(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_FLOAT)
    {
      sumVal_ += DBVAL_ELE_GET_FLOAT(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_DOUBLE)
    {
      sumVal_ += DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_);
    }
    else
    {
      return PdbE_INVALID_PARAM;
    }

    return PdbE_OK;
  }

  PdbErr_t AppendArray(BlockValues& blockValues,
    uint64_t groupId, const std::vector<size_t>& groupIdVec) override
  {
    const uint8_t* pFilter = blockValues.GetFilter();
    if (groupIdVec.empty())
    {
      if (pFilter == nullptr)
        return AppendArrayInner<false, false>(blockValues, 0, nullptr);
      else
        return AppendArrayInner<false, true>(blockValues, 0, nullptr);
    }
    else
    {
      if (groupIdVec.size() != blockValues.GetRecordSize())
        return PdbE_INVALID_PARAM;

      if (pFilter == nullptr)
        return AppendArrayInner<true, false>(blockValues, groupId, groupIdVec.data());
      else
        return AppendArrayInner<true, true>(blockValues, groupId, groupIdVec.data());
    }
  }

  PdbErr_t GetResult(DBVal* pVal) override
  {
    if (!haveVal_)
    {
      DBVAL_SET_NULL(pVal);
    }
    else
    {
      if (PDB_TYPE_IS_NUMBER(ValType))
      {
        DBVAL_SET_INT64(pVal, sumVal_);
      }
      else if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(ValType))
      {
        DBVAL_SET_DOUBLE(pVal, sumVal_);
      }
    }

    return PdbE_OK;
  }

  GroupField* NewField(int64_t devId, int64_t tstamp) override
  {
    return new SumFunc<ValType, T>(fieldPos_);
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(fieldPos_);
  }

protected:
  template<bool HaveGroup, bool HaveFilter>
  PdbErr_t AppendArrayInner(const BlockValues& blockValues,
    uint64_t groupId, const uint64_t* pGroupIds)
  {
    size_t recCnt = blockValues.GetRecordSize();
    const DBVal* pVals = blockValues.GetColumnValues(fieldPos_);
    const uint8_t* pFilter = blockValues.GetFilter();

    for (size_t idx = 0; idx < recCnt; idx++)
    {
      if constexpr (HaveFilter)
      {
        if (pFilter[idx] == PDB_BOOL_FALSE)
          continue;
      }

      if constexpr (HaveGroup)
      {
        if (pGroupIds[idx] != groupId)
          continue;
      }

      if (DBVAL_ELE_IS_NULL(pVals, idx))
        continue;

      if (DBVAL_ELE_GET_TYPE(pVals, idx) != ValType)
        return PdbE_INVALID_PARAM;

      haveVal_ = true;
      if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT8)
      {
        sumVal_ += DBVAL_ELE_GET_INT8(pVals, idx);
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT16)
      {
        sumVal_ += DBVAL_ELE_GET_INT16(pVals, idx);
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT32)
      {
        sumVal_ += DBVAL_ELE_GET_INT32(pVals, idx);
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT64)
      {
        sumVal_ += DBVAL_ELE_GET_INT64(pVals, idx);
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_FLOAT)
      {
        sumVal_ += DBVAL_ELE_GET_FLOAT(pVals, idx);
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_DOUBLE)
      {
        sumVal_ += DBVAL_ELE_GET_DOUBLE(pVals, idx);
      }
      else
      {
        return PdbE_INVALID_PARAM;
      }
    }

    return PdbE_OK;
  }

private:
  bool haveVal_;
  size_t fieldPos_;
  T sumVal_;
};

/////////////////////////////////////////////////////////////////////////////////////

class GroupTstampField : public GroupField
{
public:
  GroupTstampField(int64_t tstamp)
  {
    tstamp_ = tstamp;
  }

  virtual ~GroupTstampField() {}

  int32_t FieldType() override { return PDB_FIELD_TYPE::TYPE_DATETIME; }

  PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt) override
  {
    return PdbE_OK;
  }

  PdbErr_t AppendArray(BlockValues& blockValues,
    uint64_t groupId, const std::vector<size_t>& groupIdVec) override
  {
    return PdbE_OK;
  }

  PdbErr_t GetResult(DBVal* pVal) override
  {
    DBVAL_SET_DATETIME(pVal, tstamp_);
    return PdbE_OK;
  }

  GroupField* NewField(int64_t devId, int64_t tstamp) override
  {
    return new GroupTstampField(tstamp);
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(PDB_TSTAMP_INDEX);
  }

  bool IsLastFunc() override { return true; };
  bool IsFirstFunc() override { return true; }

private:
  int64_t tstamp_;
};

class GroupDevIdField : public GroupField
{
public:
  GroupDevIdField(int64_t devId)
  {
    devId_ = devId;
  }

  virtual ~GroupDevIdField() {}

  int32_t FieldType() override { return PDB_FIELD_TYPE::TYPE_INT64; }

  PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt) override
  {
    return PdbE_OK;
  }

  PdbErr_t AppendArray(BlockValues& blockValues,
    uint64_t groupId, const std::vector<size_t>& groupIdVec) override
  {
    return PdbE_OK;
  }

  PdbErr_t GetResult(DBVal* pVal) override
  {
    DBVAL_SET_INT64(pVal, devId_);
    return PdbE_OK;
  }

  GroupField* NewField(int64_t devId, int64_t tstamp) override
  {
    return new GroupDevIdField(devId);
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(PDB_DEVID_INDEX);
  }

  virtual bool IsLastFunc() { return true; };
  virtual bool IsFirstFunc() { return true; }

private:
  int64_t devId_;
};

/////////////////////////////////////////////////////////////////////////////////////

class AggIfExtendFunc : public GroupField
{
public:
  AggIfExtendFunc(ValueItem* pCondition, GroupField* pField, bool isRoot)
  {
    root_ = isRoot;
    pCondition_ = pCondition;
    pField_ = pField;
  }

  virtual ~AggIfExtendFunc()
  {
    if (root_ && pCondition_ != nullptr)
      delete pCondition_;

    if (pField_ != nullptr)
      delete pField_;
  }

  int32_t FieldType() override { return pField_->FieldType(); }

  PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt) override
  {
    PdbErr_t retVal = PdbE_OK;
    DBVal condiVal;
    retVal = pCondition_->GetValue(pVals, &condiVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (DBVAL_IS_BOOL(&condiVal) && DBVAL_GET_BOOL(&condiVal))
    {
      return pField_->AppendSingle(pVals, valCnt);
    }

    return PdbE_OK;
  }

  PdbErr_t AppendArray(BlockValues& blockValues,
    uint64_t groupId, const std::vector<size_t>& groupIdVec) override
  {
    PdbErr_t retVal = PdbE_OK;
    std::vector<DBVal> tmpVec;
    tmpVec.reserve(blockValues.GetResultSize());

    retVal = pCondition_->GetValueArray(blockValues, tmpVec);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = blockValues.MergeFilter(tmpVec);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pField_->AppendArray(blockValues, groupId, groupIdVec);

    return retVal;
  }

  PdbErr_t GetResult(DBVal* pVal) override
  {
    return pField_->GetResult(pVal);
  }

  GroupField* NewField(int64_t devId, int64_t tstamp) override
  {
    GroupField* pNewField = pField_->NewField(devId, tstamp);
    return new AggIfExtendFunc(pCondition_, pNewField, false);
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pCondition_ != nullptr)
      pCondition_->GetUseFields(fieldSet);

    if (pField_ != nullptr)
      pField_->GetUseFields(fieldSet);
  }

private:
  bool root_;
  ValueItem* pCondition_;
  GroupField* pField_;
};






