#pragma once

#include "internal.h"

#include "table/table_info.h"
#include "util/arena.h"

#include "expr/expr_item.h"
#include "expr/column_item.h"
#include "expr/parse.h"
#include "query/result_filter.h"



class PartItem
{
public:
  PartItem(const char* partDateStr, int partDate, int partType);
  PartItem(const PartItem& part);
  PartItem& operator=(const PartItem& part);
  ~PartItem(){ }

  std::string GetPartDateStr() const { return partDateStr_; }
  int GetPartDate() const { return partDate_; }
  int GetPartType() const { return partType_; }

private:
  std::string partDateStr_;
  int partDate_;
  int partType_;
};

class TableItem
{
public:
  TableItem(const char* pTabName);
  ~TableItem();

  void AddPartItem(const PartItem& partItem);
  void DelPartItem(int partDate);

  std::string GetTabName() const { return tabName_; }
  const std::vector<PartItem>& GetPartVec() const { return partVec_; }

private:
  TableItem(const TableItem&);
  TableItem& operator=(const TableItem&);

private:
  std::string tabName_;
  std::vector<PartItem> partVec_;
};

class TableConfig
{
public:
  TableConfig();
  ~TableConfig();

  bool LoadTableConfig();

  PdbErr_t AddTable(const char* pTabName);
  PdbErr_t DropTable(const char* pTabName);

  PdbErr_t AddFilePart(const char* pTabName, 
    const char* pPartDateStr, int partDate, int partType);
  PdbErr_t DelFilePart(const char* pTabName, int partDate);
  PdbErr_t SetFilePart(const char* pTabName,
    const char* pPartDateStr, int partDate, int partType);

  const std::vector<TableItem*>& GetTables();

  PdbErr_t QueryTable(IResultFilter* pFilter);
  PdbErr_t QueryPart(IResultFilter* pFilter);

private:
  bool _SaveConfig();

  TableConfig(const TableConfig&);
  TableConfig& operator=(const TableConfig&);

private:
  std::mutex tabMutex_;
  std::vector<TableItem*> tabVec_;
};
