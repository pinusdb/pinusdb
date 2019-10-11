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

#include "util/arena.h"
#include "query/result_field.h"
#include "util/date_time.h"

template<int ValType>
class FirstValFunc : public ResultField
{
public:
  FirstValFunc(size_t fieldPos)
  {
    fieldPos_ = fieldPos;
    firstTStamp_ = MaxMillis;
    DBVAL_SET_NULL(&firstVal_);
  }

  virtual ~FirstValFunc() {}

  virtual int32_t FieldType() { return ValType; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX) < firstTStamp_)
    {
      firstTStamp_ = DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX);
      firstVal_ = pVals[fieldPos_];
    }

    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    *pVal = firstVal_;
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new FirstValFunc<ValType>(fieldPos_);
  }

  virtual bool IsFirstFunc() { return true; }

private:
  size_t fieldPos_;
  int64_t firstTStamp_;
  DBVal firstVal_;
};

template<int ValType>
class FirstBlockFunc : public ResultField
{
public:
  FirstBlockFunc(size_t fieldPos, Arena* pArena)
  {
    fieldPos_ = fieldPos;
    firstTStamp_ = MaxMillis;
    DBVAL_SET_NULL(&firstVal_);

    pArena_ = pArena;
    pBuf_ = nullptr;
    bufLen_ = 0;
  }

  virtual ~FirstBlockFunc() {}

  virtual int32_t FieldType() { return ValType; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    do {
      if (DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX) >= firstTStamp_)
        break;

      firstTStamp_ = DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX);
      if (DBVAL_ELE_IS_NULL(pVals, fieldPos_))
      {
        DBVAL_SET_NULL(&firstVal_);
        break;
      }

      if (DBVAL_ELE_GET_LEN(pVals, fieldPos_) > 0)
      {
        if (DBVAL_ELE_GET_LEN(pVals, fieldPos_) > bufLen_)
        {
          size_t tmpLen = DBVAL_ELE_GET_LEN(pVals, fieldPos_) + 32;
          pBuf_ = (uint8_t*)pArena_->Allocate(tmpLen);
          if (pBuf_ == nullptr)
            return PdbE_NOMEM;

          bufLen_ = tmpLen;
        }

        memcpy(pBuf_, DBVAL_ELE_GET_BLOB(pVals, fieldPos_), DBVAL_ELE_GET_LEN(pVals, fieldPos_));
      }

      if (ValType == PDB_FIELD_TYPE::TYPE_STRING)
        DBVAL_SET_STRING(&firstVal_, pBuf_, DBVAL_ELE_GET_LEN(pVals, fieldPos_));
      else if (ValType == PDB_FIELD_TYPE::TYPE_BLOB)
        DBVAL_SET_BLOB(&firstVal_, pBuf_, DBVAL_ELE_GET_LEN(pVals, fieldPos_));

    } while (false);

    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    *pVal = firstVal_;
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new FirstBlockFunc<ValType>(fieldPos_, pArena_);
  }

  virtual bool IsFirstFunc() { return true; }

private:
  size_t fieldPos_;
  int64_t firstTStamp_;
  DBVal firstVal_;

  Arena* pArena_;
  uint8_t* pBuf_;
  size_t bufLen_;
};
