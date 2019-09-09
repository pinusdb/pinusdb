/*
* Copyright (c) 2019 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
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
#include "table/db_obj.h"
#include "query/data_table.h"
#include "expr/expr_item.h"
#include "expr/sql_parser.h"
#include "query/result_object.h"
#include "query/condition_filter.h"
#include "util/arena.h"

class IResultFilter
{
public:
  IResultFilter();
  virtual ~IResultFilter();

  //补全结果集，查询 last, first 时需要使用
  virtual PdbErr_t InitGrpDevResult(const std::list<int64_t>& devIdList) { return PdbE_OK; }
  virtual PdbErr_t InitGrpTsResult() { return PdbE_OK; }
  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded) = 0;
  virtual PdbErr_t BuildCustomFilter(const QueryParam* pQueryParam, const TableInfo* pTabInfo, Arena* pArena) = 0;

  virtual PdbErr_t GetData(DataTable* pDataTable);

  virtual bool GetIsFullFlag() const = 0;
  virtual bool IsQueryLast() const { return false; }
  virtual bool IsQueryFirst() const { return false; }
  virtual bool IsGroupByDevId() const { return false; }
  virtual bool IsGroupByTstamp() const { return false; }

  size_t GetQueryOffset() const { return queryOffset_; }
  size_t GetQueryRecord() const { return queryRecord_; }

  bool IsEmptySet() const { return isEmptySet_; }
  PdbErr_t BuildFilter(const QueryParam* pQueryParam, const TableInfo* pTabInfo, Arena* pArena);

protected:
  PdbErr_t AddCountField(const std::string& aliasName);

  PdbErr_t AddAggField(int32_t opFunc, const std::string& aliasName,
    size_t fieldPos, int32_t fieldType, Arena* pArena);

  ResultField* AddAggBoolField(int32_t opFunc, size_t fieldPos);
  ResultField* AddAggInt64Field(int32_t opFunc, size_t fieldPos);
  ResultField* AddAggDoubleField(int32_t opFunc, size_t fieldPos);
  ResultField* AddAggDateTimeField(int32_t opFunc, size_t fieldPos);
  ResultField* AddAggStringField(int32_t opFunc, size_t fieldPos, Arena* pArena);
  ResultField* AddAggBlobField(int32_t opFunc, size_t fieldPos, Arena* pArena);


  ///////////////////////////////////////////////////////////////////////////

  PdbErr_t AddRawField(size_t fieldPos, const std::string& aliasName, int32_t fieldType, Arena* pArena);

  ///////////////////////////////////////////////////////////////////////////

  PdbErr_t AddAggTStampField(const std::string& aliasName);
  PdbErr_t AddAggDevIdField(const std::string& aliasName);

  ////////////////////////////////////////////////////////////////////////////

  PdbErr_t BuildGroupResultField(const std::vector<ExprItem*>& colItemVec,
    const TableInfo* pTabInfo, const GroupOpt* pGroup, Arena* pArena);

  PdbErr_t AddResultField(ResultField* pField, const std::string& aliasName);

protected:
  bool isEmptySet_;   //是否是空结果集
  bool isFull_;       //结果集是否已满

  size_t queryOffset_;
  size_t queryRecord_;

  ConditionFilter condiFilter_;
  TableInfo tabInfo_;

  std::unordered_map<uint64_t, ResultObject*> objMap_;
  std::vector<ResultObject*> objVec_;

  std::vector<ResultField*> fieldVec_;
};

