
#pragma once

#include "expr/pdb_db_int.h"
#include "table/db_value.h"
#include <string>
#include <vector>

class ExprValue;
typedef std::pair<ExprValue*, std::string> TargetItem;

class TargetList
{
public:
  TargetList();
  ~TargetList();

  const std::vector<TargetItem>* GetTargetList() const
  {
    return &targetVec_;
  }

  static TargetList* AppendExprValue(TargetList* pList, ExprValue* pValue, Token* pAliasName);
  static void FreeTargetList(TargetList* pList);

private:
  int fieldNo_;
  std::vector<TargetItem> targetVec_;
};
