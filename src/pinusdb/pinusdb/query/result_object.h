#pragma once

#include "query/result_field.h"
#include "expr/expr_item.h"
#include "expr/sql_parser.h"
#include "table/data_column.h"
#include "table/db_obj.h"
#include "query/data_table.h"
#include <string>
#include <vector>

class ResultObject
{
public:
  ResultObject(const std::vector<ResultField*>& fieldVec, 
    int64_t devId, int64_t tstamp);
  virtual ~ResultObject();

  PdbErr_t AppendData(const DBVal* pVals, size_t valCnt);

  PdbErr_t GetResultObj(DBObj* pObj) const;

private:
  std::vector<ResultField*> fieldVec_;
};

