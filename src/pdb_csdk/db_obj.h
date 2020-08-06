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
#include "util/arena.h"

class DBObj
{
public:
  DBObj(Arena* pArena) : pArena_(pArena) {}
  ~DBObj() {}

  void Clear();

  size_t GetFieldCnt() const;
  const DBVal* GetFieldValue(size_t idx) const;

  void AppendNullVal();
  void AppendVal(bool val);
  void AppendVal(int64_t val);
  void AppendVal(double val);
  PdbErr_t AppendStrVal(const char* pVal, size_t len);
  PdbErr_t AppendBlobVal(const uint8_t* pVal, size_t len);
  void AppendDateTime(int64_t val);

  PdbErr_t AppendVal(const DBVal* pVal);

  PdbErr_t ParseTrans(size_t fieldCnt, const char* pData, const char* pLimit, size_t* pObjLen);

private:
  Arena* pArena_;
  std::vector<DBVal> fieldVec_;
};
