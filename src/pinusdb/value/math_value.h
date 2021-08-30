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

#ifdef _WIN32
#define ABS_FUNC(val)  std::abs(val)
#else
#define ABS_FUNC(val)  abs(val)
#endif

#define _GET_ABS_FOR_DATATYPE(pDest, pSrc, DataType) do { \
  if PDB_CONSTEXPR(DataType == PDB_VALUE_TYPE::VAL_INT8) { DBVAL_SET_INT8((pDest), ABS_FUNC(DBVAL_GET_INT8(pSrc))); }  \
  else if PDB_CONSTEXPR(DataType == PDB_VALUE_TYPE::VAL_INT16) { DBVAL_SET_INT16((pDest), ABS_FUNC(DBVAL_GET_INT16(pSrc))); }  \
  else if PDB_CONSTEXPR(DataType == PDB_VALUE_TYPE::VAL_INT32) { DBVAL_SET_INT32((pDest), ABS_FUNC(DBVAL_GET_INT32(pSrc))); }  \
  else if PDB_CONSTEXPR(DataType == PDB_VALUE_TYPE::VAL_INT64) { DBVAL_SET_INT64((pDest), ABS_FUNC(DBVAL_GET_INT64(pSrc))); }  \
  else if PDB_CONSTEXPR(DataType == PDB_VALUE_TYPE::VAL_FLOAT) { DBVAL_SET_FLOAT((pDest), ABS_FUNC(DBVAL_GET_FLOAT(pSrc))); }  \
  else if PDB_CONSTEXPR(DataType == PDB_VALUE_TYPE::VAL_DOUBLE) { DBVAL_SET_DOUBLE((pDest), ABS_FUNC(DBVAL_GET_DOUBLE(pSrc))); }  \
} while (false)

//¾ø¶ÔÖµº¯Êý
template<PDB_VALUE_TYPE ValueType>
class AbsFunction : public ValueItem
{
public:
  AbsFunction(ValueItem* pValue)
  {
    this->pValue_ = pValue;
  }

  virtual ~AbsFunction()
  {
    if (pValue_ != nullptr)
      delete pValue_;
  }

  PDB_VALUE_TYPE GetValueType() const override
  {
    return ValueType;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_IS_NULL(&tmpVal))
    {
      DBVAL_SET_NULL(pResult);
      return PdbE_OK;
    }

    _GET_ABS_FOR_DATATYPE(pResult, &tmpVal, ValueType);
    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    PdbErr_t retVal = PdbE_OK;
    size_t resultSize = blockValues.GetResultSize();
    std::vector<DBVal> valVec;
    valVec.reserve(resultSize);
    retVal = pValue_->GetValueArray(blockValues, valVec);
    if (retVal != PdbE_OK)
      return retVal;

    if (valVec.size() != resultSize)
      return PdbE_INVALID_PARAM;

    DBVal tmpVal;
    DBVal nullVal;
    DBVAL_SET_NULL(&nullVal);
    const DBVal* pVals = valVec.data();

    for (size_t idx = 0; idx < resultSize; idx++)
    {
      if (DBVAL_ELE_IS_NULL(pVals, idx))
      {
        resultVec.push_back(nullVal);
      }
      else
      {
        _GET_ABS_FOR_DATATYPE(&tmpVal, (pVals + idx), ValueType);
        resultVec.push_back(tmpVal);
      }
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pValue_ == nullptr)
      return false;

    int32_t resultType = pValue_->GetValueType();
    if (resultType != ValueType)
      return false;

    if (PDB_TYPE_IS_NUMBER(ValueType))
      return true;

    if (PDB_TYPE_IS_FLOAT_OR_DOUBLE(ValueType))
      return true;

    return false;
  }

  bool IsConstValue() const override
  {
    return pValue_->IsConstValue();
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    if (pValue_ != nullptr)
      pValue_->GetUseFields(fieldSet);
  }

private:
  ValueItem* pValue_;
};

#undef _GET_ABS_FOR_DATATYPE
#undef ABS_FUNC

ValueItem* CreateAbsFunction(ValueItem* pValue)
{
  PDB_VALUE_TYPE valueType = pValue->GetValueType();
  switch (valueType)
  {
  case PDB_VALUE_TYPE::VAL_INT8: 
    return new AbsFunction<PDB_VALUE_TYPE::VAL_INT8>(pValue);
  case PDB_VALUE_TYPE::VAL_INT16:
    return new AbsFunction<PDB_VALUE_TYPE::VAL_INT16>(pValue);
  case PDB_VALUE_TYPE::VAL_INT32:
    return new AbsFunction<PDB_VALUE_TYPE::VAL_INT32>(pValue);
  case PDB_VALUE_TYPE::VAL_INT64:
    return new AbsFunction<PDB_VALUE_TYPE::VAL_INT64>(pValue);
  case PDB_VALUE_TYPE::VAL_FLOAT:
    return new AbsFunction<PDB_VALUE_TYPE::VAL_FLOAT>(pValue);
  case PDB_VALUE_TYPE::VAL_DOUBLE:
    return new AbsFunction<PDB_VALUE_TYPE::VAL_DOUBLE>(pValue);
  }

  return nullptr;
}


