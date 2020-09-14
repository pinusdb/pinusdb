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

#include "internal.h"
#include <unordered_set>
#include "table/db_value.h"
#include "table/table_info.h"
#include "query/value_item.h"

//////////////////////////////////////////////////////////////////////////////
class ConditionFilter
{
public:
  ConditionFilter();
  ~ConditionFilter();

  PdbErr_t BuildCondition(const TableInfo* pTabInfo, 
    const ExprValue* pCondition, int64_t nowMicroseconds);
  PdbErr_t RunCondition(const DBVal* pVals, size_t valCnt, bool& resultVal) const;
  PdbErr_t RunConditionArray(BlockValues& blkValues) const;
  bool AlwaysFalse() const { return alwaysFalse_ || minDevId_ > maxDevId_ || minTstamp_ > maxTstamp_; }
  void GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const;
  void GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const;
  bool FilterDevId(int64_t devId) const;
  void GetUseFields(std::unordered_set<size_t>& fieldSet) const;

private:
  bool alwaysFalse_;
  int64_t minDevId_;
  int64_t maxDevId_;
  int64_t minTstamp_;
  int64_t maxTstamp_;
  std::vector<ValueItem*> conditionVec_;
  std::vector<ValueItem*> devIdCondiVec_;
};
