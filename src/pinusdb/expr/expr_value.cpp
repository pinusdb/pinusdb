/*
* Copyright (c) 2020 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
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

#include "expr/expr_value.h"
#include "expr/parse.h"
#include "util/string_tool.h"
#include "util/date_time.h"

ExprValue::ExprValue()
{
  valType_ = 0;
  DBVAL_SET_NULL(&dbVal_);
  pArgList_ = nullptr;
  pLeftParam_ = nullptr;
  pRightParam_ = nullptr;
}

ExprValue::~ExprValue()
{
  if (pArgList_ != nullptr)
    delete pArgList_;

  if (pLeftParam_ != nullptr)
    delete pLeftParam_;

  if (pRightParam_ != nullptr)
    delete pRightParam_;
}

void ExprValue::SwitchStringToID()
{
  //if (this->valType_ == TK_STRING)
  //{
  //  this->valType_ = TK_ID;
  //}
}

ExprValue* ExprValue::MakeStarValue()
{
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_STAR;
  return pNew;
}

ExprValue* ExprValue::MakeID(Token* pToken)
{
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_ID;
  DBVAL_SET_STRING(&pNew->dbVal_, pToken->str_, pToken->len_);
  return pNew;
}

ExprValue* ExprValue::MakeBoolValue(bool value)
{
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = value ? TK_TRUE : TK_FALSE;
  DBVAL_SET_BOOL(&pNew->dbVal_, value);
  return pNew;
}

ExprValue* ExprValue::MakeIntValue(bool negative, Token* pToken)
{
  int64_t tmpVal = 0;
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_INTEGER;
  if (StringTool::StrToInt64(pToken->str_, pToken->len_, &tmpVal))
  {
    if (negative)
      tmpVal *= -1;

    DBVAL_SET_INT64(&pNew->dbVal_, tmpVal);
  }
  return pNew;
}

ExprValue* ExprValue::MakeDoubleValue(bool negative, Token* pToken)
{
  double tmpVal = 0;
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_DOUBLE;
  if (StringTool::StrToDouble(pToken->str_, pToken->len_, &tmpVal))
  {
    if (negative)
      tmpVal *= -1;

    DBVAL_SET_DOUBLE(&pNew->dbVal_, tmpVal);
  }
  return pNew;
}

ExprValue* ExprValue::MakeStringValue(Token* pToken)
{
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_STRING;
  DBVAL_SET_STRING(&pNew->dbVal_, pToken->str_, pToken->len_);
  return pNew;
}

ExprValue* ExprValue::MakeBlobValue(Token* pToken)
{
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_BLOB;
  DBVAL_SET_BLOB(&pNew->dbVal_, pToken->str_, pToken->len_);
  return pNew;
}

ExprValue* ExprValue::MakeTimeValue(bool negative, Token* pVal, Token* pUnit)
{
  int64_t tmpVal = 0;
  int64_t microsecond = 0;
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_TIMEVAL;

  do {
    if (!StringTool::StrToInt64(pVal->str_, pVal->len_, &tmpVal))
      break;

    if (negative)
      tmpVal *= -1;

    if (!DateTime::GetMicrosecondByTimeUnit(pUnit->str_, pUnit->len_, &microsecond))
    {
      DBVAL_SET_INT64(&pNew->dbVal_, (tmpVal * microsecond));
    }
  } while (false);

  return pNew;
}

ExprValue* ExprValue::MakeDateTime(Token* pToken)
{
  ExprValue* pNew = new ExprValue();
  DateTime dt;
  if (dt.Parse(pToken->str_, pToken->len_))
  {
    pNew->valType_ = TK_TIMEVAL;
    DBVAL_SET_DATETIME(&pNew->dbVal_, dt.TotalMicrosecond());
  }
  else
  {
    pNew->valType_ = TK_STRING;
    DBVAL_SET_STRING(&pNew->dbVal_, pToken->str_, pToken->len_);
  }
  return pNew;
}

ExprValue* ExprValue::MakeFunction(Token* pFuncName, ExprValueList* pArgList)
{
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_FUNCTION;
  DBVAL_SET_STRING((&pNew->dbVal_), pFuncName->str_, pFuncName->len_);
  pNew->pArgList_ = pArgList;
  return pNew;
}

ExprValue* ExprValue::MakeCompare(int op, ExprValue* pLeft, ExprValue* pRight)
{
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = op;
  pLeft->SwitchStringToID();
  pNew->pLeftParam_ = pLeft;
  pNew->pRightParam_ = pRight;
  return pNew;
}

ExprValue* ExprValue::MakeLike(Token* pFieldName, Token* pPattern)
{
  ExprValue* pPatVal = new ExprValue();
  pPatVal->valType_ = TK_STRING;
  DBVAL_SET_STRING(&(pPatVal->dbVal_), pPattern->str_, pPattern->len_);

  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_LIKE;
  DBVAL_SET_STRING(&(pNew->dbVal_), pFieldName->str_, pFieldName->len_);
  pNew->pLeftParam_ = pPatVal;

  return pNew;
}

ExprValue* ExprValue::MakeIsNotNull(Token* pFieldName)
{
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_ISNOTNULL;
  DBVAL_SET_STRING(&(pNew->dbVal_), pFieldName->str_, pFieldName->len_);
  return pNew;
}

ExprValue* ExprValue::MakeIsNull(Token* pFieldName)
{
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_ISNULL;
  DBVAL_SET_STRING(&(pNew->dbVal_), pFieldName->str_, pFieldName->len_);
  return pNew;
}

ExprValue* ExprValue::MakeIn(Token* pFieldName, ExprValueList* pArgList)
{
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_IN;
  DBVAL_SET_STRING(&(pNew->dbVal_), pFieldName->str_, pFieldName->len_);
  pNew->pArgList_ = pArgList;
  return pNew;
}

ExprValue* ExprValue::MakeNotIn(Token* pFieldName, ExprValueList* pArgList)
{
  ExprValue* pNew = new ExprValue();
  pNew->valType_ = TK_NOTIN;
  DBVAL_SET_STRING(&(pNew->dbVal_), pFieldName->str_, pFieldName->len_);
  pNew->pArgList_ = pArgList;
  return pNew;
}

void ExprValue::FreeExprValue(ExprValue* pValue)
{
  if (pValue != nullptr)
  {
    delete pValue;
  }
}

ExprValueList::ExprValueList()
{
}

ExprValueList::~ExprValueList()
{
  for (auto valIt = valueVec_.begin(); valIt != valueVec_.end(); valIt++)
  {
    delete *valIt;
  }
}

ExprValueList* ExprValueList::AppendExprValue(ExprValueList* pList, ExprValue* pValue)
{
  if (pList == nullptr)
  {
    pList = new ExprValueList();
  }

  if (pValue != nullptr)
  {
    pList->valueVec_.push_back(pValue);
  }
  
  return pList;
}
void ExprValueList::FreeExprValueList(ExprValueList* pList)
{
  if (pList != nullptr)
  {
    delete pList;
  }
}
