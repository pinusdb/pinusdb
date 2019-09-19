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

#include "pdb.h"
#include "pdb_error.h"
#include "table/table_info.h"
#include "util/string_tool.h"

TableInfo::TableInfo()
{
  metaCode64_ = 0;
}

TableInfo::~TableInfo()
{
}

PdbErr_t TableInfo::ValidTableName(const char* pTabName, size_t nameLen)
{
  if (nameLen <= 0 || nameLen >= PDB_TABLE_NAME_LEN)
    return PdbE_INVALID_TABLE_NAME;

  if (!StringTool::ValidName(pTabName, nameLen))
    return PdbE_INVALID_TABLE_NAME;

  return PdbE_OK;
}

PdbErr_t TableInfo::SetTableName(const char* pTabName)
{
  PdbErr_t retVal = PdbE_OK;
  retVal = ValidTableName(pTabName, strlen(pTabName));
  if (retVal != PdbE_OK)
    return retVal;

  this->tabName_ = std::string(pTabName);
  return PdbE_OK;
}

const char* TableInfo::GetTableName() const
{
  return this->tabName_.c_str();
}

PdbErr_t TableInfo::AddField(const char* pFieldName, int32_t fieldType)
{
  return AddField(pFieldName, fieldType, false);
}

PdbErr_t TableInfo::AddField(const char* pFieldName, int32_t fieldType, bool isKey)
{
  PdbErr_t retVal = PdbE_OK;

  FieldInfo finfo;
  retVal = finfo.SetFieldInfo(pFieldName, fieldType, isKey);
  if (retVal != PdbE_OK)
    return retVal;

  uint64_t nameCrc = StringTool::CRC64NoCase(finfo.GetFieldName());
  if (fieldMap_.find(nameCrc) != fieldMap_.end())
    return PdbE_FIELD_NAME_EXIST;

  fieldMap_.insert(std::pair<uint64_t, size_t>(nameCrc, fieldVec_.size()));
  fieldVec_.push_back(finfo);

  metaCode64_ = StringTool::CRC64NoCase(pFieldName, strlen(pFieldName), 0, metaCode64_);
  metaCode64_ = StringTool::CRC64(&fieldType, sizeof(fieldType), 0, metaCode64_);
  return PdbE_OK;
}


//一个存储数据的表
PdbErr_t TableInfo::ValidStorageTable() const
{
  PdbErr_t retVal = PdbE_OK;

  retVal = ValidTableName(tabName_.c_str(), tabName_.size());
  if (retVal != PdbE_OK)
    return retVal;

  if (fieldVec_.size() <= 2)
  {
    return PdbE_TABLE_FIELD_TOO_LESS;
  }

  if (fieldVec_.size() > PDB_TABLE_MAX_FIELD_COUNT)
  {
    return PdbE_TABLE_FIELD_TOO_MANY;
  }

  if (fieldVec_[PDB_DEVID_INDEX].GetFieldType() != PDB_FIELD_TYPE::TYPE_INT64)
    return PdbE_INVALID_DEVID_FIELD;
  if (!StringTool::ComparyNoCase(fieldVec_[PDB_DEVID_INDEX].GetFieldName(), DEVID_FIELD_NAME))
    return PdbE_INVALID_DEVID_FIELD;

  if (fieldVec_[PDB_TSTAMP_INDEX].GetFieldType() != PDB_FIELD_TYPE::TYPE_DATETIME)
    return PdbE_INVALID_TSTAMP_FIELD;
  if (!StringTool::ComparyNoCase(fieldVec_[PDB_TSTAMP_INDEX].GetFieldName(), TSTAMP_FIELD_NAME))
    return PdbE_INVALID_TSTAMP_FIELD;

  return PdbE_OK;
}

PdbErr_t TableInfo::GetFieldInfo(const char* pFieldName, size_t* pFieldPos, int32_t* pFieldType) const
{
  uint64_t nameCrc = StringTool::CRC64NoCase(pFieldName);
  return GetFieldInfo(nameCrc, pFieldPos, pFieldType);
}
PdbErr_t TableInfo::GetFieldInfo(size_t fieldPos, int32_t* pFieldType) const
{
  if (fieldPos < fieldVec_.size())
  {
    if (pFieldType != nullptr)
      *pFieldType = fieldVec_[fieldPos].GetFieldType();

    return PdbE_OK;
  }

  return PdbE_FIELD_NOT_FOUND;
}

PdbErr_t TableInfo::GetFieldIsKey(size_t fieldPos, bool* pIsKey) const
{
  if (fieldPos < fieldVec_.size())
  {
    if (pIsKey != nullptr)
      *pIsKey = fieldVec_[fieldPos].GetFieldIsKey();

    return PdbE_OK;
  }

  return PdbE_FIELD_NOT_FOUND;
}

PdbErr_t TableInfo::GetFieldInfo(uint64_t fieldCrc, size_t* pFieldPos, int32_t* pFieldType) const
{
  auto fieldIter = fieldMap_.find(fieldCrc);
  if (fieldIter != fieldMap_.end())
  {
    if (pFieldPos != nullptr)
      *pFieldPos = fieldIter->second;

    if (pFieldType != nullptr)
      *pFieldType = fieldVec_[fieldIter->second].GetFieldType();

    return PdbE_OK;
  }

  return PdbE_FIELD_NOT_FOUND;
}
const char* TableInfo::GetFieldName(size_t fieldPos) const
{
  if (fieldPos < fieldVec_.size())
  {
    return fieldVec_[fieldPos].GetFieldName();
  }

  return nullptr;
}

size_t TableInfo::GetFieldCnt() const
{
  return fieldVec_.size();
}

uint32_t TableInfo::GetMetaCode() const
{
  return CRC64_TO_CRC32(metaCode64_);
}
