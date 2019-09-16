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

#include "query/result_filter.h"

class ISnapshotResultFilter : public IResultFilter
{
public:
  ISnapshotResultFilter() {}
  virtual ~ISnapshotResultFilter() {}

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded) = 0;
  virtual PdbErr_t BuildCustomFilter(const QueryParam* pQueryParam, const TableInfo* pTabInfo, Arena* pArena) = 0;
  virtual bool GetIsFullFlag() const = 0;
  virtual int64_t GetResultMaxDevId() { return INT64_MAX; }
  virtual void CleanUpResult() { };

};
