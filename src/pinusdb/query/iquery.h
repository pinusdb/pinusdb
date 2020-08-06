
#pragma once

#include <string>
#include <vector>
#include "expr/sql_parser.h"
#include "query/result_object.h"
#include "query/condition_filter.h"
#include "util/arena.h"
#include "table/table_info.h"

class IQuery
{
public:
  IQuery() {}
  virtual ~IQuery() {}

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded) = 0;
  virtual bool GetIsFullFlag() const = 0;
  virtual PdbErr_t GetResult(std::string& dataBuf, uint32_t* pFieldCnt, uint32_t* pRecordCnt) = 0;

  virtual void GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const = 0;
  virtual void GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const = 0;
  virtual bool FilterDevId(int64_t devId) const = 0;
  virtual bool IsEmptySet() const = 0;
  virtual size_t GetQueryOffset() const = 0;
  virtual size_t GetQueryRecord() const = 0;
  virtual PdbErr_t BuildQuery(const QueryParam* pQueryParam, const TableInfo* pTabInfo) = 0;

  //snapshot สนำร
  virtual int64_t GetMaxDevId() const { return INT64_MAX; }
};


