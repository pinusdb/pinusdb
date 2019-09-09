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
#include <stdint.h>
#include "expr/pdb_db_int.h"
#include "pdb.h"

class LimitOpt
{
public:
  LimitOpt(Token* pQueryCnt);
  LimitOpt(Token* pOffset, Token* pQueryCnt);
  ~LimitOpt();

  PdbErr_t Valid() const;

  size_t GetOffset() const;
  size_t GetQueryCnt() const;

private:
  int64_t offsetCnt_;
  int64_t queryCnt_;
};
