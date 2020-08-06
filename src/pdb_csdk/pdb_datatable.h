#pragma once

#include "pdb.h"
#include "db_obj.h"
#include "table/table_info.h"
#include "util/string_tool.h"
#include "util/arena.h"

class PDBDataTable
{
public:
  PDBDataTable();
  ~PDBDataTable();

  PdbErr_t AddColumn(const char* pColName, size_t colNameLen, uint32_t type);

  size_t GetColumnCount() const;
  PdbErr_t GetColumnInfo(size_t idx, ColumnInfo* pColInfo) const;

  PdbErr_t GetColumnPos(const char* pName, size_t* pPos) const;

  size_t GetRows() const;
  PdbErr_t AddRow(DBObj* pObj);

  const DBObj* GetData(size_t idx);

private:
  Arena arena_;

  TableInfo tabInfo_;

  std::vector<DBObj*> dataVec_;

};

