#pragma once

#include "pdb.h"
#include "query/result_filter.h"

class ResultFilterGroupTStamp : public IResultFilter
{
public:
  ResultFilterGroupTStamp();
  virtual ~ResultFilterGroupTStamp();

  virtual PdbErr_t InitGrpTsResult();
  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded);
  virtual PdbErr_t BuildCustomFilter(const QueryParam* pQueryParam, const TableInfo* pTabInfo, Arena* pArena);

  virtual bool GetIsFullFlag() const { return false; }
  virtual bool IsGroupByTstamp() const { return true; }

protected:
  int64_t timeGroupRange_;
  int64_t minTimeStamp_;
  int64_t maxTimeStamp_;

  uint64_t lastGroupId_;
  ResultObject* pLastObj_;
};


