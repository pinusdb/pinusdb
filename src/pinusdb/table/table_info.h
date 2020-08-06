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

#include <string>
#include <vector>
#include <unordered_map>
#include "table/data_column.h"
#include "table/field_info.h"
#include "internal.h"
#include "util/ref_util.h"

class TableInfo : public RefObj
{
public:
  TableInfo();
  ~TableInfo();

  static PdbErr_t ValidTableName(const char* pTabName, size_t nameLen);

  PdbErr_t SetTableName(const char* pTabName);
  const char* GetTableName() const;

  PdbErr_t AddField(const char* pFieldName, int32_t fieldType);
  PdbErr_t AddField(const char* pFieldName, int32_t fieldType, bool isKey);

  //一个存储数据的表
  PdbErr_t ValidStorageTable() const;

  PdbErr_t GetFieldInfo(const char* pFieldName, size_t* pFieldPos, int32_t* pFieldType) const;
  PdbErr_t GetFieldInfo(size_t fieldPos, int32_t* pFieldType) const;
  PdbErr_t GetFieldInfo(uint64_t fieldCrc, size_t* pFieldPos, int32_t* pFieldType) const;

  PdbErr_t GetFieldRealInfo(const char* pFieldName, size_t* pFieldPos, int32_t* pFieldType) const;
  PdbErr_t GetFieldRealInfo(size_t fieldPos, int32_t* pFieldType) const;
  PdbErr_t GetFieldRealInfo(uint64_t fieldCrc, size_t* pFieldPos, int32_t* pFieldType) const;

  PdbErr_t GetFieldIsKey(size_t fieldPos, bool* pIsKey) const;
  const char* GetFieldName(size_t fieldPos) const;

  size_t GetFieldCnt() const;
  uint32_t GetMetaCode() const;

  void Serialize(std::string& dataBuf) const;

private:
  std::string tabName_;
  uint64_t metaCode64_;
  std::vector<FieldInfo> fieldVec_;
  std::unordered_map<uint64_t, size_t> fieldMap_;
};

