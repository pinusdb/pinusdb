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
#include "query/result_filter.h"
#include "query/dev_filter.h"
#include "query/tstamp_filter.h"

class ITablePart
{
public:
  ITablePart() {}
  virtual ~ITablePart() {}

  virtual PdbErr_t QueryDataASC(const DevFilter* pDevFilter, const TStampFilter* pTSFilter, 
    IResultFilter* pResult, uint64_t timeOut) = 0;
  virtual PdbErr_t QueryDataDESC(const DevFilter* pDevFilter, const TStampFilter* pTSFilter, 
    IResultFilter* pResult, uint64_t timeOut) = 0;
  virtual PdbErr_t QueryLast(int64_t devId, int64_t maxTs, 
    IResultFilter* pResult, uint64_t timeOut, bool& isAdded) = 0;
  virtual PdbErr_t QueryFirst(int64_t devId, int64_t minTs, 
    IResultFilter* pResult, uint64_t timeOut, bool& isAdded) = 0;
};
