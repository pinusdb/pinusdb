#pragma once

#include "internal.h"
#include "query/result_field.h"
#include "query/result_field.h"

template<int ValType>
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
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return PdbE_INVALID_PARAM;

    haveVal_ = true;
    switch (ValType)
    {
    case PDB_VALUE_TYPE::VAL_INT64:
      sumVal_ += DBVAL_ELE_GET_INT64(pVals, fieldPos_);
      break;
    default:
      return PdbE_INVALID_PARAM;
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
    return new SumNumFunc<ValType>(fieldPos_);
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

    haveVal_ = true;
    sumVal_ += DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_);

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
