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

#pragma once

#include "internal.h"
#include "query/result_field.h"

template<int CompareType>
class MinValueFunc : public ResultField
{
public:
  MinValueFunc(size_t comparePos, size_t targetPos, int32_t targetType)
  {
    haveVal_ = false;
    targetType_ = targetType;
    targetPos_ = targetPos;
    comparePos_ = comparePos;
    compareIntVal_ = 0;
    compareDoubleVal_ = 0;
    DBVAL_SET_NULL(&targetVal_);
  }

  virtual ~MinValueFunc() {}

  virtual int32_t FieldType() { return targetType_; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (DBVAL_ELE_IS_NULL(pVals, comparePos_))
      return PdbE_OK;

    if (CompareType == PDB_FIELD_TYPE::TYPE_DOUBLE)
    {
      if (haveVal_ && DBVAL_ELE_GET_DOUBLE(pVals, comparePos_) >= compareDoubleVal_)
        return PdbE_OK;

      compareDoubleVal_ = DBVAL_ELE_GET_DOUBLE(pVals, comparePos_);
    }
    else
    {
      if (haveVal_ && DBVAL_ELE_GET_INT64(pVals, comparePos_) >= compareIntVal_)
        return PdbE_OK;

      compareIntVal_ = DBVAL_ELE_GET_INT64(pVals, comparePos_);
    }

    haveVal_ = true;
    targetVal_ = pVals[targetPos_];
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    *pVal = targetVal_;
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new MinValueFunc<CompareType>(comparePos_, targetPos_, targetType_);
  }

private:
  bool haveVal_;
  int targetType_;
  size_t targetPos_;
  size_t comparePos_;
  int64_t compareIntVal_;
  double compareDoubleVal_;
  DBVal targetVal_;
};

template<int CompareType>
class MinBlockFunc : public ResultField
{
public:
  MinBlockFunc(size_t comparePos, size_t targetPos, int32_t targetType, Arena* pArena)
  {
    haveVal_ = false;
    targetType_ = targetType;
    targetPos_ = targetPos;
    comparePos_ = comparePos;
    compareIntVal_ = 0;
    compareDoubleVal_ = 0;
    DBVAL_SET_NULL(&targetVal_);

    pArena_ = pArena;
    pBuf_ = nullptr;
    bufLen_ = 0;
  }

  virtual ~MinBlockFunc() {}

  virtual int32_t FieldType() { return targetType_; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (DBVAL_ELE_IS_NULL(pVals, comparePos_))
      return PdbE_OK;

    if (CompareType == PDB_FIELD_TYPE::TYPE_DOUBLE)
    {
      if (haveVal_ && DBVAL_ELE_GET_DOUBLE(pVals, comparePos_) >= compareDoubleVal_)
        return PdbE_OK;

      compareDoubleVal_ = DBVAL_ELE_GET_DOUBLE(pVals, comparePos_);
    }
    else
    {
      if (haveVal_ && DBVAL_ELE_GET_INT64(pVals, comparePos_) >= compareIntVal_)
        return PdbE_OK;

      compareIntVal_ = DBVAL_ELE_GET_INT64(pVals, comparePos_);
    }

    haveVal_ = true;
    if (DBVAL_ELE_IS_NULL(pVals, targetPos_))
    {
      DBVAL_SET_NULL(&targetVal_);
      return PdbE_OK;
    }

    if (DBVAL_ELE_GET_LEN(pVals, targetPos_) > bufLen_)
    {
      size_t tmpLen = DBVAL_ELE_GET_LEN(pVals, targetPos_) + 32;
      pBuf_ = (uint8_t*)pArena_->Allocate(tmpLen);
      if (pBuf_ == nullptr)
        return PdbE_NOMEM;

      bufLen_ = tmpLen;
    }

    memcpy(pBuf_, DBVAL_ELE_GET_BLOB(pVals, targetPos_), DBVAL_ELE_GET_LEN(pVals, targetPos_));
    DBVAL_SET_BLOCK_VALUE(&targetVal_, targetType_, pBuf_, DBVAL_ELE_GET_LEN(pVals, targetPos_));
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    *pVal = targetVal_;
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new MinBlockFunc<CompareType>(comparePos_, targetPos_, targetType_, pArena_);
  }

private:
  bool haveVal_;
  int targetType_;
  size_t targetPos_;
  size_t comparePos_;
  int64_t compareIntVal_;
  double compareDoubleVal_;
  DBVal targetVal_;

  Arena* pArena_;
  uint8_t* pBuf_;
  size_t bufLen_;
};
