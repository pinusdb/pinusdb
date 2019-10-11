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

#include "expr/parse.h"
#include "expr/expr_item.h"
#include "util/string_tool.h"
#include "util/date_time.h"

ExprItem::ExprItem()
{
  op_ = 0;
  pLeft_ = nullptr;
  pRight_ = nullptr;
  pExprList_ = nullptr;
  tkVal_.str_ = nullptr;
  tkVal_.len_ = 0;
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

const std::string& ExprItem::GetAliasName() const
{
  return this->aliasName_;
}

std::string ExprItem::GetValueStr() const
{
  return std::string(tkVal_.str_, tkVal_.len_);
}

bool ExprItem::GetTimeVal(int64_t* pVal) const
{
  int64_t tmpVal = 0;
  if (op_ != TK_TIMEVAL)
    return false;

  if (pExprList_ == nullptr)
    return false;

  const std::vector<ExprItem*>& argItemVec = pExprList_->GetExprList();
  if (argItemVec.size() != 2)
    return false;

  if (argItemVec[0]->op_ != TK_INTEGER && argItemVec[0]->op_ != TK_UINTEGER)
    return false;

  if (!StringTool::StrToInt64(argItemVec[0]->tkVal_.str_, argItemVec[0]->tkVal_.len_, &tmpVal))
    return false;

  if (argItemVec[0]->op_ == TK_UINTEGER)
    tmpVal *= -1;

  if (argItemVec[1]->op_ != TK_ID)
    return false;

  if (argItemVec[1]->tkVal_.len_ != 1)
    return false;

  switch (argItemVec[1]->tkVal_.str_[0])
  {
  case 's':
  case 'S':
    tmpVal *= MillisPerSecond;
    break;
  case 'm':
  case 'M':
    tmpVal *= MillisPerMinute;
    break;
  case 'h':
  case 'H':
    tmpVal *= MillisPerHour;
    break;
  case 'd':
  case 'D':
    tmpVal *= MillisPerDay;
    break;
  default:
    return false;
  }

  if (pVal != nullptr)
    *pVal = tmpVal;

  return true;
}

bool ExprItem::GetNowFuncVal(int64_t* pVal) const
{
  // now(-5h, 'hour')    s,m,h,d  ,  s/second, m/minute, h/hour, d/day
  int64_t tmpVal = DateTime::NowMilliseconds();
  if (op_ != TK_FUNCTION)
    return false;

  if (!StringTool::ComparyNoCase(funcName_, PDB_SQL_FUNC_NOW_NAME, PDB_SQL_FUNC_NOW_NAME_LEN))
    return false;

  do {
    if (pExprList_ == nullptr)
      break;

    const std::vector<ExprItem*>& argVec = pExprList_->GetExprList();
    if (argVec.size() == 0)
      break;

    if (argVec.size() > 2)
      return false;

    if (argVec.size() == 2)
    {
      //处理第二个参数
      if (argVec[1]->op_ != TK_ID && argVec[1]->op_ != TK_STRING)
        return false;

      std::string tValStr(argVec[1]->tkVal_.str_, argVec[1]->tkVal_.len_);
      if (StringTool::ComparyNoCase(tValStr, "s", (sizeof("s") - 1))
        || StringTool::ComparyNoCase(tValStr, "second", (sizeof("second") - 1)))
      {
        tmpVal -= (tmpVal % MillisPerSecond);
      }
      else if (StringTool::ComparyNoCase(tValStr, "m", (sizeof("m") - 1))
        || StringTool::ComparyNoCase(tValStr, "minute", (sizeof("minute") - 1)))
      {
        tmpVal -= (tmpVal % MillisPerMinute);
      }
      else if (StringTool::ComparyNoCase(tValStr, "h", (sizeof("h") - 1))
        || StringTool::ComparyNoCase(tValStr, "hour", (sizeof("hour") - 1)))
      {
        tmpVal -= (tmpVal % MillisPerHour);
      }
      else if (StringTool::ComparyNoCase(tValStr, "d", (sizeof("d") - 1))
        || StringTool::ComparyNoCase(tValStr, "day", (sizeof("day") - 1)))
      {
        tmpVal -= (tmpVal % MillisPerDay);
        tmpVal += DateTime::GetSysTimeZone();
      }
      else
      {
        return false;
      }
    }

    int64_t timeOffset = 0;
    if (!argVec[0]->GetTimeVal(&timeOffset))
      return false;

    tmpVal += timeOffset;

  } while (false);

  if (pVal != nullptr)
    *pVal = tmpVal;

  return true;
}

bool ExprItem::GetIntVal(int64_t* pVal) const
{
  int64_t tmpVal = 0;
  if (op_ != TK_INTEGER && op_ != TK_UINTEGER)
    return false;

  if (!StringTool::StrToInt64(tkVal_.str_, tkVal_.len_, &tmpVal))
    return false;

  if (op_ == TK_UINTEGER)
    tmpVal *= -1;

  if (pVal != nullptr)
    *pVal = tmpVal;

  return true;
}

bool ExprItem::GetDoubleVal(double* pVal) const
{
  double tmpVal = 0;
  if (op_ != TK_DOUBLE && op_ != TK_UDOUBLE)
    return false;

  if (!StringTool::StrToDouble(tkVal_.str_, tkVal_.len_, &tmpVal))
    return false;

  if (op_ == TK_UDOUBLE)
    tmpVal *= -1;

  if (pVal != nullptr)
    *pVal = tmpVal;

  return true;
}

bool ExprItem::GetDBVal(DBVal* pVal) const
{
  int64_t i64Val = 0;
  double dVal = 0;
  DBVal tmpVal;

  switch (op_)
  {
  case TK_TRUE:
  {
    DBVAL_SET_BOOL(&tmpVal, true);
    break;
  }
  case TK_FALSE:
  {
    DBVAL_SET_BOOL(&tmpVal, false);
    break;
  }
  case TK_INTEGER:
  case TK_UINTEGER:
  {  
    if (!GetIntVal(&i64Val))
      return false;
    
    DBVAL_SET_INT64(&tmpVal, i64Val);
    break;
  }
  case TK_DOUBLE:
  case TK_UDOUBLE:
  {
    if (!GetDoubleVal(&dVal))
      return false;

    DBVAL_SET_DOUBLE(&tmpVal, dVal);
    break;
  }
  case TK_STRING:
  { 
    DBVAL_SET_STRING(&tmpVal, tkVal_.str_, tkVal_.len_);
    break;
  }
  case TK_BLOB:
  {
    DBVAL_SET_BLOB(&tmpVal, tkVal_.str_, tkVal_.len_);
    break;
  }
  case TK_FUNCTION:
  {
    if (!GetNowFuncVal(&i64Val))
      return false;

    DBVAL_SET_DATETIME(&tmpVal, i64Val);
    break;
  }
  default:
    return false;
  }

  if (pVal != nullptr)
    *pVal = tmpVal;

  return true;
}

const std::string& ExprItem::GetFuncName() const
{
  return this->funcName_;
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
const ExprList* ExprItem::GetExprList() const
{
  return pExprList_;
}

ExprItem* ExprItem::MakeCondition(int op, Token* pID, ExprItem* pRight)
{
  ExprItem* pNew = new ExprItem();
  pNew->op_ = op;
  pNew->pLeft_ = new ExprItem();
  pNew->pLeft_->op_ = TK_ID;
  pNew->pLeft_->tkVal_ = *pID;

  pNew->pRight_ = pRight;

  return pNew;
}

ExprItem* ExprItem::MakeCondition(int op, ExprItem* pLeft, ExprItem* pRight)
{
  ExprItem* pNew = new ExprItem();
  pNew->op_ = op;
  pNew->pLeft_ = pLeft;
  pNew->pRight_ = pRight;

  return pNew;
}

ExprItem* ExprItem::MakeFuncCondition(int op, Token* pID, ExprList* pArgs)
{
  ExprItem* pNew = new ExprItem();
  pNew->op_ = op;
  pNew->pLeft_ = new ExprItem();
  pNew->pLeft_->op_ = TK_ID;
  pNew->pLeft_->tkVal_ = *pID;

  pNew->pExprList_ = pArgs;
  return pNew;
}

ExprItem* ExprItem::MakeFunction(int op, Token* pFuncName, ExprList* pArgs, Token* pAsName)
{
  ExprItem* pNew = new ExprItem();
  pNew->op_ = op;
  pNew->pExprList_ = pArgs;

  if (pFuncName != nullptr)
    pNew->funcName_ = std::string(pFuncName->str_, pFuncName->len_);
  else
    pNew->funcName_ = "";

  if (pAsName != nullptr)
    pNew->aliasName_ = std::string(pAsName->str_, pAsName->len_);
  else
    pNew->aliasName_ = "";

  return pNew;
}

ExprItem* ExprItem::MakeTimeVal(bool nonnegative, Token* pVal, Token* pUnit)
{
  ExprItem* pNew = new ExprItem();
  pNew->op_ = TK_TIMEVAL;
  pNew->pExprList_ = new ExprList();

  if (pVal != nullptr)
  {
    ExprItem* pValItem = new ExprItem();
    pValItem->op_ = nonnegative ? TK_INTEGER : TK_UINTEGER;
    pValItem->tkVal_ = *pVal;

    pNew->pExprList_->AddExprItem(pValItem);
  }

  if (pUnit != nullptr)
  {
    ExprItem* pUnitItem = new ExprItem();
    pUnitItem->op_ = TK_ID;
    pUnitItem->tkVal_ = *pUnit;

    pNew->pExprList_->AddExprItem(pUnitItem);
  }

  return pNew;
}

ExprItem* ExprItem::MakeValue(int op, Token* pVal)
{
  ExprItem* pNew = new ExprItem();
  pNew->op_ = op;

  if (pVal != nullptr)
  {
    pNew->tkVal_ = *pVal;
  }

  return pNew;
}

ExprItem* ExprItem::MakeValue(int op, Token* pVal, Token* pAsName)
{
  ExprItem* pNew = new ExprItem();
  pNew->op_ = op;

  if (pVal != nullptr)
  {
    pNew->tkVal_ = *pVal;
  }

  if (pAsName != nullptr)
  {
    pNew->aliasName_ = std::string(pAsName->str_, pAsName->len_);
  }

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

bool ExprList::GetIntValList(std::list<int64_t>& valList) const
{
  int64_t tmpVal = 0;
  for (auto exprIt = exprVec_.begin(); exprIt != exprVec_.end(); exprIt++)
  {
    if (!(*exprIt)->GetIntVal(&tmpVal))
      return false;

    valList.push_back(tmpVal);
  }

  return true;
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

