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
#include "util/coding.h"
#include <string.h>

TableInfo::TableInfo()
{
  metaCode64_ = 0;
  fixedSize_ = 0;
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

  if (fieldVec_.size() >= 2)
  {
    if (fieldVec_.size() == 2)
    {
      bool storageTable = false;

      do {
        if (fieldVec_[PDB_DEVID_INDEX].GetFieldType() != PDB_FIELD_TYPE::TYPE_INT64)
          break;
        if (!StringTool::ComparyNoCase(fieldVec_[PDB_DEVID_INDEX].GetFieldName(), DEVID_FIELD_NAME))
          break;

        if (fieldVec_[PDB_TSTAMP_INDEX].GetFieldType() != PDB_FIELD_TYPE::TYPE_DATETIME)
          break;
        if (!StringTool::ComparyNoCase(fieldVec_[PDB_TSTAMP_INDEX].GetFieldName(), TSTAMP_FIELD_NAME))
          break;

        storageTable = true;
      } while (false);

      if (storageTable)
      {
        storePosVec_.push_back(0); //devid
        storePosVec_.push_back(2); //tstamp 
        fixedSize_ = 10; // tstamp length
      }
    }
    else if (storePosVec_.size() > 0)
    {
      storePosVec_.push_back(fixedSize_);
      switch (fieldType)
      {
      case PDB_FIELD_TYPE::TYPE_BOOL: fixedSize_ += 1; break;
      case PDB_FIELD_TYPE::TYPE_INT8: fixedSize_ += 1; break;
      case PDB_FIELD_TYPE::TYPE_INT16: fixedSize_ += 2; break;
      case PDB_FIELD_TYPE::TYPE_INT32: fixedSize_ += 4; break;
      case PDB_FIELD_TYPE::TYPE_INT64: fixedSize_ += 8; break;
      case PDB_FIELD_TYPE::TYPE_DATETIME: fixedSize_ += 8; break;
      case PDB_FIELD_TYPE::TYPE_FLOAT: fixedSize_ += 4; break;
      case PDB_FIELD_TYPE::TYPE_DOUBLE: fixedSize_ += 8; break;
      case PDB_FIELD_TYPE::TYPE_STRING: fixedSize_ += 2; break;
      case PDB_FIELD_TYPE::TYPE_BLOB: fixedSize_ += 2; break;
      case PDB_FIELD_TYPE::TYPE_REAL2: fixedSize_ += 8; break;
      case PDB_FIELD_TYPE::TYPE_REAL3: fixedSize_ += 8; break;
      case PDB_FIELD_TYPE::TYPE_REAL4: fixedSize_ += 8; break;
      case PDB_FIELD_TYPE::TYPE_REAL6: fixedSize_ += 8; break;
      }
    }
  }

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

  for (size_t idx = (PDB_TSTAMP_INDEX + 1); idx < fieldVec_.size(); idx++)
  {
    const char* pFieldName = fieldVec_[idx].GetFieldName();
    size_t nameLen = strlen(pFieldName);
    retVal = FieldInfo::ValidFieldName(pFieldName, nameLen);
    if (retVal != PdbE_OK)
      return retVal;
  }

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
    {
      *pFieldType = fieldVec_[fieldPos].GetFieldType();
      if (PDB_TYPE_IS_REAL(*pFieldType))
      {
        *pFieldType = PDB_FIELD_TYPE::TYPE_DOUBLE;
      }
    }

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
    {
      *pFieldType = fieldVec_[fieldIter->second].GetFieldType();
      if (PDB_TYPE_IS_REAL(*pFieldType))
      {
        *pFieldType = PDB_FIELD_TYPE::TYPE_DOUBLE;
      }
    }

    return PdbE_OK;
  }

  return PdbE_FIELD_NOT_FOUND;
}

PdbErr_t TableInfo::GetFieldRealInfo(const char* pFieldName, size_t* pFieldPos, int32_t* pFieldType, size_t* pStorePos) const
{
  uint64_t nameCrc = StringTool::CRC64NoCase(pFieldName);
  return GetFieldRealInfo(nameCrc, pFieldPos, pFieldType, pStorePos);
}

PdbErr_t TableInfo::GetFieldRealInfo(size_t fieldPos, int32_t* pFieldType, size_t* pStorePos) const
{
  if (fieldPos < fieldVec_.size())
  {
    if (pFieldType != nullptr)
    {
      *pFieldType = fieldVec_[fieldPos].GetFieldType();
    }

    if (pStorePos != nullptr)
    {
      if (storePosVec_.size() == fieldVec_.size())
      {
        *pStorePos = storePosVec_[fieldPos];
      }
      else
      {
        *pStorePos = 0;
      }
    }

    return PdbE_OK;
  }

  return PdbE_FIELD_NOT_FOUND;
}

PdbErr_t TableInfo::GetFieldRealInfo(uint64_t fieldCrc, size_t* pFieldPos, int32_t* pFieldType, size_t* pStorePos) const
{
  auto fieldIter = fieldMap_.find(fieldCrc);
  if (fieldIter != fieldMap_.end())
  {
    if (pFieldPos != nullptr)
      *pFieldPos = fieldIter->second;

    if (pFieldType != nullptr)
    {
      *pFieldType = fieldVec_[fieldIter->second].GetFieldType();
    }

    if (pStorePos != nullptr)
    {
      if (storePosVec_.size() == fieldVec_.size())
      {
        *pStorePos = storePosVec_[fieldIter->second];
      }
      else
      {
        *pStorePos = 0;
      }
    }

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

void TableInfo::Serialize(std::string& dataBuf) const
{
  std::string fullName;
  for (auto fieldIt = fieldVec_.begin(); fieldIt != fieldVec_.end(); fieldIt++)
  {
    fullName.clear();
    switch (fieldIt->GetFieldType())
    {
    case PDB_FIELD_TYPE::TYPE_BOOL: fullName.append("bool;"); break;
    case PDB_FIELD_TYPE::TYPE_INT8: fullName.append("tinyint;"); break;
    case PDB_FIELD_TYPE::TYPE_INT16: fullName.append("smallint;"); break;
    case PDB_FIELD_TYPE::TYPE_INT32: fullName.append("int;"); break;
    case PDB_FIELD_TYPE::TYPE_INT64: fullName.append("bigint;"); break;
    case PDB_FIELD_TYPE::TYPE_DATETIME: fullName.append("datetime;"); break;
    case PDB_FIELD_TYPE::TYPE_FLOAT: fullName.append("float;"); break;
    case PDB_FIELD_TYPE::TYPE_DOUBLE: fullName.append("double;"); break;
    case PDB_FIELD_TYPE::TYPE_STRING: fullName.append("string;"); break;
    case PDB_FIELD_TYPE::TYPE_BLOB: fullName.append("blob;"); break;
    }

    fullName.append(fieldIt->GetFieldName());
    dataBuf.push_back(static_cast<char>(PDB_FIELD_TYPE::TYPE_STRING));
    Coding::PutVarint64(&dataBuf, fullName.size());
    dataBuf.append(fullName);
  }
}
