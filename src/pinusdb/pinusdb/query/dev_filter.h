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
#include "expr/expr_item.h"

class DevFilter
{
public:
  DevFilter();
  ~DevFilter();

  PdbErr_t BuildFilter(const ExprItem* pCondition);
  bool IsEmptySet() const;
  bool HaveEqualObjId() const;
  int64_t GetEqualObjId() const;
  bool Filter(int64_t devId) const;
  int64_t GetMinDevId() const { return minDevId_; }
  int64_t GetMaxDevId() const { return maxDevId_; }

private:
  PdbErr_t _BuildFilter(const ExprItem* pExpr);

private:
  bool emptySet_;
  bool includeMin_;
  bool includeMax_;

  int64_t minDevId_;
  int64_t maxDevId_;
};
