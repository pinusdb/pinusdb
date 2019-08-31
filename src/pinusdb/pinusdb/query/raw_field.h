#pragma once

#include "internal.h"
#include "query/result_field.h"
#include "util/arena.h"

template<int ValType>
class RawValField : public ResultField
{
public:
  RawValField(size_t fieldPos)
  {
    fieldPos_ = fieldPos;
    DBVAL_SET_NULL(&val_);
  }

  virtual ~RawValField() {}

  virtual int32_t FieldType()
  {
    return ValType;
  }
  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return PdbE_INVALID_PARAM;

    val_ = pVals[fieldPos_];

    return PdbE_OK;
  }
  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    *pVal = val_;
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new RawValField<ValType>(fieldPos_);
  }

  virtual bool IsAggFunc() { return false; };

private:
  size_t fieldPos_;
  DBVal val_;
};

template<int ValType>
class RawBlockField : public ResultField
{
public:
  RawBlockField(size_t fieldPos, Arena* pArena)
  {
    fieldPos_ = fieldPos;
    DBVAL_SET_NULL(&val_);
    pArena_ = pArena;
  }

  virtual ~RawBlockField() {}

  virtual int32_t FieldType() { return ValType; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return PdbE_INVALID_PARAM;

    char* pBuf = nullptr;
    int32_t bufLen = 0;

    if (DBVAL_ELE_GET_LEN(pVals, fieldPos_) > 0)
    {
      bufLen = DBVAL_ELE_GET_LEN(pVals, fieldPos_);
      pBuf = pArena_->Allocate(bufLen);
      if (pBuf == nullptr)
        return PdbE_NOMEM;

      memcpy(pBuf, DBVAL_ELE_GET_BLOB(pVals, fieldPos_), bufLen);
    }

    if (ValType == PDB_FIELD_TYPE::TYPE_STRING)
      DBVAL_SET_STRING(&val_, pBuf, bufLen);
    else if (ValType == PDB_FIELD_TYPE::TYPE_BLOB)
      DBVAL_SET_BLOB(&val_, pBuf, bufLen);
    else
      return PdbE_INVALID_PARAM;

    return PdbE_OK;
  }
  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    *pVal = val_;
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new RawBlockField<ValType>(fieldPos_, pArena_);
  }

  virtual bool IsAggFunc() { return false; };

private:
  size_t fieldPos_;
  DBVal val_;

  Arena* pArena_;
};