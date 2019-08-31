#pragma once

#include "internal.h"
#include "query/result_field.h"

class GroupTstampField : public ResultField
{
public:
  GroupTstampField(int64_t tstamp)
  {
    tstamp_ = tstamp;
  }

  virtual ~GroupTstampField() {}
  
  virtual int32_t FieldType() { return PDB_FIELD_TYPE::TYPE_DATETIME; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    DBVAL_SET_DATETIME(pVal, tstamp_);
    return PdbE_OK;
  }

  virtual ResultField* NewField(int64_t devId, int64_t tstamp)
  {
    return new GroupTstampField(tstamp);
  }

private:
  int64_t tstamp_;
};
