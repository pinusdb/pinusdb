#pragma once

#include "internal.h"
#include "query/result_field.h"

class CountFunc : public ResultField
{
public:
  CountFunc()
  {
    this->dataCnt_ = 0;
  }

  virtual ~CountFunc() {}

  virtual int32_t FieldType() { return PDB_FIELD_TYPE::TYPE_INT64; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    this->dataCnt_++;
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    DBVAL_SET_INT64(pVal, dataCnt_);
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new CountFunc();
  }

private:
  int64_t dataCnt_;
};
