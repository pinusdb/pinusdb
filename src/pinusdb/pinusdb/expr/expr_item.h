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

#include "expr/pdb_db_int.h"
#include "table/db_value.h"
#include <string>
#include <vector>

class ExprItem;
class ExprList;

class ExprItem
{
public:
  ExprItem();
  ~ExprItem();

  const std::string& GetAliasName() const;
  std::string GetValueStr() const;
  
  bool GetTimeVal(int64_t* pVal) const;
  bool GetNowFuncVal(int64_t* pVal) const;
  bool GetIntVal(int64_t* pVal) const;
  bool GetDoubleVal(double* pVal) const;
  bool GetDBVal(DBVal* pVal) const;

  const std::string& GetFuncName() const;

  int GetOp() const;
  const ExprItem* GetLeftExpr() const;
  const ExprItem* GetRightExpr() const;
  const ExprItem* GetParentExpr() const;
  const ExprList* GetExprList() const;

  static ExprItem* MakeCondition(int op, Token* pID, ExprItem* pRight);
  static ExprItem* MakeCondition(int op, ExprItem* pLeft, ExprItem* pRight);
  static ExprItem* MakeFunction(int op, Token* pFuncName, ExprList* pArgs, Token* pAsName);
  static ExprItem* MakeTimeVal(bool nonnegative, Token* pVal, Token* pUnit);
  static ExprItem* MakeValue(int op, Token* pVal);
  static ExprItem* MakeValue(int op, Token* pVal, Token* pAsName);
  static void FreeExprItem(ExprItem* pExprItem);

private:
  int op_;        // Operation performed by this node
  ExprItem* pLeft_;   // Left subnodes
  ExprItem* pRight_;  // Right subnodes
  ExprItem* pParentExpr_;  // Parent
  ExprList* pExprList_;    // A list of expressions used as function arguments
                           // or in "<expr> IN (<expr-list>)"
  std::string funcName_;
  std::string aliasName_;
  Token tkVal_;
};

class ExprList
{
public:
  ExprList();
  ~ExprList();

  ExprList* AddExprItem(ExprItem* pExprItem);
  const std::vector<ExprItem*>& GetExprList() const;

  static ExprList* AppendExprItem(ExprList* pExprList, ExprItem* pExprItem);
  static void FreeExprList(ExprList* pExprList);

private:
  std::vector<ExprItem*> exprVec_;
};

