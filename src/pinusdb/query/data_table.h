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
#include "table/data_column.h"
#include "table/db_obj.h"
#include "table/table_info.h"
#include "util/arena.h"
#include <string>
#include <vector>

class DataTable
{
public:
  DataTable();
  ~DataTable();

  PdbErr_t AddColumn(const std::string& colName, int32_t type);

  size_t GetColumnCnt() const;
  PdbErr_t GetColumnInfo(size_t colPos, std::string& colName, int32_t* pColType) const;

  PdbErr_t ClearData();
  PdbErr_t AppendData(DBObj* pObj);

  Arena* GetArena();
  size_t GetRecordCnt() const;
  
  PdbErr_t GetSerializeLen(size_t* pDataLen);
  PdbErr_t Serialize(uint8_t* pData, size_t *pBodyLen);

private:
  PdbErr_t HeadSerialize(uint8_t* pData, size_t* pDataLen);

private:
  Arena arena_;
  TableInfo tabInfo_;
  std::vector<DBObj*> dataVec_;
};

