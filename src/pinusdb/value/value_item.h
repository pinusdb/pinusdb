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

#pragma once
#include "internal.h"
#include "util/string_tool.h"
#include "table/db_value.h"
#include "util/date_time.h"
#include "expr/expr_value.h"
#include "table/table_info.h"
#include "query/group_field.h"
#include "query/block_values.h"
#include <list>

class ValueItem
{
public:
  ValueItem() {}
  virtual ~ValueItem() {}

  //获取值
  virtual PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const = 0;
  virtual PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const = 0;
  //获取值类型
  virtual PDB_VALUE_TYPE GetValueType() const = 0;
  //是否符合规则
  virtual bool IsValid() const = 0;
  //是否是常数值
  virtual bool IsConstValue() const = 0;
  //获取使用过的字段
  virtual void GetUseFields(std::unordered_set<size_t>& fieldSet) const = 0;

  //是否是DevId过滤条件
  virtual bool IsDevIdCondition() const { return false; }
  //是否是Tstamp过滤条件
  virtual bool IsTstampCondition() const { return false; }
  //获取DevId范围
  virtual bool GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const { return false; }
  //获取Tstamp范围
  virtual bool GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const { return false; }
};


ValueItem* CreateFieldValue(int fieldType, size_t fieldPos);

ValueItem* BuildGeneralValueItem(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMicroseconds);
PdbErr_t BuildTargetGroupItem(const TableInfo* pTableInfo, const ExprValue* pExpr,
  TableInfo* pGroupInfo, std::vector<GroupField*>& fieldVec, int64_t nowMicroseconds);

bool IncludeAggFunction(const ExprValue* pExpr);

/* 获取计算类型
**        整数     浮点数  时间戳
**   整数 Int64    Double  DateTime
** 浮点数 Double   Dobule     X
** 时间戳 DateTime   X     Int64(减法，时间差)
*/
PDB_VALUE_TYPE GetCalcType(PDB_SQL_FUNC op, PDB_VALUE_TYPE type1, PDB_VALUE_TYPE type2);


/* 获取公共类型
**             布尔   整数   浮点数   时间戳  字符串   二进制
**     布尔    Bool    X       X       X        X        X
**     整数     X    Int64   Double    X        X        X
**    浮点数    X    Double  Double    X        X        X
**    时间戳    X      X      X     DateTime    X        X
**    字符串    X      X      X        X      String     X
**    二进制    X      X      X        X        X       Blob
*/
PDB_VALUE_TYPE GetCommonType(PDB_VALUE_TYPE type1, PDB_VALUE_TYPE type2);

#define _GET_INT64_BY_DATATYPE_ARRAY(pVs, i, r, DType) do { \
  if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT8) { r = DBVAL_ELE_GET_INT8(pVs, i); }  \
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT16) { r = DBVAL_ELE_GET_INT16(pVs, i); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT32) { r = DBVAL_ELE_GET_INT32(pVs, i); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT64) { r = DBVAL_ELE_GET_INT64(pVs, i); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_DATETIME) { r = DBVAL_ELE_GET_DATETIME(pVs, i); }\
} while (false)

#define _GET_DOUBLE_BY_DATATYPE_ARRAY(pVs, i, r, DType) do { \
  if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT8) { r = static_cast<double>(DBVAL_ELE_GET_INT8(pVs, i)); }  \
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT16) { r = static_cast<double>(DBVAL_ELE_GET_INT16(pVs, i)); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT32) { r = static_cast<double>(DBVAL_ELE_GET_INT32(pVs, i)); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT64) { r = static_cast<double>(DBVAL_ELE_GET_INT64(pVs, i)); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_DATETIME) { r = static_cast<double>(DBVAL_ELE_GET_DATETIME(pVs, i)); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_FLOAT) { r = static_cast<double>(DBVAL_ELE_GET_FLOAT(pVs, i)); } \
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_DOUBLE) { r = DBVAL_ELE_GET_DOUBLE(pVs, i); } \
} while (false)

#define _GET_INT64_BY_DATATYPE_SINGLE(pV, r, DType) do { \
  if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT8) { r = DBVAL_GET_INT8(pV); }  \
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT16) { r = DBVAL_GET_INT16(pV); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT32) { r = DBVAL_GET_INT32(pV); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT64) { r = DBVAL_GET_INT64(pV); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_DATETIME) { r = DBVAL_GET_DATETIME(pV); }\
} while (false)

#define _GET_DOUBLE_BY_DATATYPE_SINGLE(pV, r, DType) do { \
  if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT8) { r = static_cast<double>(DBVAL_GET_INT8(pV)); }  \
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT16) { r = static_cast<double>(DBVAL_GET_INT16(pV)); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT32) { r = static_cast<double>(DBVAL_GET_INT32(pV)); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_INT64) { r = static_cast<double>(DBVAL_GET_INT64(pV)); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_DATETIME) { r = static_cast<double>(DBVAL_GET_DATETIME(pV)); }\
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_FLOAT) { r = static_cast<double>(DBVAL_GET_FLOAT(pV)); } \
  else if PDB_CONSTEXPR(DType == PDB_VALUE_TYPE::VAL_DOUBLE) { r = DBVAL_GET_DOUBLE(pV); } \
} while (false)
