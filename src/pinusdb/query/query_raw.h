/*
* Copyright (c) 2020 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; If not, see <http://www.gnu.org/licenses>
*/

#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include "query/iquery.h"
#include "expr/sql_parser.h"
#include "query/result_object.h"
#include "query/condition_filter.h"
#include "table/table_info.h"
#include "query/value_item.h"

class QueryRaw : public IQuery
{
public:
  QueryRaw();
  virtual ~QueryRaw();

  PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt, bool* pIsAdded) override;
  PdbErr_t AppendArray(BlockValues& blockValues, bool* pIsAdded) override;
  bool GetIsFullFlag() const override;
  PdbErr_t GetResult(std::string& dataBuf, uint32_t* pFieldCnt, uint32_t* pRecordCnt) override;

  void GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const override;
  void GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const override;
  bool FilterDevId(int64_t devId) const override;
  bool IsEmptySet() const override;
  size_t GetQueryOffset() const override;
  size_t GetQueryRecord() const override;
  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override;

  PdbErr_t BuildQuery(const QueryParam* pQueryParam, const TableInfo* pTabInfo) override;


private:
  size_t queryOffset_;
  size_t queryRecord_;
  ConditionFilter condiFilter_;
  std::vector<ValueItem*> fieldVec_;

  TableInfo tabInfo_;
  size_t curRecord_;
  std::string resultData_;
};
