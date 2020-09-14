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
#include "query/iquery.h"
#include "expr/sql_parser.h"
#include "query/result_object.h"
#include "query/condition_filter.h"
#include "table/table_info.h"
#include "query/group_field.h"

class QueryGroup : public IQuery
{
public:
  QueryGroup();
  virtual ~QueryGroup();

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

  virtual bool IsQueryLast() const;
  virtual bool IsQueryFirst() const;

  PdbErr_t BuildQuery(const QueryParam* pQueryParam, const TableInfo* pTabInfo) override;

protected:
  virtual int64_t GetGroupId(const DBVal* pVals, size_t valCnt) = 0;
  virtual PdbErr_t GetGroupArray(BlockValues& blockValues, std::vector<uint64_t>& groupIdVec, bool& singleGroup) = 0;
  virtual PdbErr_t CustomBuild(const QueryParam* pQueryParam) { return PdbE_OK; }

protected:
  size_t queryOffset_;
  size_t queryRecord_;
  std::vector<ValueItem*> rstFieldVec_;
  TableInfo resultInfo_;

  ConditionFilter condiFilter_;
  std::unordered_map<uint64_t, ResultObject*> objMap_;
  std::vector<ResultObject*> objVec_;
  std::vector<GroupField*> grpFieldVec_;
  TableInfo groupInfo_;

  uint64_t lastGroupId_;
  ResultObject* pLastObj_;
};

class QueryGroupAll : public QueryGroup
{
public:
  QueryGroupAll();
  virtual ~QueryGroupAll();

  bool IsEmptySet() const override;

protected:
  int64_t GetGroupId(const DBVal* pVals, size_t valCnt) override;
  PdbErr_t GetGroupArray(BlockValues& blockValues, std::vector<uint64_t>& groupIdVec, bool& singleGroup) override;
  PdbErr_t CustomBuild(const QueryParam* pQueryParam) override;
};

class QueryGroupDevID : public QueryGroup
{
public:
  QueryGroupDevID();
  virtual ~QueryGroupDevID();

  PdbErr_t InitGroupDevID(const std::list<int64_t>& devIdList);
  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override;

protected:
  int64_t GetGroupId(const DBVal* pVals, size_t valCnt) override;
  PdbErr_t GetGroupArray(BlockValues& blockValues, std::vector<uint64_t>& groupIdVec, bool& singleGroup) override;
};

class QueryGroupTstamp : public QueryGroup
{
public:
  QueryGroupTstamp();
  virtual ~QueryGroupTstamp();

  bool IsEmptySet() const override;
  void GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const override;

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override;

  bool IsQueryLast() const override { return false; }
  bool IsQueryFirst() const override { return false; }

protected:
  int64_t GetGroupId(const DBVal* pVals, size_t valCnt) override;
  PdbErr_t GetGroupArray(BlockValues& blockValues, std::vector<uint64_t>& groupIdVec, bool& singleGroup) override;
  PdbErr_t CustomBuild(const QueryParam* pQueryParam) override;

protected:
  int64_t timeGroupRange_;
  int64_t minTstamp_;
  int64_t maxTstamp_;
};

