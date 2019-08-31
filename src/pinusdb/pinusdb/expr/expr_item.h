#pragma once

#include "expr/pdb_db_int.h"
#include <string>
#include <vector>

class ExprItem;
class ExprList;

class ExprItem
{
public:
  ExprItem();
  ~ExprItem();

  void SetExprList(ExprList* pExprList);
  const std::string& GetAliasName() const;
  void SetAliasName(Token* pToken);

  const std::string& GetValueStr() const;

  int GetOp() const;
  const ExprItem* GetLeftExpr() const;
  const ExprItem* GetRightExpr() const;
  const ExprItem* GetParentExpr() const;
  const ExprList* GetExprList() const;

  static ExprItem* MakeExpr(int op, ExprItem* pLeft, ExprItem* pRight, Token* pValStr);
  static ExprItem* MakeFunction(int op, Token* pFieldName, Token* pAsName);
  static void FreeExprItem(ExprItem* pExprItem);

private:
  int op_;        // Operation performed by this node
  ExprItem* pLeft_;   // Left subnodes
  ExprItem* pRight_;  // Right subnodes
  ExprItem* pParentExpr_;  // Parent
  ExprList* pExprList_;    // A list of expressions used as function arguments
                           // or in "<expr> IN (<expr-list>)"
  std::string aliasName_;
  std::string valueStr_;
  //Token token_;
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

