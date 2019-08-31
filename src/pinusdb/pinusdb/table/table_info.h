#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "table/data_column.h"
#include "table/field_info.h"
#include "internal.h"

/////////////////////////////////////////////////

class TableInfo
{
public:
  TableInfo();
  ~TableInfo();

  static PdbErr_t ValidTableName(const char* pTabName, size_t nameLen);

  PdbErr_t SetTableName(const char* pTabName, size_t nameLen);
  const char* GetTableName() const;

  PdbErr_t AddField(const char* pFieldName, int32_t fieldType);
  PdbErr_t AddField(const char* pFieldName, int32_t fieldType, bool isKey);

  PdbErr_t ValidInfo() const;

  //一个存储数据的表
  PdbErr_t ValidStorageTable() const;

  PdbErr_t GetFieldInfo(const char* pFieldName, size_t* pFieldPos, int32_t* pFieldType) const;
  PdbErr_t GetFieldInfo(size_t fieldPos, int32_t* pFieldType) const;
  PdbErr_t GetFieldIsKey(size_t fieldPos, bool* pIsKey) const;
  PdbErr_t GetFieldInfo(uint64_t fieldCrc, size_t* pFieldPos, int32_t* pFieldType) const;
  const char* GetFieldName(size_t fieldPos) const;

  size_t GetFieldCnt() const;
  uint32_t GetFieldCrc() const;

private:
  std::string tabName_;

  std::vector<FieldInfo> fieldVec_;
  std::unordered_map<uint64_t, size_t> fieldMap_;

};

