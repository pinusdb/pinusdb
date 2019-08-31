#pragma once

#include "internal.h"
#include "expr/expr_item.h"

class DevFilter
{
public:
  DevFilter();
  ~DevFilter();

  PdbErr_t BuildFilter(const ExprItem* pCondition);
  bool IsEmptySet() const;
  bool HaveEqualObjId() const;
  int64_t GetEqualObjId() const;
  bool Filter(int64_t devId) const;
  int64_t GetMinDevId() const { return minDevId_; }
  int64_t GetMaxDevId() const { return maxDevId_; }

private:
  PdbErr_t _BuildFilter(const ExprItem* pExpr);

private:
  bool emptySet_;
  bool includeMin_;
  bool includeMax_;

  int64_t minDevId_;
  int64_t maxDevId_;
};
