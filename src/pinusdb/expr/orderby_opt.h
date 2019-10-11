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
#include "internal.h"
#include "expr/pdb_db_int.h"
#include "util/string_tool.h"

class OrderByOpt
{
public:
  OrderByOpt(Token* pToken, bool isAsc)
  {
    valid_ = StringTool::ComparyNoCase(pToken->str_, (size_t)pToken->len_,
      TSTAMP_FIELD_NAME, (sizeof(TSTAMP_FIELD_NAME) - 1));
    isAsc_ = isAsc;
  }

  ~OrderByOpt() { }

  bool Valid() const { return valid_; }
  bool IsASC() const { return isAsc_; }

private:
  bool valid_;
  bool isAsc_;
};


