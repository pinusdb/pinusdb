#pragma once

#include "internal.h"
#include "expr/expr_item.h"

class TStampFilter
{
public:
  TStampFilter();
  ~TStampFilter();

  bool BuildFilter(const ExprItem* pCondition);
  int64_t GetMinTstamp() const { return minTimeStamp_; }
  int64_t GetMaxTstamp() const { return maxTimeStamp_; }

private:
  bool _BuildFilter(const ExprItem* pExpr);

private:
  bool includeMinTimeStamp_;
  bool includeMaxTimeStamp_;
  int64_t minTimeStamp_;
  int64_t maxTimeStamp_;
};

