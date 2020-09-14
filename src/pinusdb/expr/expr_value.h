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

#pragma once

#include "expr/pdb_db_int.h"
#include "table/db_value.h"
#include <string>
#include <vector>

class ExprValueList;
class TargetList;

class ExprValue
{
public:
  ExprValue();
  ~ExprValue();

  int GetValueType() const { return valType_; }
  DBVal GetValue() const { return dbVal_; }
  const ExprValueList* GetArgList() const { return pArgList_; }
  const ExprValue* GetLeftParam() const { return pLeftParam_; }
  const ExprValue* GetRightParam() const { return pRightParam_; }

  void SwitchStringToID();

  static ExprValue* MakeStarValue();
  static ExprValue* MakeID(Token* pToken);
  static ExprValue* MakeBoolValue(bool value);
  static ExprValue* MakeIntValue(bool negative, Token* pToken);
  static ExprValue* MakeDoubleValue(bool negative, Token* pToken);
  static ExprValue* MakeStringValue(Token* pToken);
  static ExprValue* MakeBlobValue(Token* pToken);
  static ExprValue* MakeTimeValue(bool negative, Token* pVal, Token* pUnit);
  static ExprValue* MakeFunction(Token* pFuncName, ExprValueList* pArgList);
  static ExprValue* MakeCompare(int op, ExprValue* pLeft, ExprValue* pRight);
  static ExprValue* MakeLike(Token* pFieldName, Token* pPattern);
  static ExprValue* MakeIsNotNull(Token* pFieldName);
  static ExprValue* MakeIsNull(Token* pFieldName);
  static ExprValue* MakeIn(Token* pFieldName, ExprValueList* pArgList);
  static ExprValue* MakeNotIn(Token* pFieldName, ExprValueList* pArgList);

  static void FreeExprValue(ExprValue* pValue);

private:
  int valType_;
  DBVal dbVal_;

  ExprValueList* pArgList_;
  ExprValue* pLeftParam_;
  ExprValue* pRightParam_;
};

class ExprValueList
{
public:
  ExprValueList();
  ~ExprValueList();

  const std::vector<ExprValue*>* GetValueList() const { return &valueVec_; }
  void Clear() { valueVec_.clear(); }

  static ExprValueList* AppendExprValue(ExprValueList* pList, ExprValue* pValue);
  static void FreeExprValueList(ExprValueList* pList);

private:
  std::vector<ExprValue*> valueVec_;
};

