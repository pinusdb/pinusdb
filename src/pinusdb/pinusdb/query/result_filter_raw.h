#pragma once

#include "pdb.h"
#include "query/result_filter.h"

class ResultFilterRaw : public IResultFilter
{
public:
  ResultFilterRaw();
  virtual ~ResultFilterRaw();

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded);
  virtual PdbErr_t BuildCustomFilter(const QueryParam* pQueryParam, const TableInfo* pTabInfo, Arena* pArena);

  virtual bool GetIsFullFlag() const { return objVec_.size() >= (size_t)queryRecord_; }


protected:

  PdbErr_t BuildResultField(const std::vector<ExprItem*>& colItemVec,
    const TableInfo* pTabInfo, Arena* pArena);
};

