#pragma once

#include <string>
#include <vector>
#include "util/arena.h"
#include "query/iquery.h"
#include "expr/sql_parser.h"
#include "query/result_object.h"
#include "query/condition_filter.h"
#include "table/table_info.h"
#include "query/value_item.h"

class TmpRecord;

class QuerySnapshot : public IQuery
{
public:
  QuerySnapshot();
  virtual ~QuerySnapshot();

  PdbErr_t AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded) override;
  bool GetIsFullFlag() const override;
  PdbErr_t GetResult(std::string& dataBuf, uint32_t* pFieldCnt, uint32_t* pRecordCnt) override;

  void GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const override;
  void GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const override;
  bool FilterDevId(int64_t devId) const override;
  bool IsEmptySet() const override;
  size_t GetQueryOffset() const override;
  size_t GetQueryRecord() const override;

  //snapshot สนำร
  int64_t GetMaxDevId() const override;

  PdbErr_t BuildQuery(const QueryParam* pQueryParam,
    const TableInfo* pTabInfo);

private:
  size_t queryOffset_;
  size_t queryRecord_;
  ConditionFilter condiFilter_;

  int64_t maxDevId_;
  std::vector<TmpRecord*> recVec_;

  TableInfo tabInfo_;
  std::vector<ValueItem*> fieldVec_;
};


