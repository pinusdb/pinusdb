/*
* Copyright (c) 2019 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; If not, see <http://www.gnu.org/licenses>
*/

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
  bool AlwaysFalse() const { return alwaysFalse_; }

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
  bool alwaysFalse_;
  std::vector<ConditionItem*> conditionVec_;
};
