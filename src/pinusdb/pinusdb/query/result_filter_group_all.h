#pragma once

#include "pdb.h"
#include "query/result_filter.h"

class ResultFilterGroupAll : public IResultFilter
{
public:
  ResultFilterGroupAll();
  virtual ~ResultFilterGroupAll();

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded);
  virtual PdbErr_t BuildCustomFilter(const QueryParam* pQueryParam, const TableInfo* pTabInfo, Arena* pArena);

  virtual bool GetIsFullFlag() const;

};

