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

#include "expr/sql_parser.h"
#include "table/data_column.h"
#include "query/group_field.h"
#include <string>
#include <vector>

class ResultObject
{
public:
  ResultObject(const std::vector<GroupField*>& fieldVec,
    int64_t devId, int64_t tstamp);
  virtual ~ResultObject();

  PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt);
  PdbErr_t AppendArray(BlockValues& blockValues, uint64_t groupId, const std::vector<size_t>& groupIdVec);
  PdbErr_t GetRecord(DBVal* pVals, size_t valCnt);

private:
  std::vector<GroupField*> fieldVec_;
};

