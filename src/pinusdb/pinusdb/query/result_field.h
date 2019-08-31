#pragma once

#include "internal.h"
#include "table/db_value.h"

class ResultField
{
public:
  ResultField() {}
  virtual ~ResultField() {}

  virtual int32_t FieldType() = 0;
  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt) = 0;
  virtual PdbErr_t GetResult(DBVal* pVal) = 0;

  virtual ResultField* NewField(int64_t devId, int64_t tstamp) = 0;

  virtual bool IsAggFunc() { return true; };
  virtual bool IsLastFunc() { return false; };
  virtual bool IsFirstFunc() { return false; }
};

