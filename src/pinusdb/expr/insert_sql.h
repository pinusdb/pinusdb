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
#include "table/db_value.h"
#include "table/table_info.h"

class InsertSql
{
public:
  InsertSql();
  ~InsertSql();

  void SetTableName(const char* pName, size_t len);
  void SetTableName(const std::string& tabName);
  void SetFieldCnt(size_t fieldCnt) { fieldCnt_ = fieldCnt; }
  void SetRecCnt(size_t recCnt) { recCnt_ = recCnt; }
  void AppendFieldName(const char* pName, size_t len);
  void AppendFieldName(const std::string& fieldName);
  PdbErr_t AppendVal(const DBVal* pVal);
  bool Valid() const;

  const std::string& GetTableName() const;
  PdbErr_t InitTableInfo(const TableInfo* pTabInfo);

  bool IsEnd() const;
  PdbErr_t GetNextRec(DBVal* pVals, size_t valCnt);
  PdbErr_t GetNextRecBinary(std::string& buf, int64_t& devId, int64_t& tstamp);

private:
  size_t fieldCnt_;
  size_t recCnt_;
  size_t fixedSize_;

  std::string tabName_;
  std::vector<std::string> colNameVec_;
  std::vector<size_t> fieldVec_;
  std::vector<size_t> storeVec_;
  std::vector<int32_t> typeVec_;

  std::list<DBVal> valList_;
  std::list<DBVal>::const_iterator valIter_;
};
