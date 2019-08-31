#pragma once

#include "internal.h"
#include "util/arena.h"

template<int ValType>
class LastValFunc : public ResultField
{
public:
  LastValFunc(size_t fieldPos)
  {
    fieldPos_ = fieldPos;
    lastTStamp_ = -1;
    DBVAL_SET_NULL(&lastVal_);
  }

  virtual ~LastValFunc() {}

  virtual int32_t FieldType() { return ValType; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (!DBVAL_ELE_IS_DATETIME(pVals, PDB_TSTAMP_INDEX)
      || !DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return PdbE_INVALID_PARAM;

    if (DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX) > lastTStamp_)
    {
      lastTStamp_ = DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX);
      lastVal_ = pVals[fieldPos_];
    }

    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    *pVal = lastVal_;
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new LastValFunc<ValType>(fieldPos_);
  }

  virtual bool IsLastFunc() { return true; }

private:
  size_t fieldPos_;
  int64_t lastTStamp_;
  DBVal lastVal_;
};

template<int ValType>
class LastBlockFunc : public ResultField
{
public:
  LastBlockFunc(size_t fieldPos, Arena* pArena)
  {
    fieldPos_ = fieldPos;
    lastTStamp_ = -1;
    DBVAL_SET_NULL(&lastVal_);

    pArena_ = pArena;
    pBuf_ = nullptr;
    bufLen_ = 0;
  }

  virtual ~LastBlockFunc() {}

  virtual int32_t FieldType() { return ValType; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (!DBVAL_ELE_IS_DATETIME(pVals, PDB_TSTAMP_INDEX)
      || !DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return PdbE_INVALID_PARAM;

    if (DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX) > lastTStamp_)
    {
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
        DBVAL_SET_STRING(&lastVal_, pBuf_, DBVAL_ELE_GET_LEN(pVals, fieldPos_));
      else if (ValType == PDB_FIELD_TYPE::TYPE_BLOB)
        DBVAL_SET_BLOB(&lastVal_, pBuf_, DBVAL_ELE_GET_LEN(pVals, fieldPos_));
      else
        return PdbE_INVALID_PARAM;

      lastTStamp_ = DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX);
    }

    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    *pVal = lastVal_;
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new LastBlockFunc<ValType>(fieldPos_, pArena_);
  }

  virtual bool IsLastFunc() { return true; }

private:
  size_t fieldPos_;
  int64_t lastTStamp_;
  DBVal lastVal_;

  Arena* pArena_;
  uint8_t* pBuf_;
  size_t bufLen_;
};
