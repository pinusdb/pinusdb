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

class AvgBigIntFunc : public ResultField
{
public:
  AvgBigIntFunc(size_t fieldPos)
  {
    this->fieldPos_ = fieldPos;
    this->totalVal_ = 0;
    this->dataCnt_ = 0;
  }

  virtual ~AvgBigIntFunc() { }

  virtual int32_t FieldType() { return PDB_VALUE_TYPE::VAL_INT64; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (DBVAL_ELE_IS_NULL(pVals, fieldPos_))
      return PdbE_OK;

    if (!DBVAL_ELE_IS_INT64(pVals, fieldPos_))
      return PdbE_INVALID_PARAM;

    totalVal_ += DBVAL_ELE_GET_INT64(pVals, fieldPos_);
    dataCnt_++;
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    if (dataCnt_ > 0)
    {
      DBVAL_SET_INT64(pVal, static_cast<int64_t>(totalVal_ / dataCnt_));
    }
    else
    {
      DBVAL_SET_NULL(pVal);
    }
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new AvgBigIntFunc(fieldPos_);
  }

private:
  size_t fieldPos_;
  int64_t totalVal_;
  int64_t dataCnt_;
};

class AvgDoubleFunc : public ResultField
{
public:
  AvgDoubleFunc(size_t fieldPos)
  {
    fieldPos_ = fieldPos;
    totalVal_ = 0;
    dataCnt_ = 0;
  }

  virtual ~AvgDoubleFunc() {}

  virtual int32_t FieldType() { return PDB_VALUE_TYPE::VAL_DOUBLE; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (DBVAL_ELE_IS_NULL(pVals, fieldPos_))
      return PdbE_OK;

    if (!DBVAL_ELE_IS_DOUBLE(pVals, fieldPos_))
      return PdbE_INVALID_PARAM;

    totalVal_ += DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_);
    dataCnt_++;
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    if (dataCnt_ > 0)
    {
      DBVAL_SET_DOUBLE(pVal, (totalVal_ / dataCnt_));
    }
    else
    {
      DBVAL_SET_NULL(pVal);
    }

    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new AvgDoubleFunc(fieldPos_);
  }

private:
  size_t fieldPos_;
  double totalVal_;
  int64_t dataCnt_;
};
