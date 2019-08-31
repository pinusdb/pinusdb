#pragma once

#include "internal.h"
#include "expr/pdb_db_int.h"

class ColumnItem;
class ColumnList;

class ColumnItem
{
public:
  ColumnItem(const Token* pToken, int32_t type);
  ~ColumnItem();

  const char* GetName() const;
  int32_t GetNameLen() const;

  int32_t GetType() const;

  static ColumnItem* MakeColumnItem(const Token* pToken, int32_t type);
  static void FreeColumnItem(ColumnItem* pColItem);

private:
  std::string name_;
  int32_t type_;
};

class ColumnList
{
public:
  ColumnList();
  ~ColumnList();

  ColumnList* AddColumnItem(ColumnItem* pColItem);
  const std::vector<ColumnItem*>& GetColumnList() const;

  static ColumnList* AppendColumnItem(ColumnList* pColList, ColumnItem* pColItem);
  static void FreeColumnList(ColumnList* pColList);

private:
  std::vector<ColumnItem*> colItemVec_;
};

