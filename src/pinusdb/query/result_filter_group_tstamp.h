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

#include "pdb.h"
#include "query/result_filter.h"

class ResultFilterGroupTStamp : public IResultFilter
{
public:
  ResultFilterGroupTStamp();
  virtual ~ResultFilterGroupTStamp();

  virtual PdbErr_t InitGrpTsResult();
  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded);
  virtual PdbErr_t BuildCustomFilter(const QueryParam* pQueryParam, const TableInfo* pTabInfo, Arena* pArena);

  virtual bool GetIsFullFlag() const { return false; }
  virtual bool IsGroupByTstamp() const { return true; }

protected:
  int64_t timeGroupRange_;
  int64_t minTimeStamp_;
  int64_t maxTimeStamp_;

  uint64_t lastGroupId_;
  ResultObject* pLastObj_;
};


