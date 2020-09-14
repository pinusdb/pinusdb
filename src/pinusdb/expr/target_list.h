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

#include "expr/pdb_db_int.h"
#include "table/db_value.h"
#include <string>
#include <vector>

class ExprValue;
typedef std::pair<ExprValue*, std::string> TargetItem;

class TargetList
{
public:
  TargetList();
  ~TargetList();

  const std::vector<TargetItem>* GetTargetList() const
  {
    return &targetVec_;
  }

  static TargetList* AppendExprValue(TargetList* pList, ExprValue* pValue, Token* pAliasName);
  static void FreeTargetList(TargetList* pList);

private:
  int fieldNo_;
  std::vector<TargetItem> targetVec_;
};
