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

class TStampFilter
{
public:
  TStampFilter();
  ~TStampFilter();

  bool BuildFilter(const ExprItem* pCondition);
  int64_t GetMinTstamp() const { return minTimeStamp_; }
  int64_t GetMaxTstamp() const { return maxTimeStamp_; }

private:
  bool _BuildFilter(const ExprItem* pExpr);

private:
  int64_t minTimeStamp_;
  int64_t maxTimeStamp_;
};

