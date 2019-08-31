#pragma once
#include "internal.h"
#include "query/result_filter.h"
#include "query/dev_filter.h"
#include "query/tstamp_filter.h"

class ITablePart
{
public:
  ITablePart() {}
  virtual ~ITablePart() {}

  virtual PdbErr_t QueryDataASC(const DevFilter* pDevFilter, const TStampFilter* pTSFilter, 
    IResultFilter* pResult, uint64_t timeOut) = 0;
  virtual PdbErr_t QueryDataDESC(const DevFilter* pDevFilter, const TStampFilter* pTSFilter, 
    IResultFilter* pResult, uint64_t timeOut) = 0;
  virtual PdbErr_t QueryLast(int64_t devId, int64_t maxTs, 
    IResultFilter* pResult, uint64_t timeOut, bool& isAdded) = 0;
  virtual PdbErr_t QueryFirst(int64_t devId, int64_t minTs, 
    IResultFilter* pResult, uint64_t timeOut, bool& isAdded) = 0;
};
