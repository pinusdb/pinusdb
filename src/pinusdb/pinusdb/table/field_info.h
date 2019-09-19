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

#include <stdint.h>
#include <string>
#include "internal.h"

class FieldInfo
{
public:
  FieldInfo();
  ~FieldInfo();

  static PdbErr_t ValidFieldName(const char* pName, size_t nameLen);

  FieldInfo(const FieldInfo& cpy);
  void operator=(const FieldInfo& cpy);

  PdbErr_t SetFieldInfo(const char* pName, int32_t fieldType, bool isKey);
  const char* GetFieldName() const { return fieldName_.c_str(); }
  int32_t GetFieldType() const { return fieldType_; }
  bool GetFieldIsKey() const { return isKey_; }
  uint64_t GetFieldNameCrc() const { return fieldNameCrc_; }

private:
  bool isKey_;
  int32_t fieldType_;
  uint64_t fieldNameCrc_;
  std::string fieldName_;
};
