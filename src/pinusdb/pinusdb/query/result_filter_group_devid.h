#pragma once

#include "pdb.h"
#include "query/result_filter.h"

class ResultFilterGroupDevID : public IResultFilter
{
public:
  ResultFilterGroupDevID();
  virtual ~ResultFilterGroupDevID();

  virtual PdbErr_t InitGrpDevResult(const std::list<int64_t>& devIdList);
  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded);
  virtual PdbErr_t BuildCustomFilter(const QueryParam* pQueryParam, const TableInfo* pTabInfo, Arena* pArena);

  virtual bool GetIsFullFlag() const { return false; }

  virtual bool IsQueryLast() const;
  virtual bool IsQueryFirst() const;
  virtual bool IsGroupByDevId() const { return true; }

protected:
  uint64_t lastGroupId_;
  ResultObject* pLastObj_;
};

