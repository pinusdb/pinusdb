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
#include "expr/pdb_db_int.h"
#include "expr/expr_value.h"
#include <string>
#include <vector>

class GroupOpt
{
public:
  GroupOpt(Token* pToken1);
  GroupOpt(Token* pToken1, ExprValue* pTimeVal);
  ~GroupOpt();

  bool Valid() const;
  bool IsTableNameGroup() const;
  bool IsDevIdGroup() const;
  bool IsTStampGroup() const;
  PdbErr_t GetTStampStep(int64_t& timeStampStep) const;

  static GroupOpt* MakeGroupOpt(Token* pToken1);
  static GroupOpt* MakeGroupOpt(Token* pToken1, ExprValue* pTimeVal);
  static void FreeGroupOpt(GroupOpt* pGroupOpt);

private:
  bool valid_;
  bool isTableName_;
  bool isDevId_;
  bool isTStamp_;
  int64_t tVal_;

};
