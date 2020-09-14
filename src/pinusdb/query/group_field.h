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

#include "internal.h"
#include "table/db_value.h"
#include "query/block_values.h"

class GroupField
{
public:
  GroupField() {}
  virtual ~GroupField() {}

  virtual int32_t FieldType() = 0;
  virtual PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt) = 0;
  virtual PdbErr_t AppendArray(BlockValues& blockValues,
    uint64_t groupId, const std::vector<size_t>& groupIdVec) = 0;
  virtual PdbErr_t GetResult(DBVal* pVal) = 0;

  virtual void GetUseFields(std::unordered_set<size_t>& fieldSet) const = 0;
  virtual GroupField* NewField(int64_t devId, int64_t tstamp) = 0;

  virtual bool IsLastFunc() { return false; }
  virtual bool IsFirstFunc() { return false; }
};


