#pragma once

#include "internal.h"
#include "query/result_field.h"

template<int ValType>
class MinNumFunc : public ResultField
{
public:
  MinNumFunc(size_t fieldPos)
  {
    haveVal_ = false;
    fieldPos_ = fieldPos;
    val_ = 0;
  }

  virtual ~MinNumFunc() {}

  virtual int32_t FieldType() { return ValType; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return PdbE_INVALID_PARAM;

    int64_t curVal = 0;
    switch (ValType)
    {
    case PDB_VALUE_TYPE::VAL_INT64:
      curVal = DBVAL_ELE_GET_INT64(pVals, fieldPos_);
      break;
    case PDB_VALUE_TYPE::VAL_DATETIME:
      curVal = DBVAL_ELE_GET_DATETIME(pVals, fieldPos_);
      break;
    default:
      return PdbE_INVALID_PARAM;
    }

    if (!haveVal_ || curVal < val_)
    {
      haveVal_ = true;
      val_ = curVal;
    }

    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    if (haveVal_)
    {
      switch (ValType)
      {
      case PDB_VALUE_TYPE::VAL_INT64:
        DBVAL_SET_INT64(pVal, val_);
        break;
      case PDB_VALUE_TYPE::VAL_DATETIME:
        DBVAL_SET_DATETIME(pVal, val_);
        break;
      default:
        return PdbE_INVALID_PARAM;
      }
    }
    else
    {
      DBVAL_SET_NULL(pVal);
    }

    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new MinNumFunc<ValType>(fieldPos_);
  }

private:
  bool haveVal_;
  size_t fieldPos_;
  int64_t val_;
};

class MinDoubleFunc : public ResultField
{
public:
  MinDoubleFunc(size_t fieldPos)
  {
    haveVal_ = false;
    fieldPos_ = fieldPos;
    val_ = 0;
  }

  virtual ~MinDoubleFunc() {}

  virtual int32_t FieldType() { return PDB_FIELD_TYPE::TYPE_DOUBLE; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (!DBVAL_ELE_IS_DOUBLE(pVals, fieldPos_))
      return PdbE_INVALID_PARAM;

    if (!haveVal_ || DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_) < val_)
    {
      haveVal_ = true;
      val_ = DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_);
    }

    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    if (haveVal_)
    {
      DBVAL_SET_DOUBLE(pVal, val_);
    }
    else
    {
      DBVAL_SET_NULL(pVal);
    }

    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new MinDoubleFunc(fieldPos_);
  }

private:
  bool haveVal_;
  size_t fieldPos_;
  double val_;
};
