#include "expr/parse.h"
#include "expr/expr_item.h"

ExprItem::ExprItem()
{
  op_ = 0;
  pLeft_ = nullptr;
  pRight_ = nullptr;
  pParentExpr_ = nullptr;
  pExprList_ = nullptr;
}
ExprItem::~ExprItem()
{
  if (pLeft_ != nullptr)
    delete pLeft_;

  if (pRight_ != nullptr)
    delete pRight_;

  if (pExprList_ != nullptr)
    delete pExprList_;
}

void ExprItem::SetExprList(ExprList* pExprList)
{
  pExprList_ = pExprList;
}

const std::string& ExprItem::GetAliasName() const
{
  return this->aliasName_;
}
void ExprItem::SetAliasName(Token* pToken)
{
  if (pToken != nullptr && pToken->str_ != nullptr && pToken->len_ > 0)
    this->aliasName_ = std::string(pToken->str_, pToken->len_);
  else
    this->aliasName_ = "";
}

const std::string& ExprItem::GetValueStr() const
{
  return this->valueStr_;
}

int ExprItem::GetOp() const
{
  return op_;
}
const ExprItem* ExprItem::GetLeftExpr() const
{
  return pLeft_;
}
const ExprItem* ExprItem::GetRightExpr() const
{
  return pRight_;
}
const ExprItem* ExprItem::GetParentExpr() const
{
  return pParentExpr_;
}
const ExprList* ExprItem::GetExprList() const
{
  return pExprList_;
}

ExprItem* ExprItem::MakeExpr(int op, ExprItem* pLeft, ExprItem* pRight, Token* pValStr)
{
  ExprItem* pNew = new ExprItem();
  pNew->op_ = op;
  pNew->pLeft_ = pLeft;
  pNew->pRight_ = pRight;
  if (pLeft != nullptr)
  {
    pLeft->pParentExpr_ = pNew;
  }
  if (pRight != nullptr)
  {
    pRight->pParentExpr_ = pNew;
  }

  if (pValStr != nullptr)
  {
    pNew->valueStr_ = std::string(pValStr->str_, pValStr->len_);
  }
  else
  {
    pNew->valueStr_ = "";
  }

  return pNew;
}

ExprItem* ExprItem::MakeFunction(int op, Token* pArgField, Token* pAsName)
{
  ExprItem* pNew = new ExprItem();
  pNew->op_ = op;
  pNew->pExprList_ = new ExprList();
  pNew->pExprList_->AddExprItem(ExprItem::MakeExpr(TK_ID, nullptr, nullptr, pArgField));

  if (pAsName != nullptr && pAsName->str_ != nullptr && pAsName->len_ > 0)
    pNew->aliasName_ = std::string(pAsName->str_, pAsName->len_);
  else
    pNew->aliasName_ = "";

  return pNew;
}

void ExprItem::FreeExprItem(ExprItem* pExprItem)
{
  if (pExprItem != nullptr)
  {
    delete pExprItem;
  }
}


ExprList::ExprList()
{

}
ExprList::~ExprList()
{
  for (auto iter = exprVec_.begin(); iter != exprVec_.end(); iter++)
  {
    delete *iter;
  }
}

ExprList* ExprList::AddExprItem(ExprItem* pExprItem)
{
  if (pExprItem != nullptr)
  {
    exprVec_.push_back(pExprItem);
  }

  return this;
}
const std::vector<ExprItem*>& ExprList::GetExprList() const
{
  return exprVec_;
}

ExprList* ExprList::AppendExprItem(ExprList* pExprList, ExprItem* pExprItem)
{
  if (pExprList == nullptr)
  {
    pExprList = new ExprList();
  }

  if (pExprItem != nullptr)
    pExprList->AddExprItem(pExprItem);

  return pExprList;
}
void ExprList::FreeExprList(ExprList* pExprList)
{
  if (pExprList != nullptr)
  {
    delete pExprList;
  }
}

