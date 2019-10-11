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

#include "table/data_column.h"
#include "util/string_tool.h"
#include "internal.h"
#include <string.h>

DataColumn::DataColumn()
{
  memset(colName_, 0, sizeof(colName_));

  colType_ = 0;

  isObjNameField_ = false;
  isTimeStampField_ = false;
}
DataColumn::~DataColumn()
{

}

bool DataColumn::SetColumnInfo(const char* pColName, size_t nameLen, int32_t type)
{
  if (!PDB_TYPE_IS_VALID(type))
  {
    return false;
  }

  colType_ = type;

  if (!StringTool::ValidColumnName(pColName, nameLen))
    return false;

  memcpy(colName_, pColName, nameLen);
  colName_[nameLen] = '\0';

  return true;
}

const char* DataColumn::GetColName() const
{
  return colName_;
}

int32_t DataColumn::GetColType() const
{
  return colType_;
}

bool DataColumn::IsObjNameField() const
{
  return isObjNameField_;
}

bool DataColumn::IsTimeStampField() const
{
  return isTimeStampField_;
}

