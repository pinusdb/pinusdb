/*
* Copyright (c) 2021 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
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

#include "value/value_item.h"
#include "value/calc_value.h"
#include "value/condition_value.h"
#include "value/const_value.h"
#include "value/datetime_value.h"
#include "value/field_value.h"
#include "value/math_value.h"
#include "value/value_item.h"
#include "expr/parse.h"
#include "query/group_function.h"
#include "util/string_tool.h"
#include "query/group_field.h"
#include "query/group_function.h"
#include "util/date_time.h"

/* 获取计算类型
**        整数     浮点数  时间戳
**   整数 Int64    Double  DateTime
** 浮点数 Double   Dobule     X
** 时间戳 DateTime   X     Int64(减法，时间差)
*/
PDB_VALUE_TYPE GetCalcType(PDB_SQL_FUNC op, PDB_VALUE_TYPE type1, PDB_VALUE_TYPE type2)
{
  if (op != PDB_SQL_FUNC::FUNC_ADD
    && op != PDB_SQL_FUNC::FUNC_SUB
    && op != PDB_SQL_FUNC::FUNC_MUL
    && op != PDB_SQL_FUNC::FUNC_DIV
    && op != PDB_SQL_FUNC::FUNC_MOD)
  {
    return PDB_VALUE_TYPE::VAL_NULL;
  }

  if (PDB_VALUE_TYPE::VAL_DOUBLE == type2 && op == PDB_SQL_FUNC::FUNC_MOD)
  {
    return PDB_VALUE_TYPE::VAL_NULL;
  }

  PDB_VALUE_TYPE resultArray[3][3] =
  {
    { PDB_VALUE_TYPE::VAL_INT64, PDB_VALUE_TYPE::VAL_DOUBLE, PDB_VALUE_TYPE::VAL_DATETIME },
    { PDB_VALUE_TYPE::VAL_DOUBLE, PDB_VALUE_TYPE::VAL_DOUBLE, PDB_VALUE_TYPE::VAL_NULL},
    { PDB_VALUE_TYPE::VAL_DATETIME, PDB_VALUE_TYPE::VAL_NULL, PDB_VALUE_TYPE::VAL_INT64}
  };

  if ((PDB_VALUE_TYPE::VAL_DATETIME == type1 || PDB_VALUE_TYPE::VAL_DATETIME == type2)
    && (PDB_TYPE_IS_NUMBER(type1) || PDB_TYPE_IS_NUMBER(type2)))
  {
    // 如果一个参数是时间戳一个参数是整数，只允许做加减法
    if (op != PDB_SQL_FUNC::FUNC_ADD && op != PDB_SQL_FUNC::FUNC_SUB)
    {
      return PDB_VALUE_TYPE::VAL_NULL;
    }
  }

  int i, j = 0;
  if (PDB_TYPE_IS_NUMBER(type1))
  {
    i = 0;
  }
  else if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(type1))
  {
    i = 1;
  }
  else if (PDB_VALUE_TYPE::VAL_DATETIME == type1)
  {
    i = 2;
  }
  else
  {
    return PDB_VALUE_TYPE::VAL_NULL;
  }

  if (PDB_TYPE_IS_NUMBER(type2))
  {
    j = 0;
  }
  else if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(type2))
  {
    j = 1;
  }
  else if (PDB_VALUE_TYPE::VAL_DATETIME == type2)
  {
    j = 2;
  }
  else
  {
    return PDB_VALUE_TYPE::VAL_NULL;
  }

  return resultArray[i][j];
}

/* 获取公共类型
**             布尔   整数   浮点数   时间戳  字符串   二进制
**     布尔    Bool    X       X       X        X        X     
**     整数     X    Int64   Double    X        X        X     
**    浮点数    X    Double  Double    X        X        X   
**    时间戳    X      X      X     DateTime    X        X    
**    字符串    X      X      X        X      String     X     
**    二进制    X      X      X        X        X       Blob
*/
PDB_VALUE_TYPE GetCommonType(PDB_VALUE_TYPE type1, PDB_VALUE_TYPE type2)
{
  if (PDB_TYPE_IS_NUMBER(type1))
  {
    type1 = PDB_VALUE_TYPE::VAL_INT64;
  }
  else if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(type1))
  {
    type1 = PDB_VALUE_TYPE::VAL_DOUBLE;
  }
  if (PDB_TYPE_IS_NUMBER(type2))
  {
    type2 = PDB_VALUE_TYPE::VAL_INT64;
  }
  else if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(type2))
  {
    type2 = PDB_VALUE_TYPE::VAL_DOUBLE;
  }

  if (type1 == type2)
  {
    return type1;
  }

  if (type1 == PDB_VALUE_TYPE::VAL_INT64 && type2 == PDB_VALUE_TYPE::VAL_DOUBLE)
  {
    return PDB_VALUE_TYPE::VAL_DOUBLE;
  }

  if (type1 == PDB_VALUE_TYPE::VAL_DOUBLE && type2 == PDB_VALUE_TYPE::VAL_INT64)
  {
    return PDB_VALUE_TYPE::VAL_DOUBLE;
  }

  return PDB_VALUE_TYPE::VAL_NULL;
}

ValueItem* ConvertStringToDateTime(ValueItem* pValue)
{
  if (pValue->GetValueType() != PDB_VALUE_TYPE::VAL_STRING)
  {
    return pValue;
  }

  if (!(pValue->IsConstValue()))
  {
    return pValue;
  }

  DBVal strVal;
  if (pValue->GetValue(nullptr, &strVal) != PdbE_OK)
  {
    return pValue;
  }

  if (!DBVAL_IS_STRING(&strVal))
  {
    return pValue;
  }

  DateTime dt;
  if (dt.Parse(DBVAL_GET_STRING(&strVal), DBVAL_GET_LEN(&strVal)))
  {
    delete pValue;
    return new ConstValue(dt.TotalMicrosecond(), true);
  }

  return pValue;
}


int GetFunctionId(const ExprValue* pExpr)
{
  if (pExpr == nullptr)
    return 0;

  if (pExpr->GetValueType() != TK_FUNCTION)
    return 0;

  DBVal nameVal = pExpr->GetValue();
  if (!DBVAL_IS_STRING(&nameVal))
    return 0;

  const char* pName = DBVAL_GET_STRING(&nameVal);
  size_t nameLen = DBVAL_GET_LEN(&nameVal);
  if (nameLen < 2 || nameLen > 20)
    return 0;

  if (pName[0] == 'A' || pName[0] == 'a')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "AVG", (sizeof("AVG") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_AVG;
    if (StringTool::ComparyNoCase(pName, nameLen, "AVGIF", (sizeof("AVGIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_AVG_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "ABS", (sizeof("ABS") - 1)))
      return PDB_SQL_FUNC::FUNC_ABS;
    else if (StringTool::ComparyNoCase(pName, nameLen, "ADD", (sizeof("ADD") - 1)))
      return PDB_SQL_FUNC::FUNC_ADD;
  }
  else if (pName[0] == 'D' || pName[0] == 'd')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "DIV", (sizeof("DIV") - 1)))
      return PDB_SQL_FUNC::FUNC_DIV;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMEADD", (sizeof("DATETIMEADD") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMEADD;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMEDIFF", (sizeof("DATETIMEDIFF") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMEDIFF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMEFLOOR", (sizeof("DATETIMEFLOOR") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMEFLOOR;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMECEIL", (sizeof("DATETIMECEIL") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMECEIL;
  }
  else if (pName[0] == 'M' || pName[0] == 'm')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "MIN", (sizeof("MIN") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MIN;
    if (StringTool::ComparyNoCase(pName, nameLen, "MINIF", (sizeof("MINIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MIN_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MAX", (sizeof("MAX") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MAX;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MAXIF", (sizeof("MAXIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MAX_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MUL", (sizeof("MUL") - 1)))
      return PDB_SQL_FUNC::FUNC_MUL;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MOD", (sizeof("MOD") - 1)))
      return PDB_SQL_FUNC::FUNC_MOD;
  }
  else if (pName[0] == 'S' || pName[0] == 's')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "SUM", (sizeof("SUM") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_SUM;
    if (StringTool::ComparyNoCase(pName, nameLen, "SUMIF", (sizeof("SUMIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_SUM_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "SUB", (sizeof("SUB") - 1)))
      return PDB_SQL_FUNC::FUNC_SUB;
  }
  else if (pName[0] == 'C' || pName[0] == 'c')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "COUNT", (sizeof("COUNT") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_COUNT;
    if (StringTool::ComparyNoCase(pName, nameLen, "COUNTIF", (sizeof("COUNTIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_COUNT_IF;
  }
  else if (pName[0] == 'F' || pName[0] == 'f')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "FIRST", (sizeof("FIRST") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_FIRST;
    else if (StringTool::ComparyNoCase(pName, nameLen, "FIRSTIF", (sizeof("FIRSTIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_FIRST_IF;
  }
  else if (pName[0] == 'L' || pName[0] == 'l')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "LAST", (sizeof("LAST") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_LAST;
    else if (StringTool::ComparyNoCase(pName, nameLen, "LASTIF", (sizeof("LASTIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_LAST_IF;
  }
  else
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "NOW", (sizeof("NOW") - 1)))
      return PDB_SQL_FUNC::FUNC_NOW;
    else if (StringTool::ComparyNoCase(pName, nameLen, "IF", (sizeof("IF") - 1)))
      return PDB_SQL_FUNC::FUNC_IF;
  }

  return 0;
}

typedef struct _FieldPosTypePair
{
  int32_t type_;
  size_t pos_;
}FieldPosTypePair;

PdbErr_t GetFieldInfoByExpr(const TableInfo* pTabInfo, const ExprValue* pExpr, FieldPosTypePair* pPosTypePair)
{
  if (pTabInfo == nullptr || pExpr == nullptr)
    return PdbE_INVALID_PARAM;

  DBVal fieldVal = pExpr->GetValue();
  if (!DBVAL_IS_STRING(&fieldVal))
    return PdbE_INVALID_PARAM;

  std::string fieldName = std::string(DBVAL_GET_STRING(&fieldVal), DBVAL_GET_LEN(&fieldVal));
  return pTabInfo->GetFieldInfo(fieldName.c_str(), &(pPosTypePair->pos_), &(pPosTypePair->type_));
}

void DeleteValueVec(std::vector<ValueItem*>* pValVec)
{
  for (size_t idx = 0; idx < pValVec->size(); idx++)
  {
    delete (*pValVec)[idx];
  }
}

bool ConvertExprToValue(const TableInfo* pTabInfo, int64_t nowMicroseconds,
  const ExprValueList* pArgList, std::vector<ValueItem*>* pValVec)
{
  if (pArgList == nullptr)
    return true;

  const std::vector<ExprValue*>* pExprVec = pArgList->GetValueList();
  for (auto argIt = pExprVec->begin(); argIt != pExprVec->end(); argIt++)
  {
    ValueItem* pTmpVal = BuildGeneralValueItem(pTabInfo, *argIt, nowMicroseconds);
    if (pTmpVal == nullptr)
      break;

    pValVec->push_back(pTmpVal);
  }

  if (pValVec->size() != pExprVec->size())
  {
    DeleteValueVec(pValVec);
    return false;
  }

  return true;
}

ValueItem* BuildCalcFunction(int op, const TableInfo* pTabInfo, int64_t nowMicroseconds, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
  {
    return nullptr;
  }

  if (!ConvertExprToValue(pTabInfo, nowMicroseconds, pArgList, &argVec))
  {
    return nullptr;
  }

  ValueItem* pResult = nullptr;
  do {
    if (argVec.size() != 2)
    {
      break;
    }

    pResult = CreateCalcFunction((PDB_SQL_FUNC)op, argVec[0], argVec[1]);

  } while (false);

  if (pResult == nullptr)
  {
    DeleteValueVec(&argVec);
  }
  return pResult;
}

ValueItem* BuildDateTimeCalcFunction(int op, const TableInfo* pTabInfo, int64_t nowMicroseconds, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
  {
    return nullptr;
  }

  if (!ConvertExprToValue(pTabInfo, nowMicroseconds, pArgList, &argVec))
  {
    return nullptr;
  }

  ValueItem* pResult = nullptr;

  do {
    if (argVec.size() != 2)
      break;

    argVec[0] = ConvertStringToDateTime(argVec[0]);
    argVec[1] = ConvertStringToDateTime(argVec[1]);

    if (op == PDB_SQL_FUNC::FUNC_DATETIMEADD)
    {
      pResult = CreateDateTimeAdd(argVec[0], argVec[1]);
    }
    else if (op == PDB_SQL_FUNC::FUNC_DATETIMEDIFF)
    {
      pResult = CreateDateTimeDiff(argVec[0], argVec[1]);
    }
    else if (op == PDB_SQL_FUNC::FUNC_DATETIMEFLOOR || op == PDB_SQL_FUNC::FUNC_DATETIMECEIL)
    {
      DBVal tmpUnit;
      if (!argVec[1]->IsConstValue())
        break;

      PdbErr_t retVal = argVec[1]->GetValue(nullptr, &tmpUnit);
      if (retVal != PdbE_OK)
        break;

      delete argVec[1];
      argVec.erase((argVec.begin() + 1));

      if (!DBVAL_IS_STRING(&tmpUnit))
        break;

      int64_t microOffset = 0;
      if (!DateTime::GetMicrosecondByTimeUnit(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), &microOffset))
      {
        break;
      }

      pResult = CreateDateTimeAlign(op == PDB_SQL_FUNC::FUNC_DATETIMECEIL, argVec[0], microOffset);
      if (op == PDB_SQL_FUNC::FUNC_DATETIMEFLOOR)
      {
        pResult = new DateTimeAlign<false>(argVec[0], microOffset);
      }
      else if (op == PDB_SQL_FUNC::FUNC_DATETIMECEIL)
      {
        pResult = new DateTimeAlign<true>(argVec[0], microOffset);
      }
    }

  } while (false);

  if (pResult == nullptr)
  {
    DeleteValueVec(&argVec);
  }

  return pResult;
}

ValueItem* BuildNowFunction(const TableInfo* pTabInfo, int64_t nowMicroseconds, const ExprValueList* pArgList)
{
  if (pArgList != nullptr)
  {
    const std::vector<ExprValue*>* pExprVec = pArgList->GetValueList();
    if (pExprVec->size() > 0)
    {
      return nullptr;
    }
  }

  return new ConstValue(nowMicroseconds, true);
}

ValueItem* CreateAbsFunctionExpr(const TableInfo* pTabInfo, int64_t nowMicroseconds, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
    return nullptr;

  if (!ConvertExprToValue(pTabInfo, nowMicroseconds, pArgList, &argVec))
    return nullptr;

  if (argVec.size() == 1)
  {
    int32_t valueType = argVec[0]->GetValueType();
    switch (valueType)
    {
    case PDB_FIELD_TYPE::TYPE_INT8:
      return new AbsFunction<PDB_VALUE_TYPE::VAL_INT8>(argVec[0]);
    case PDB_FIELD_TYPE::TYPE_INT16:
      return new AbsFunction<PDB_VALUE_TYPE::VAL_INT16>(argVec[0]);
    case PDB_FIELD_TYPE::TYPE_INT32:
      return new AbsFunction<PDB_VALUE_TYPE::VAL_INT32>(argVec[0]);
    case PDB_FIELD_TYPE::TYPE_INT64:
      return new AbsFunction<PDB_VALUE_TYPE::VAL_INT64>(argVec[0]);
    case PDB_FIELD_TYPE::TYPE_FLOAT:
      return new AbsFunction<PDB_VALUE_TYPE::VAL_FLOAT>(argVec[0]);
    case PDB_FIELD_TYPE::TYPE_DOUBLE:
      return new AbsFunction<PDB_VALUE_TYPE::VAL_DOUBLE>(argVec[0]);
    }
  }

  DeleteValueVec(&argVec);
  return nullptr;
}

ValueItem* BuildIfFunction(const TableInfo* pTabInfo, int64_t nowMicroseconds, const ExprValueList* pArgList)
{
  ValueItem* pResult = nullptr;
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
    return nullptr;

  if (!ConvertExprToValue(pTabInfo, nowMicroseconds, pArgList, &argVec))
  {
    return nullptr;
  }

  do {
    if (argVec.size() != 3)
    {
      break; //参数个数错误
    }

    pResult = CreateIfFunction(argVec[0], argVec[1], argVec[2]);
    if (pResult != nullptr)
    {
      return pResult;
    }

  } while (false);

  DeleteValueVec(&argVec);
  return nullptr;
}

ValueItem* BuildAbsFunction(const TableInfo* pTabInfo, int64_t nowMicroseconds, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
    return nullptr;

  if (!ConvertExprToValue(pTabInfo, nowMicroseconds, pArgList, &argVec))
    return nullptr;

  if (argVec.size() == 1)
  {
    ValueItem* pResult = CreateAbsFunction(argVec[0]);
    if (pResult != nullptr)
    {
      return pResult;
    }
  }

  DeleteValueVec(&argVec);
  return nullptr;
}

ValueItem* CreateFieldValue(int fieldType, size_t fieldPos)
{
  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_BOOL:
    return new FieldValue<PDB_VALUE_TYPE::VAL_BOOL>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new FieldValue<PDB_VALUE_TYPE::VAL_INT8>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new FieldValue<PDB_VALUE_TYPE::VAL_INT16>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new FieldValue<PDB_VALUE_TYPE::VAL_INT32>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new FieldValue<PDB_VALUE_TYPE::VAL_INT64>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    return new FieldValue<PDB_VALUE_TYPE::VAL_DATETIME>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return new FieldValue<PDB_VALUE_TYPE::VAL_FLOAT>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new FieldValue<PDB_VALUE_TYPE::VAL_DOUBLE>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_STRING:
    return new FieldValue<PDB_VALUE_TYPE::VAL_STRING>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_BLOB:
    return new FieldValue<PDB_VALUE_TYPE::VAL_BLOB>(fieldPos);
  }

  return nullptr;
}

ValueItem* BuildFieldValue(const TableInfo* pTableInfo, const ExprValue* pExpr)
{
  PdbErr_t retVal = PdbE_OK;
  FieldPosTypePair posTypePair;
  retVal = GetFieldInfoByExpr(pTableInfo, pExpr, &posTypePair);
  if (retVal != PdbE_OK)
    return nullptr;

  return CreateFieldValue(posTypePair.type_, posTypePair.pos_);
}

ValueItem* CreateIsNullOrNullFunction(bool notNull, const TableInfo* pTabInfo, const ExprValue* pExpr)
{
  ValueItem* pFieldValue = BuildFieldValue(pTabInfo, pExpr);
  if (pFieldValue == nullptr)
    return nullptr;

  if (notNull)
    return new NullFunction<true>(pFieldValue);
  else
    return new NullFunction<false>(pFieldValue);
}

ValueItem* CreateLikeFunction(const TableInfo* pTableInfo, const ExprValue* pExpr)
{
  if (pTableInfo == nullptr || pExpr == nullptr)
    return nullptr;

  const ExprValue* pPatVal = pExpr->GetLeftParam();
  if (pPatVal == nullptr)
    return nullptr;

  if (pPatVal->GetValueType() != TK_STRING)
    return nullptr;

  DBVal patStr = pPatVal->GetValue();
  if (!DBVAL_IS_STRING(&patStr))
    return nullptr;

  ValueItem* pFieldValue = BuildFieldValue(pTableInfo, pExpr);
  if (pFieldValue == nullptr)
    return nullptr;

  return new LikeFunction(pFieldValue, DBVAL_GET_STRING(&patStr), DBVAL_GET_LEN(&patStr));
}


template<bool NotIn>
ValueItem* CreateInOrNotInFunction(const TableInfo* pTableInfo, 
  const ExprValue* pExpr, int64_t nowMicroseconds)
{
  PdbErr_t retVal = PdbE_OK;
  FieldPosTypePair posTypePair;
  retVal = GetFieldInfoByExpr(pTableInfo, pExpr, &posTypePair);
  if (retVal != PdbE_OK)
    return nullptr;

  int32_t argType = 0;
  if (PDB_TYPE_IS_NUMBER(posTypePair.type_))
  {
    argType = PDB_FIELD_TYPE::TYPE_INT64;
  }
  else if (PDB_FIELD_TYPE::TYPE_STRING == posTypePair.type_)
  {
    argType = PDB_FIELD_TYPE::TYPE_STRING;
  }
  else
  {
    return nullptr;
  }

  const ExprValueList* pArgList = pExpr->GetArgList();
  const std::vector<ExprValue*>* pValVec = pArgList->GetValueList();
  std::list<int64_t> argList;
  for (auto argIt = pValVec->begin(); argIt != pValVec->end(); argIt++)
  {
    DBVal tmpVal = (*argIt)->GetValue();
    if (DBVAL_GET_TYPE(&tmpVal) != argType)
    {
      return nullptr;
    }

    if (DBVAL_IS_INT64(&tmpVal))
    {
      argList.push_back(DBVAL_GET_INT64(&tmpVal));
    }
    else
    {
      argList.push_back(static_cast<int64_t>(StringTool::CRC64(DBVAL_GET_STRING(&tmpVal), DBVAL_GET_LEN(&tmpVal))));
    }
  }

  if (argList.empty())
  {
    return nullptr;
  }

  switch (posTypePair.type_)
  {
  case PDB_VALUE_TYPE::VAL_INT8:
    return new InFunction<NotIn, PDB_VALUE_TYPE::VAL_INT8>(posTypePair.pos_, argList);
  case PDB_VALUE_TYPE::VAL_INT16:
    return new InFunction<NotIn, PDB_VALUE_TYPE::VAL_INT16>(posTypePair.pos_, argList);
  case PDB_VALUE_TYPE::VAL_INT32:
    return new InFunction<NotIn, PDB_VALUE_TYPE::VAL_INT32>(posTypePair.pos_, argList);
  case PDB_VALUE_TYPE::VAL_INT64:
    return new InFunction<NotIn, PDB_VALUE_TYPE::VAL_INT64>(posTypePair.pos_, argList);
  case PDB_VALUE_TYPE::VAL_STRING:
    return new InFunction<NotIn, PDB_VALUE_TYPE::VAL_STRING>(posTypePair.pos_, argList);
  }

  return nullptr;
}

ValueItem* CreateGeneralFunction(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMicroseconds)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  size_t fieldPos = 0;
  char uniqueName[32];
  if (pTableInfo == nullptr || pExpr == nullptr)
    return nullptr;

  int funcId = GetFunctionId(pExpr);
  if (funcId <= 0)
    return nullptr;

  ValueItem* pResultVal = nullptr;

  do {
    if (funcId == PDB_SQL_FUNC::FUNC_AGG_COUNT
      || funcId == PDB_SQL_FUNC::FUNC_AGG_FIRST
      || funcId == PDB_SQL_FUNC::FUNC_AGG_LAST
      || funcId == PDB_SQL_FUNC::FUNC_AGG_AVG
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MIN
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MAX
      || funcId == PDB_SQL_FUNC::FUNC_AGG_SUM
      || funcId == PDB_SQL_FUNC::FUNC_AGG_COUNT_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_FIRST_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_LAST_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_AVG_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MIN_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MAX_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_SUM_IF)
    {
#ifdef _WIN32
      sprintf(uniqueName, "agg_%llu", reinterpret_cast<uintptr_t>(pExpr));
#else
      sprintf(uniqueName, "agg_%lu", reinterpret_cast<uintptr_t>(pExpr));
#endif
      retVal = pTableInfo->GetFieldInfo(uniqueName, &fieldPos, &fieldType);
      if (retVal != PdbE_OK)
      {
        return nullptr;
      }

      return CreateFieldValue(fieldType, fieldPos);
    }

    switch (funcId)
    {
    case PDB_SQL_FUNC::FUNC_ADD:
    case PDB_SQL_FUNC::FUNC_SUB:
    case PDB_SQL_FUNC::FUNC_MUL:
    case PDB_SQL_FUNC::FUNC_DIV:
    case PDB_SQL_FUNC::FUNC_MOD:
      pResultVal = BuildCalcFunction(funcId, pTableInfo, nowMicroseconds, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_DATETIMEADD:
    case PDB_SQL_FUNC::FUNC_DATETIMEDIFF:
    case PDB_SQL_FUNC::FUNC_DATETIMEFLOOR:
    case PDB_SQL_FUNC::FUNC_DATETIMECEIL:
      pResultVal = BuildDateTimeCalcFunction(funcId, pTableInfo, nowMicroseconds, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_IF:
      pResultVal = BuildIfFunction(pTableInfo, nowMicroseconds, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_ABS:
      pResultVal = BuildAbsFunction(pTableInfo, nowMicroseconds, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_NOW:
      pResultVal = BuildNowFunction(pTableInfo, nowMicroseconds, pExpr->GetArgList());
      break;
    }

  } while (false);

  if (pResultVal != nullptr)
  {
    if (pResultVal->IsValid())
      return pResultVal;

    delete pResultVal;
  }

  return nullptr;
}

ValueItem* BuildOperator(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMicroseconds)
{
  if (pExpr == nullptr)
  {
    return nullptr;
  }

  int op = pExpr->GetValueType();
  const ExprValue* pLeftExpr = pExpr->GetLeftParam();
  const ExprValue* pRightExpr = pExpr->GetRightParam();
  if (pLeftExpr == nullptr || pRightExpr == nullptr)
  {
    return nullptr;
  }

  PdbErr_t retVal = PdbE_OK;
  ValueItem* pLeftValue = nullptr;
  ValueItem* pRightValue = BuildGeneralValueItem(pTableInfo, pRightExpr, nowMicroseconds);
  if (pRightValue == nullptr)
  {
    return nullptr;
  }

  ValueItem* pResultValue = nullptr;
  DateTime dt;
  FieldPosTypePair posTypePair;

  if (pLeftExpr->GetValueType() == TK_ID && pRightValue->IsConstValue())
  {
    do {

      retVal = GetFieldInfoByExpr(pTableInfo, pLeftExpr, &posTypePair);
      if (retVal != PdbE_OK)
        break;
      
      DBVal fieldVal = pLeftExpr->GetValue();
      std::string fieldName = std::string(DBVAL_GET_STRING(&fieldVal), DBVAL_GET_LEN(&fieldVal));

      DBVal rightVal;
      retVal = pRightValue->GetValue(nullptr, &rightVal);
      if (retVal != PdbE_OK)
        break;

      if (posTypePair.type_ == PDB_VALUE_TYPE::VAL_DATETIME && DBVAL_IS_STRING(&rightVal))
      {
        if (dt.Parse(DBVAL_GET_STRING(&rightVal), DBVAL_GET_LEN(&rightVal)))
        {
          DBVAL_SET_DATETIME(&rightVal, dt.TotalMicrosecond());
        }
      }

      PDB_VALUE_TYPE commonType = GetCommonType((PDB_VALUE_TYPE)posTypePair.type_, (PDB_VALUE_TYPE)DBVAL_GET_TYPE(&rightVal));
      if (commonType == PDB_VALUE_TYPE::VAL_NULL)
      {
        return nullptr;
      }

      if (StringTool::ComparyNoCase(fieldName.c_str(), DEVID_FIELD_NAME))
      {
        //如果字段是 Devid
        pResultValue = CreateDevIdCompareFunction(op, DBVAL_GET_INT64(&rightVal));
      }
      else if (StringTool::ComparyNoCase(fieldName.c_str(), TSTAMP_FIELD_NAME))
      {
        pResultValue = CreateTsCompareFunction(op, DBVAL_GET_DATETIME(&rightVal));
      }
      else if (posTypePair.type_ == PDB_FIELD_TYPE::TYPE_STRING)
      {
        pResultValue = CreateStringFieldCompareFunction(op, posTypePair.pos_, std::string(DBVAL_GET_STRING(&rightVal), DBVAL_GET_LEN(&rightVal)));
      }
      else 
      {
        pResultValue = CreateNumFieldCompareFunction(op, posTypePair.pos_, (PDB_VALUE_TYPE)posTypePair.type_, rightVal);
      }

    } while (false);

    delete pRightValue;
    return pResultValue;
  }
  else
  {
    do {
      pLeftValue = BuildGeneralValueItem(pTableInfo, pLeftExpr, nowMicroseconds);
      if (pLeftValue == nullptr)
      {
        break;
      }

      pResultValue = CreateCompareFunction(op, pLeftValue, pRightValue);
    } while (false);

    if (pResultValue == nullptr)
    {
      if (pLeftValue != nullptr)
      {
        delete pLeftValue;
      }

      if (pRightValue != nullptr)
      {
        delete pRightValue;
      }
    }

    return pResultValue;
  }
}

PdbErr_t BuildAndFunctionLeaf(const TableInfo* pTableInfo, const ExprValue* pExpr,
  int64_t nowMicroseconds, AndFunction* pAndFunc)
{
  PdbErr_t retVal = PdbE_OK;
  int opType = pExpr->GetValueType();
  if (opType == TK_AND)
  {
    const ExprValue* pLeftExpr = pExpr->GetLeftParam();
    const ExprValue* pRightExpr = pExpr->GetRightParam();
    if (pLeftExpr == nullptr || pRightExpr == nullptr)
      return PdbE_SQL_ERROR;

    retVal = BuildAndFunctionLeaf(pTableInfo, pLeftExpr, nowMicroseconds, pAndFunc);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = BuildAndFunctionLeaf(pTableInfo, pRightExpr, nowMicroseconds, pAndFunc);
    if (retVal != PdbE_OK)
      return retVal;
  }
  else
  {
    ValueItem* pValue = BuildGeneralValueItem(pTableInfo, pExpr, nowMicroseconds);
    if (pValue == nullptr)
      return PdbE_SQL_ERROR;

    pAndFunc->AddValueItem(pValue);
  }

  return PdbE_OK;
}

ValueItem* BuildAndFunction(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMicroseconds)
{
  AndFunction* pAndFunc = new AndFunction();
  if (BuildAndFunctionLeaf(pTableInfo, pExpr, nowMicroseconds, pAndFunc) != PdbE_OK)
  {
    delete pAndFunc;
    return nullptr;
  }

  return pAndFunc;
}

template<bool IsFirst>
GroupField* CreateAggFirstOrLast(size_t fieldPos, int fieldType)
{
  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_BOOL:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_BOOL, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_INT8, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_INT16, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_INT32, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_INT64, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_DATETIME, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_FLOAT, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_DOUBLE, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_STRING:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_STRING, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_BLOB:
    return new FirstOrLastValueFunc<PDB_FIELD_TYPE::TYPE_BLOB, IsFirst>(fieldPos);
  }

  return nullptr;
}

GroupField* CreateAggAvg(size_t fieldPos, int fieldType)
{
  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_INT8, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_INT16, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_INT32, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_INT64, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_FLOAT, double>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_DOUBLE, double>(fieldPos);
  }

  return nullptr;
}

GroupField* CreateAggSum(size_t fieldPos, int fieldType)
{
  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_INT8, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_INT16, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_INT32, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_INT64, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_FLOAT, double>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_DOUBLE, double>(fieldPos);
  }

  return nullptr;
}

template<int CompareType, bool IsMax>
GroupField* CreateAggExtremeLeaf(int targetType, size_t comparePos, size_t targetPos)
{
  switch (targetType)
  {
  case PDB_FIELD_TYPE::TYPE_BOOL:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_BOOL, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT8:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_INT8, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_INT16, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_INT32, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_INT64, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_DATETIME, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_FLOAT, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_DOUBLE, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_STRING:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_STRING, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_BLOB:
    return new ExtremeValueFunc<CompareType, PDB_FIELD_TYPE::TYPE_BLOB, IsMax>(comparePos, targetPos);
  }

  return nullptr;
}

template<bool IsMax>
GroupField* CreateAggExtreme(int compareType, int targetType, size_t comparePos, size_t targetPos)
{
  switch (compareType)
  {
  case PDB_FIELD_TYPE::TYPE_INT8:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_INT8, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT16:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_INT16, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT32:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_INT32, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_INT64, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_DATETIME, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_FLOAT:
    return CreateAggExtremeLeaf< PDB_FIELD_TYPE::TYPE_FLOAT, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return CreateAggExtremeLeaf<PDB_FIELD_TYPE::TYPE_DOUBLE, IsMax>(targetType, comparePos, targetPos);
  }

  return nullptr;
}


ValueItem* BuildGeneralValueItem(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMicroseconds)
{
  if (pExpr == nullptr)
    return nullptr;

  ValueItem* pNewValue = nullptr;
  DBVal exprVal = pExpr->GetValue();
  int exprType = pExpr->GetValueType();
  switch (exprType)
  {
  case TK_ID:
    pNewValue = BuildFieldValue(pTableInfo, pExpr);
    break;
  case TK_TRUE:
  case TK_FALSE:
  case TK_INTEGER:
  case TK_DOUBLE:
  case TK_STRING:
  case TK_BLOB:
  case TK_TIMEVAL:
    pNewValue = new ConstValue(exprVal);
    break;
  case TK_FUNCTION:
    pNewValue = CreateGeneralFunction(pTableInfo, pExpr, nowMicroseconds);
    break;
  case TK_LT:
  case TK_LE:
  case TK_GT:
  case TK_GE:
  case TK_EQ:
  case TK_NE:
    pNewValue = BuildOperator(pTableInfo, pExpr, nowMicroseconds);
    break;
  case TK_AND:
    pNewValue = BuildAndFunction(pTableInfo, pExpr, nowMicroseconds);
    break;
  case TK_LIKE:
    pNewValue = CreateLikeFunction(pTableInfo, pExpr);
    break;
  case TK_ISNOTNULL:
    pNewValue = CreateIsNullOrNullFunction(true, pTableInfo, pExpr);
    break;
  case TK_ISNULL:
    pNewValue = CreateIsNullOrNullFunction(false, pTableInfo, pExpr);
    break;
  case TK_IN:
    pNewValue = CreateInOrNotInFunction<false>(pTableInfo, pExpr, nowMicroseconds);
    break;
  case TK_NOTIN:
    pNewValue = CreateInOrNotInFunction<true>(pTableInfo, pExpr, nowMicroseconds);
    break;
  }

  if (pNewValue != nullptr)
  {
    if (!pNewValue->IsValid())
    {
      delete pNewValue;
      pNewValue = nullptr;
    }
  }

  return pNewValue;
}

GroupField* BuildTargetGroupStep(int funcId, const TableInfo* pTableInfo, int64_t nowMicroseconds,
  const ExprValue* pParam1, const ExprValue* pParam2)
{
  PdbErr_t retVal = PdbE_OK;
  FieldPosTypePair posType1;
  FieldPosTypePair posType2;

  //先处理count() 和 count(*)的情况
  if (pParam2 == nullptr && funcId == PDB_SQL_FUNC::FUNC_AGG_COUNT)
  {
    if (pParam1 == nullptr || (pParam1 != nullptr && pParam1->GetValueType() == TK_STAR))
    {
      return new CountFunc(PDB_DEVID_INDEX);
    }
  }

  if (pParam1 != nullptr)
  {
    retVal = GetFieldInfoByExpr(pTableInfo, pParam1, &posType1);
    if (retVal != PdbE_OK)
      return nullptr;
  }

  if (pParam2 != nullptr)
  {
    retVal = GetFieldInfoByExpr(pTableInfo, pParam2, &posType2);
    if (retVal != PdbE_OK)
      return nullptr;
  }

  if (pParam1 != nullptr && pParam2 == nullptr)
  {
    switch (funcId)
    {
    case PDB_SQL_FUNC::FUNC_AGG_COUNT:
      return new CountFunc(posType1.pos_);
    case PDB_SQL_FUNC::FUNC_AGG_FIRST:
      return CreateAggFirstOrLast<true>(posType1.pos_, posType1.type_);
    case PDB_SQL_FUNC::FUNC_AGG_LAST:
      return CreateAggFirstOrLast<false>(posType1.pos_, posType1.type_);
    case PDB_SQL_FUNC::FUNC_AGG_AVG:
      return CreateAggAvg(posType1.pos_, posType1.type_);
    case PDB_SQL_FUNC::FUNC_AGG_MIN:
      return CreateAggExtreme<false>(posType1.type_, posType1.type_, posType1.pos_, posType1.pos_);
    case PDB_SQL_FUNC::FUNC_AGG_MAX:
      return CreateAggExtreme<true>(posType1.type_, posType1.type_, posType1.pos_, posType1.pos_);
    case PDB_SQL_FUNC::FUNC_AGG_SUM:
      return CreateAggSum(posType1.pos_, posType1.type_);
    }
  }
  else if (pParam1 != nullptr && pParam2 != nullptr)
  {
    if (pParam1->GetValueType() != TK_ID)
      return nullptr;

    if (funcId == PDB_SQL_FUNC::FUNC_AGG_MIN)
    {
      return CreateAggExtreme<false>(posType1.type_, posType2.type_, posType1.pos_, posType2.pos_);
    }
    else if (funcId == PDB_SQL_FUNC::FUNC_AGG_MAX)
    {
      return CreateAggExtreme<true>(posType1.type_, posType2.type_, posType1.pos_, posType2.pos_);
    }
  }

  return nullptr;
}

PdbErr_t BuildTargetGroupItem(const TableInfo* pTableInfo, const ExprValue* pExpr,
  TableInfo* pGroupInfo, std::vector<GroupField*>& fieldVec, int64_t nowMicroseconds)
{
  PdbErr_t retVal = PdbE_OK;
  char uniqueName[32];
  if (pTableInfo == nullptr || pExpr == nullptr || pGroupInfo == nullptr)
    return PdbE_INVALID_PARAM;

#ifdef _WIN32
  sprintf(uniqueName, "agg_%llu", reinterpret_cast<uintptr_t>(pExpr));
#else
  sprintf(uniqueName, "agg_%lu", reinterpret_cast<uintptr_t>(pExpr));
#endif

  if (pExpr->GetValueType() == TK_FUNCTION)
  {
    int functionId = GetFunctionId(pExpr);
    if (functionId <= 0)
    {
      return PdbE_SQL_ERROR;
    }

    const ExprValue* pParam1 = nullptr;
    const ExprValue* pParam2 = nullptr;
    const ExprValueList* pArgList = pExpr->GetArgList();

    if (functionId == PDB_SQL_FUNC::FUNC_AGG_COUNT
      || functionId == PDB_SQL_FUNC::FUNC_AGG_FIRST
      || functionId == PDB_SQL_FUNC::FUNC_AGG_LAST
      || functionId == PDB_SQL_FUNC::FUNC_AGG_AVG
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MIN
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MAX
      || functionId == PDB_SQL_FUNC::FUNC_AGG_SUM)
    {
      if (pArgList != nullptr)
      {
        const std::vector<ExprValue*>* pArgVec = pArgList->GetValueList();
        if (pArgVec != nullptr)
        {
          if (pArgVec->size() > 2)
            return PdbE_SQL_ERROR;

          if (pArgVec->size() >= 1)
            pParam1 = pArgVec->at(0);

          if (pArgVec->size() == 2)
            pParam2 = pArgVec->at(1);
        }
      }

      GroupField* pAggFunc = BuildTargetGroupStep(functionId, pTableInfo, nowMicroseconds, pParam1, pParam2);
      if (pAggFunc == nullptr)
        return PdbE_SQL_ERROR;

      pGroupInfo->AddField(uniqueName, pAggFunc->FieldType());
      fieldVec.push_back(pAggFunc);
      return PdbE_OK;
    }
    else if (functionId == PDB_SQL_FUNC::FUNC_AGG_COUNT_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_FIRST_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_LAST_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_AVG_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MIN_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MAX_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_SUM_IF)
    {
      if (pArgList == nullptr)
        return PdbE_SQL_ERROR;

      const std::vector<ExprValue*>* pArgVec = pArgList->GetValueList();
      if (pArgVec == nullptr)
        return PdbE_SQL_ERROR;

      if (pArgVec->size() < 1 || pArgVec->size() > 3)
        return PdbE_SQL_ERROR;

      if (pArgVec->size() >= 2)
        pParam1 = pArgVec->at(1);

      if (pArgVec->size() == 3)
        pParam2 = pArgVec->at(2);

      ValueItem* pCondi = BuildGeneralValueItem(pTableInfo, pArgVec->at(0), nowMicroseconds);
      if (pCondi == nullptr)
        return PdbE_SQL_ERROR;

      int subFunc = 0;
      switch (functionId)
      {
      case PDB_SQL_FUNC::FUNC_AGG_COUNT_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_COUNT; break;
      case PDB_SQL_FUNC::FUNC_AGG_FIRST_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_FIRST; break;
      case PDB_SQL_FUNC::FUNC_AGG_LAST_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_LAST; break;
      case PDB_SQL_FUNC::FUNC_AGG_AVG_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_AVG; break;
      case PDB_SQL_FUNC::FUNC_AGG_MIN_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_MIN; break;
      case PDB_SQL_FUNC::FUNC_AGG_MAX_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_MAX; break;
      case PDB_SQL_FUNC::FUNC_AGG_SUM_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_SUM; break;
      }

      GroupField* pSubAggFunc = BuildTargetGroupStep(subFunc, pTableInfo, nowMicroseconds, pParam1, pParam2);
      if (pSubAggFunc == nullptr)
      {
        delete pCondi;
        return PdbE_SQL_ERROR;
      }

      GroupField* pAggFunc = new AggIfExtendFunc(pCondi, pSubAggFunc, true);
      pGroupInfo->AddField(uniqueName, pAggFunc->FieldType());
      fieldVec.push_back(pAggFunc);
      return PdbE_OK;
    }
  }
  else
  {
    const ExprValue* pLeftParam = pExpr->GetLeftParam();
    if (pLeftParam != nullptr)
    {
      retVal = BuildTargetGroupItem(pTableInfo, pLeftParam, pGroupInfo, fieldVec, nowMicroseconds);
      if (retVal != PdbE_OK)
        return retVal;
    }

    const ExprValue* pRightParam = pExpr->GetRightParam();
    if (pRightParam != nullptr)
    {
      retVal = BuildTargetGroupItem(pTableInfo, pRightParam, pGroupInfo, fieldVec, nowMicroseconds);
      if (retVal != PdbE_OK)
        return retVal;
    }

    const ExprValueList* pArgList = pExpr->GetArgList();
    if (pArgList != nullptr)
    {
      const std::vector<ExprValue*>* pValVec = pArgList->GetValueList();
      for (auto argIt = pValVec->begin(); argIt != pValVec->end(); argIt++)
      {
        retVal = BuildTargetGroupItem(pTableInfo, (*argIt), pGroupInfo, fieldVec, nowMicroseconds);
        if (retVal != PdbE_OK)
          return retVal;
      }
    }
  }

  return PdbE_OK;
}

bool IncludeAggFunction(const ExprValue* pExpr)
{
  if (pExpr == nullptr)
  {
    return false;
  }

  if (pExpr->GetValueType() == TK_FUNCTION)
  {
    int functionId = GetFunctionId(pExpr);
    if (functionId <= 0)
    {
      return false;
    }

    if (functionId > PDB_SQL_FUNC::_FUNC_AGG_MINID_ && functionId < PDB_SQL_FUNC::_FUNC_AGG_MAXID_)
    {
      return true;
    }
  }

  if (IncludeAggFunction(pExpr->GetLeftParam()))
  {
    return true;
  }
  
  if (IncludeAggFunction(pExpr->GetRightParam()))
  {
    return true;
  }

  const ExprValueList* pArgList = pExpr->GetArgList();
  if (pArgList != nullptr)
  {
    const std::vector<ExprValue*>* pArgVec = pArgList->GetValueList();
    for (auto argIt = pArgVec->begin(); argIt != pArgVec->end(); argIt++)
    {
      if (IncludeAggFunction(*argIt))
      {
        return true;
      }
    }
  }

  return false;
}


