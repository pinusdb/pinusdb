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

