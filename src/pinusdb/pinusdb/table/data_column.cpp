#include "table/data_column.h"
#include "util/string_tool.h"
#include "internal.h"

DataColumn::DataColumn()
{
  memset(colName_, 0, sizeof(colName_));

  colType_ = 0;

  isObjNameField_ = false;
  isTimeStampField_ = false;
}
DataColumn::~DataColumn()
{

}

bool DataColumn::SetColumnInfo(const char* pColName, size_t nameLen, int32_t type)
{
  if (!PDB_TYPE_IS_VALID(type))
  {
    return false;
  }

  colType_ = type;

  if (!StringTool::ValidColumnName(pColName, nameLen))
    return false;

  memcpy(colName_, pColName, nameLen);
  colName_[nameLen] = '\0';

  return true;
}

const char* DataColumn::GetColName() const
{
  return colName_;
}

int32_t DataColumn::GetColType() const
{
  return colType_;
}

bool DataColumn::IsObjNameField() const
{
  return isObjNameField_;
}

bool DataColumn::IsTimeStampField() const
{
  return isTimeStampField_;
}

