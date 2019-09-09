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
#include "expr/pdb_db_int.h"

class ColumnItem;
class ColumnList;

class ColumnItem
{
public:
  ColumnItem(const Token* pToken, int32_t type);
  ~ColumnItem();

  const char* GetName() const;
  int32_t GetNameLen() const;

  int32_t GetType() const;

  static ColumnItem* MakeColumnItem(const Token* pToken, int32_t type);
  static void FreeColumnItem(ColumnItem* pColItem);

private:
  std::string name_;
  int32_t type_;
};

class ColumnList
{
public:
  ColumnList();
  ~ColumnList();

  ColumnList* AddColumnItem(ColumnItem* pColItem);
  const std::vector<ColumnItem*>& GetColumnList() const;

  static ColumnList* AppendColumnItem(ColumnList* pColList, ColumnItem* pColItem);
  static void FreeColumnList(ColumnList* pColList);

private:
  std::vector<ColumnItem*> colItemVec_;
};

