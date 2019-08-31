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

