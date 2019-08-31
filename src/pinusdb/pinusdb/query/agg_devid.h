#pragma once

#include "internal.h"
#include "query/result_field.h"

class GroupDevIdField : public ResultField
{
public:
  GroupDevIdField(int64_t devId)
  {
    devId_ = devId;
  }

  virtual ~GroupDevIdField() {}

  virtual int32_t FieldType() { return PDB_FIELD_TYPE::TYPE_INT64; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    DBVAL_SET_INT64(pVal, devId_);
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new GroupDevIdField(devId);
  }

  virtual bool IsLastFunc() { return true; };
  virtual bool IsFirstFunc() { return true; }

private:
  int64_t devId_;
};
