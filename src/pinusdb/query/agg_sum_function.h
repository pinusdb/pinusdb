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
#include "query/result_field.h"

class SumNumFunc : public ResultField
{
public:
  SumNumFunc(size_t fieldPos)
  {
    haveVal_ = false;
    fieldPos_ = fieldPos;
    sumVal_ = 0;
  }

  virtual ~SumNumFunc() {}

  virtual int32_t FieldType() { return PDB_FIELD_TYPE::TYPE_INT64; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (!DBVAL_ELE_IS_NULL(pVals, fieldPos_))
    {
      haveVal_ = true;
      sumVal_ += DBVAL_ELE_GET_INT64(pVals, fieldPos_);
    }

    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    if (haveVal_)
      DBVAL_SET_INT64(pVal, sumVal_);
    else
      DBVAL_SET_NULL(pVal);

    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new SumNumFunc(fieldPos_);
  }

private:
  bool haveVal_;
  size_t fieldPos_;
  int64_t sumVal_;
};

class SumDoubleFunc : public ResultField
{
public:

  SumDoubleFunc(size_t fieldPos)
  {
    haveVal_ = false;
    fieldPos_ = fieldPos;
    sumVal_ = 0;
  }

  virtual ~SumDoubleFunc() {}

  virtual int32_t FieldType() { return PDB_FIELD_TYPE::TYPE_DOUBLE; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (!DBVAL_ELE_IS_DOUBLE(pVals, fieldPos_))
      return PdbE_INVALID_PARAM;

    if (!DBVAL_ELE_IS_NULL(pVals, fieldPos_))
    {
      haveVal_ = true;
      sumVal_ += DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_);
    }

    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    if (haveVal_)
      DBVAL_SET_DOUBLE(pVal, sumVal_);
    else
      DBVAL_SET_NULL(pVal);

    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new SumDoubleFunc(fieldPos_);
  }

private:
  bool haveVal_;
  size_t fieldPos_;
  double sumVal_;
};
