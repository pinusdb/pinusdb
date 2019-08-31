#pragma once

#include "internal.h"
#include "table/db_obj.h"
#include "table/db_value.h"
#include "expr/expr_item.h"
#include "query/condition_item.h"

//////////////////////////////////////////////////////////////////////////////
class ConditionFilter
{
public:
  ConditionFilter();
  ~ConditionFilter();

  PdbErr_t BuildCondition(const ExprItem* pExpr, const TableInfo* pTabInfo);
  PdbErr_t RunCondition(const DBVal* pVals, size_t valCnt, bool& resultVal) const;

  bool IsNullCondition() const;

private:
  PdbErr_t _BuildCondition(const ExprItem* pExpr, const TableInfo* pTabInfo);

  ConditionItem* BuildLtCondition(size_t fieldPos, DBVal* pVal);

  ConditionItem* BuildLeCondition(size_t fieldPos, DBVal* pVal);

  ConditionItem* BuildGtCondition(size_t fieldPos, DBVal* pVal);

  ConditionItem* BuildGeCondition(size_t fieldPos, DBVal* pVal);

  ConditionItem* BuildNeCondition(size_t fieldPos, DBVal* pVal);

  ConditionItem* BuildEqCondition(size_t fieldPos, DBVal* pVal);

  ConditionItem* BuildLikeCondition(size_t fieldPos, DBVal* pVal);

private:
  std::vector<ConditionItem*> conditionVec_;
};
