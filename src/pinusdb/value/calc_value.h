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
#include "pdb.h"
#include "pdb_error.h"
#include "internal.h"
#include "expr/parse.h"
#include "value/value_item.h"
#include "table/db_value.h"

#define _GET_CALC_RESULT_INT64_(Op, r, v1, v2) do { \
  if PDB_CONSTEXPR(Op == PDB_SQL_FUNC::FUNC_ADD) { r = (v1) + (v2); } \
  else if PDB_CONSTEXPR(Op == PDB_SQL_FUNC::FUNC_SUB) { r = (v1) - (v2); } \
  else if PDB_CONSTEXPR(Op == PDB_SQL_FUNC::FUNC_MUL) { r = (v1) * (v2); } \
  else if PDB_CONSTEXPR(Op == PDB_SQL_FUNC::FUNC_DIV) { if (v2 != 0) { r = (v1) / (v2); } } \
  else if PDB_CONSTEXPR(Op == PDB_SQL_FUNC::FUNC_MOD) { if (v2 != 0) { r = (v1) % (v2); } }  \
} while (false)

#define _GET_CALC_RESULT_DOUBLE_(Op, r, v1, v2) do { \
  if PDB_CONSTEXPR(Op == PDB_SQL_FUNC::FUNC_ADD) { r = (v1) + (v2); } \
  else if PDB_CONSTEXPR(Op == PDB_SQL_FUNC::FUNC_SUB) { r = (v1) - (v2); } \
  else if PDB_CONSTEXPR(Op == PDB_SQL_FUNC::FUNC_MUL) { r = (v1) * (v2); } \
  else if PDB_CONSTEXPR(Op == PDB_SQL_FUNC::FUNC_DIV) { if (!DOUBLE_EQUAL_ZERO(v2)) { r = (v1) / (v2); } } \
} while (false)


template<PDB_SQL_FUNC Op, PDB_VALUE_TYPE ResultType, PDB_VALUE_TYPE LeftType, PDB_VALUE_TYPE RightType>
class CalcValueItem : public ValueItem
{
public:
  CalcValueItem(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~CalcValueItem()
  {
    if (pLeft_ != nullptr)
    {
      delete pLeft_;
    }

    if (pRight_ != nullptr)
    {
      delete pRight_;
    }
  }

  PDB_VALUE_TYPE GetValueType() const override
  {
    return ResultType;
  }

  PdbErr_t GetValue(const DBVal* pRecord, DBVal* pResult) const override
  {
    PdbErr_t retVal = PdbE_OK;
    DBVal leftVal, rightVal;
    retVal = pLeft_->GetValue(pRecord, &leftVal);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

    retVal = pRight_->GetValue(pRecord, &rightVal);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

    if (DBVAL_GET_TYPE(&leftVal) != LeftType || DBVAL_GET_TYPE(&rightVal) != RightType)
    {
      DBVAL_SET_NULL(pResult);
      return PdbE_OK;
    }

    if PDB_CONSTEXPR(ResultType == PDB_VALUE_TYPE::VAL_INT64)
    {
      int64_t i64V1 = 0;
      int64_t i64V2 = 0;
      int64_t i64R = 0;
      _GET_INT64_BY_DATATYPE_SINGLE(&leftVal, i64V1, LeftType);
      _GET_INT64_BY_DATATYPE_SINGLE(&rightVal, i64V2, RightType);
      _GET_CALC_RESULT_INT64_(Op, i64R, i64V1, i64V2);
      if (i64V2 == 0 && (Op == PDB_SQL_FUNC::FUNC_DIV || Op == PDB_SQL_FUNC::FUNC_MOD))
      {
        DBVAL_SET_NULL(pResult);
      }
      else
      {
        DBVAL_SET_INT64(pResult, i64R);
      }
    }
    else if PDB_CONSTEXPR(ResultType == PDB_VALUE_TYPE::VAL_DOUBLE)
    {
      double dV1 = 0;
      double dV2 = 0;
      double dR = 0;
      _GET_DOUBLE_BY_DATATYPE_SINGLE(&leftVal, dV1, LeftType);
      _GET_DOUBLE_BY_DATATYPE_SINGLE(&rightVal, dV2, RightType);
      _GET_CALC_RESULT_DOUBLE_(Op, dR, dV1, dV2);
      if (DOUBLE_EQUAL_ZERO(dV2) && (Op == PDB_SQL_FUNC::FUNC_DIV || Op == PDB_SQL_FUNC::FUNC_MOD))
      {
        DBVAL_SET_NULL(pResult);
      }
      else
      {
        DBVAL_SET_DOUBLE(pResult, dR);
      }
    }

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> leftValVec;
    std::vector<DBVal> rightValVec;
    leftValVec.reserve(resultSize);
    rightValVec.reserve(resultSize);
    retVal = pLeft_->GetValueArray(blockValues, leftValVec);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

    retVal = pRight_->GetValueArray(blockValues, rightValVec);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

#ifdef _DEBUG
    if (leftValVec.size() != resultSize || rightValVec.size() != resultSize)
    {
      return PdbE_INVALID_PARAM;
    }
#endif

    DBVal tmpVal;
    const DBVal* pLeftVals = leftValVec.data();
    const DBVal* pRightVals = rightValVec.data();

    int64_t i64V1 = 0;
    int64_t i64V2 = 0;
    int64_t i64R = 0;
    double dV1 = 0;
    double dV2 = 0;
    double dR = 0;

    for (size_t idx = 0; idx < resultSize; idx++)
    {
      DBVAL_SET_NULL(&tmpVal);
      if (DBVAL_ELE_GET_TYPE(pLeftVals, idx) == LeftType && DBVAL_ELE_GET_TYPE(pRightVals, idx) == RightType)
      {
        if PDB_CONSTEXPR(ResultType == PDB_VALUE_TYPE::VAL_INT64)
        {
          _GET_INT64_BY_DATATYPE_ARRAY(pLeftVals, idx, i64V1, LeftType);
          _GET_INT64_BY_DATATYPE_ARRAY(pRightVals, idx, i64V2, RightType);
          _GET_CALC_RESULT_INT64_(Op, i64R, i64V1, i64V2);
          if (i64V2 == 0 && (Op == PDB_SQL_FUNC::FUNC_DIV || Op == PDB_SQL_FUNC::FUNC_MOD))
          {
            DBVAL_SET_NULL(&tmpVal);
          }
          else
          {
            DBVAL_SET_INT64(&tmpVal, i64R);
          }
        }
        else if PDB_CONSTEXPR(ResultType == PDB_VALUE_TYPE::VAL_DOUBLE)
        {
          _GET_DOUBLE_BY_DATATYPE_ARRAY(pLeftVals, idx, dV1, LeftType);
          _GET_DOUBLE_BY_DATATYPE_ARRAY(pRightVals, idx, dV2, RightType);
          _GET_CALC_RESULT_DOUBLE_(Op, dR, dV1, dV2);
          if (DOUBLE_EQUAL_ZERO(dV2) && (Op == PDB_SQL_FUNC::FUNC_DIV))
          {
            DBVAL_SET_NULL(&tmpVal);
          }
          else
          {
            DBVAL_SET_DOUBLE(&tmpVal, dR);
          }
        }
      }

      resultVec.push_back(tmpVal);
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pLeft_ == nullptr || pRight_ == nullptr)
    {
      return false;
    }

    if ((!pLeft_->IsValid()) || (!pRight_->IsValid()))
    {
      return false;
    }

    if (LeftType != pLeft_->GetValueType())
    {
      return false;
    }

    if (RightType != pRight_->GetValueType())
    {
      return false;
    }

    if (ResultType == PDB_VALUE_TYPE::VAL_NULL)
    {
      return false;
    }

    if (ResultType != GetCalcType(Op, LeftType, RightType))
    {
      return false;
    }

    return true;
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pLeft_ != nullptr)
    {
      pLeft_->GetUseFields(fieldSet);
    }

    if (pRight_ != nullptr)
    {
      pRight_->GetUseFields(fieldSet);
    }
  }

private:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

#define _CONVERT_TO_COMMONTYPE(pD, pS, DestType, SrcType) do { \
  if PDB_CONSTEXPR(DestType != SrcType) { \
    if ((PDB_VALUE_TYPE)DBVAL_GET_TYPE(pS) == SrcType ) { \
      if PDB_CONSTEXPR(DestType == PDB_VALUE_TYPE::VAL_INT64) { \
        if PDB_CONSTEXPR(SrcType == PDB_VALUE_TYPE::VAL_INT8) { DBVAL_SET_INT64(pD, DBVAL_GET_INT8(pS));  break; }  \
        if PDB_CONSTEXPR(SrcType == PDB_VALUE_TYPE::VAL_INT16) { DBVAL_SET_INT64(pD, DBVAL_GET_INT16(pS)); break; }  \
        if PDB_CONSTEXPR(SrcType == PDB_VALUE_TYPE::VAL_INT32) { DBVAL_SET_INT64(pD, DBVAL_GET_INT32(pS)); break; }  \
      }  \
      else if PDB_CONSTEXPR(DestType == PDB_VALUE_TYPE::VAL_DOUBLE) { \
        if PDB_CONSTEXPR(SrcType == PDB_VALUE_TYPE::VAL_INT8) { DBVAL_SET_DOUBLE(pD, DBVAL_GET_INT8(pS)); break; }  \
        if PDB_CONSTEXPR(SrcType == PDB_VALUE_TYPE::VAL_INT16) { DBVAL_SET_DOUBLE(pD, DBVAL_GET_INT16(pS)); break; }  \
        if PDB_CONSTEXPR(SrcType == PDB_VALUE_TYPE::VAL_INT32) { DBVAL_SET_DOUBLE(pD, DBVAL_GET_INT32(pS)); break; }  \
        if PDB_CONSTEXPR(SrcType == PDB_VALUE_TYPE::VAL_INT64) { DBVAL_SET_DOUBLE(pD, DBVAL_GET_INT64(pS)); break; }  \
        if PDB_CONSTEXPR(SrcType == PDB_VALUE_TYPE::VAL_FLOAT) { DBVAL_SET_DOUBLE(pD, DBVAL_GET_FLOAT(pS)); break; }  \
      }  \
    } \
  } \
  *pD = *pS;  \
} while(false)

template<PDB_VALUE_TYPE ResultType, PDB_VALUE_TYPE LeftType, PDB_VALUE_TYPE RightType>
class IfFunction : public ValueItem
{
public:
  IfFunction(ValueItem* pCondition, ValueItem* pResult0, ValueItem* pResult1)
  {
    this->pCondition_ = pCondition;
    this->pResult0_ = pResult0;
    this->pResult1_ = pResult1;
  }

  virtual ~IfFunction()
  {
    if (pCondition_ != nullptr)
    {
      delete pCondition_;
    }

    if (pResult0_ != nullptr)
    {
      delete pResult0_;
    }

    if (pResult1_ != nullptr)
    {
      delete pResult1_;
    }
  }

  PDB_VALUE_TYPE GetValueType() const override
  {
    return ResultType;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal = PdbE_OK;
    DBVal condiVal, rst0Val, rst1Val;
    retVal = pCondition_->GetValue(pVals, &condiVal);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

    retVal = pResult0_->GetValue(pVals, &rst0Val);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

    retVal = pResult1_->GetValue(pVals, &rst1Val);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

    if (pResult == nullptr)
    {
      return PdbE_OK;
    }

    int64_t i64Val = 0;
    double dVal = 0;
    if (DBVAL_IS_BOOL(&condiVal) && DBVAL_GET_BOOL(&condiVal))
    {
      _CONVERT_TO_COMMONTYPE(pResult, &rst0Val, ResultType, LeftType);
    }
    else
    {
      _CONVERT_TO_COMMONTYPE(pResult, &rst1Val, ResultType, RightType);
    }

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> condiVec;
    std::vector<DBVal> param0ValVec;
    std::vector<DBVal> param1ValVec;
    condiVec.reserve(resultSize);
    param0ValVec.reserve(resultSize);
    param1ValVec.reserve(resultSize);

    retVal = pCondition_->GetValueArray(blockValues, condiVec);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

    if (condiVec.size() != resultSize)
    {
      return PdbE_INVALID_PARAM;
    }

    retVal = pResult0_->GetValueArray(blockValues, param0ValVec);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

    if (param0ValVec.size() != resultSize)
    {
      return PdbE_INVALID_PARAM;
    }

    retVal = pResult1_->GetValueArray(blockValues, param1ValVec);
    if (retVal != PdbE_OK)
    {
      return retVal;
    }

    if (param1ValVec.size() != resultSize)
    {
      return PdbE_INVALID_PARAM;
    }

    const DBVal* pCondiVals = condiVec.data();
    const DBVal* pLeftVals = param0ValVec.data();
    const DBVal* pRightVals = param1ValVec.data();
    DBVal tmpRet;

    for (size_t idx = 0; idx < resultSize; idx++)
    {
      if (DBVAL_ELE_IS_BOOL(pCondiVals, idx) && DBVAL_ELE_GET_BOOL(pCondiVals, idx))
      {
        _CONVERT_TO_COMMONTYPE(&tmpRet, (pLeftVals + idx), ResultType, LeftType);
      }
      else
      {
        _CONVERT_TO_COMMONTYPE(&tmpRet, (pRightVals + idx), ResultType, RightType);
      }
      resultVec.push_back(tmpRet);
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pCondition_ == nullptr || pResult0_ == nullptr || pResult1_ == nullptr)
    {
      return false;
    }

    if ((!pCondition_->IsValid()) || (!pResult0_->IsValid()) || (!pResult1_->IsValid()))
    {
      return false;
    }

    if (pCondition_->GetValueType() != PDB_VALUE_TYPE::VAL_BOOL)
    {
      return false;
    }

    if (pResult0_->GetValueType() != LeftType)
    {
      return false;
    }

    if (pResult1_->GetValueType() != RightType)
    {
      return false;
    }

    if (ResultType == PDB_VALUE_TYPE::VAL_NULL)
    {
      return false;
    }

    if (ResultType != GetCommonType(LeftType, RightType))
    {
      return false;
    }

    return true;
  }

  bool IsConstValue() const override
  {
    return pCondition_->IsConstValue()
      && pResult0_->IsConstValue() && pResult1_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pCondition_ != nullptr)
    {
      pCondition_->GetUseFields(fieldSet);
    }

    if (pResult0_ != nullptr)
    {
      pResult0_->GetUseFields(fieldSet);
    }

    if (pResult1_ != nullptr)
    {
      pResult1_->GetUseFields(fieldSet);
    }
  }

private:
  ValueItem* pCondition_;
  ValueItem* pResult0_;
  ValueItem* pResult1_;
};

#undef _GET_CALC_RESULT_
#undef _CONVERT_TO_COMMONTYPE

template<PDB_VALUE_TYPE ResultType, PDB_VALUE_TYPE LeftType, PDB_VALUE_TYPE RightType>
ValueItem* _CreateCalcFunction3(PDB_SQL_FUNC op, ValueItem* pLeft, ValueItem* pRight)
{
  switch (op)
  {
  case PDB_SQL_FUNC::FUNC_ADD:
    return new CalcValueItem<PDB_SQL_FUNC::FUNC_ADD, ResultType, LeftType, RightType>(pLeft, pRight);
  case PDB_SQL_FUNC::FUNC_SUB:
    return new CalcValueItem<PDB_SQL_FUNC::FUNC_SUB, ResultType, LeftType, RightType>(pLeft, pRight);
  case PDB_SQL_FUNC::FUNC_MUL:
    return new CalcValueItem<PDB_SQL_FUNC::FUNC_MUL, ResultType, LeftType, RightType>(pLeft, pRight);
  case PDB_SQL_FUNC::FUNC_DIV:
    return new CalcValueItem<PDB_SQL_FUNC::FUNC_DIV, ResultType, LeftType, RightType>(pLeft, pRight);
  case PDB_SQL_FUNC::FUNC_MOD:
    return new CalcValueItem<PDB_SQL_FUNC::FUNC_MOD, ResultType, LeftType, RightType>(pLeft, pRight);
  }
  return nullptr;
}

template<PDB_VALUE_TYPE LeftType, PDB_VALUE_TYPE RightType>
ValueItem* _CreateCalcFunction2(PDB_SQL_FUNC op, ValueItem* pLeft, ValueItem* pRight)
{
  PDB_VALUE_TYPE calcResultType = GetCalcType(op, LeftType, RightType);
  if (calcResultType == PDB_VALUE_TYPE::VAL_INT64)
  {
    return _CreateCalcFunction3<PDB_VALUE_TYPE::VAL_INT64, LeftType, RightType>(op, pLeft, pRight);
  }
  else if (calcResultType == PDB_VALUE_TYPE::VAL_DOUBLE)
  {
    return _CreateCalcFunction3<PDB_VALUE_TYPE::VAL_DOUBLE, LeftType, RightType>(op, pLeft, pRight);
  }

  return nullptr;
}

template<PDB_VALUE_TYPE RightType>
ValueItem* _CreateCalcFunction1(PDB_SQL_FUNC op, ValueItem* pLeft, ValueItem* pRight)
{
  PDB_VALUE_TYPE leftType = pLeft->GetValueType();
  switch (leftType)
  {
  case PDB_VALUE_TYPE::VAL_INT8: 
    return _CreateCalcFunction2<PDB_VALUE_TYPE::VAL_INT8, RightType>(op, pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT16:
    return _CreateCalcFunction2<PDB_VALUE_TYPE::VAL_INT16, RightType>(op, pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT32:
    return _CreateCalcFunction2<PDB_VALUE_TYPE::VAL_INT32, RightType>(op, pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT64:
    return _CreateCalcFunction2<PDB_VALUE_TYPE::VAL_INT64, RightType>(op, pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_FLOAT:
    return _CreateCalcFunction2<PDB_VALUE_TYPE::VAL_FLOAT, RightType>(op, pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return _CreateCalcFunction2<PDB_VALUE_TYPE::VAL_DOUBLE, RightType>(op, pLeft, pRight);
  }

  return nullptr;
}

ValueItem* CreateCalcFunction(PDB_SQL_FUNC op, ValueItem* pLeft, ValueItem* pRight)
{
  PDB_VALUE_TYPE rightType = pRight->GetValueType();
  switch (rightType)
  {
  case PDB_VALUE_TYPE::VAL_INT8:
    return _CreateCalcFunction1<PDB_VALUE_TYPE::VAL_INT8>(op, pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT16:
    return _CreateCalcFunction1<PDB_VALUE_TYPE::VAL_INT16>(op, pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT32:
    return _CreateCalcFunction1<PDB_VALUE_TYPE::VAL_INT32>(op, pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_INT64:
    return _CreateCalcFunction1<PDB_VALUE_TYPE::VAL_INT64>(op, pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_FLOAT:
    return _CreateCalcFunction1<PDB_VALUE_TYPE::VAL_FLOAT>(op, pLeft, pRight);
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return _CreateCalcFunction1<PDB_VALUE_TYPE::VAL_DOUBLE>(op, pLeft, pRight);
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////

template<PDB_VALUE_TYPE ResultType, PDB_VALUE_TYPE LeftType>
ValueItem* _CreateIfFunction2(ValueItem* pCondition, ValueItem* pResult0, ValueItem* pResult1)
{
  PDB_VALUE_TYPE rightType = pResult1->GetValueType();
  switch (rightType)
  {
  case PDB_VALUE_TYPE::VAL_BOOL:
    return new IfFunction<ResultType, LeftType, PDB_VALUE_TYPE::VAL_BOOL>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT8:
    return new IfFunction<ResultType, LeftType, PDB_VALUE_TYPE::VAL_INT8>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT16:
    return new IfFunction<ResultType, LeftType, PDB_VALUE_TYPE::VAL_INT16>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT32:
    return new IfFunction<ResultType, LeftType, PDB_VALUE_TYPE::VAL_INT32>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT64:
    return new IfFunction<ResultType, LeftType, PDB_VALUE_TYPE::VAL_INT64>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return new IfFunction<ResultType, LeftType, PDB_VALUE_TYPE::VAL_DATETIME>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_FLOAT:
    return new IfFunction<ResultType, LeftType, PDB_VALUE_TYPE::VAL_FLOAT>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return new IfFunction<ResultType, LeftType, PDB_VALUE_TYPE::VAL_DOUBLE>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_STRING:
    return new IfFunction<ResultType, LeftType, PDB_VALUE_TYPE::VAL_STRING>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_BLOB:
    return new IfFunction<ResultType, LeftType, PDB_VALUE_TYPE::VAL_BLOB>(pCondition, pResult0, pResult1);
  }

  return nullptr;
}

template<PDB_VALUE_TYPE ResultType>
ValueItem* _CreateIfFunction1(ValueItem* pCondition, ValueItem* pResult0, ValueItem* pResult1)
{
  PDB_VALUE_TYPE leftType = pResult0->GetValueType();
  switch (leftType)
  {
  case PDB_VALUE_TYPE::VAL_BOOL:
    return _CreateIfFunction2<ResultType, PDB_VALUE_TYPE::VAL_BOOL>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT8:
    return _CreateIfFunction2<ResultType, PDB_VALUE_TYPE::VAL_INT8>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT16:
    return _CreateIfFunction2<ResultType, PDB_VALUE_TYPE::VAL_INT16>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT32:
    return _CreateIfFunction2<ResultType, PDB_VALUE_TYPE::VAL_INT32>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT64:
    return _CreateIfFunction2<ResultType, PDB_VALUE_TYPE::VAL_INT64>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return _CreateIfFunction2<ResultType, PDB_VALUE_TYPE::VAL_DATETIME>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_FLOAT:
    return _CreateIfFunction2<ResultType, PDB_VALUE_TYPE::VAL_FLOAT>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return _CreateIfFunction2<ResultType, PDB_VALUE_TYPE::VAL_DOUBLE>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_STRING:
    return _CreateIfFunction2<ResultType, PDB_VALUE_TYPE::VAL_STRING>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_BLOB:
    return _CreateIfFunction2<ResultType, PDB_VALUE_TYPE::VAL_BLOB>(pCondition, pResult0, pResult1);
  }

  return nullptr;
}

ValueItem* CreateIfFunction(ValueItem* pCondition, ValueItem* pResult0, ValueItem* pResult1)
{
  if (pCondition->GetValueType() != PDB_VALUE_TYPE::VAL_BOOL)
  {
    return nullptr;
  }

  PDB_VALUE_TYPE leftType = pResult0->GetValueType();
  PDB_VALUE_TYPE rightType = pResult1->GetValueType();
  PDB_VALUE_TYPE resultType = GetCommonType(leftType, rightType);
  switch (resultType)
  {
  case PDB_VALUE_TYPE::VAL_BOOL:
    return _CreateIfFunction1<PDB_VALUE_TYPE::VAL_BOOL>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT8:
    return _CreateIfFunction1<PDB_VALUE_TYPE::VAL_INT8>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT16:
    return _CreateIfFunction1<PDB_VALUE_TYPE::VAL_INT16>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT32:
    return _CreateIfFunction1<PDB_VALUE_TYPE::VAL_INT32>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_INT64:
    return _CreateIfFunction1<PDB_VALUE_TYPE::VAL_INT64>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_DATETIME:
    return _CreateIfFunction1<PDB_VALUE_TYPE::VAL_DATETIME>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_FLOAT:
    return _CreateIfFunction1<PDB_VALUE_TYPE::VAL_FLOAT>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return _CreateIfFunction1<PDB_VALUE_TYPE::VAL_DOUBLE>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_STRING:
    return _CreateIfFunction1<PDB_VALUE_TYPE::VAL_STRING>(pCondition, pResult0, pResult1);
  case PDB_VALUE_TYPE::VAL_BLOB:
    return _CreateIfFunction1<PDB_VALUE_TYPE::VAL_BLOB>(pCondition, pResult0, pResult1);
  }

  return nullptr;
}
