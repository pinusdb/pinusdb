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

class DataColumn
{
public:
  DataColumn();
  ~DataColumn();

  bool SetColumnInfo(const char* pColName, size_t nameLen, int32_t type);

  const char* GetColName() const;

  int32_t GetColType() const;

  bool IsObjNameField() const;
  bool IsTimeStampField() const;

private:
  // no copy
  DataColumn(const DataColumn&);
  const DataColumn& operator=(const DataColumn&);

private:
  char colName_[PDB_FILED_NAME_LEN];
  int32_t colType_;

  bool isObjNameField_;
  bool isTimeStampField_;
};

