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

//×Ö¶ÎÖµ
template<PDB_VALUE_TYPE FieldType>
class FieldValue : public ValueItem
{
public:
  FieldValue(size_t fieldPos)
  {
    fieldPos_ = fieldPos;
  }

  virtual ~FieldValue()
  {}

  PDB_VALUE_TYPE GetValueType() const override
  {
    return FieldType;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pVals == nullptr)
      return PdbE_INVALID_PARAM;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_ELE_GET_TYPE(pVals, fieldPos_) == FieldType)
    {
      *pResult = pVals[fieldPos_];
    }
    else
    {
      DBVAL_SET_NULL(pResult);
    }

    return PdbE_OK;
  }

  PdbErr_t GetValueArray(const BlockValues& blockValues, std::vector<DBVal>& resultVec) const override
  {
    size_t recordSize = blockValues.GetRecordSize();
    size_t resultSize = blockValues.GetResultSize();
    const uint8_t* pFilter = blockValues.GetFilter();
    const DBVal* pVals = blockValues.GetColumnValues(fieldPos_);

    if (resultSize == recordSize)
    {
      pFilter = nullptr;
    }

    DBVal nullVal;
    DBVAL_SET_NULL(&nullVal);

    if (pVals == nullptr)
    {
      for (size_t idx = 0; idx < resultSize; idx++)
      {
        resultVec.push_back(nullVal);
      }
    }
    else
    {
      if (pFilter == nullptr)
      {
        for (size_t idx = 0; idx < recordSize; idx++)
        {
          resultVec.push_back(pVals[idx]);
        }
      }
      else
      {
        for (size_t idx = 0; idx < recordSize; idx++)
        {
          if (pFilter[idx] != PDB_BOOL_FALSE)
          {
            resultVec.push_back(pVals[idx]);
          }
        }
      }
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    return true;
  }

  bool IsConstValue() const override
  {
    return false;
  }

  void GetUseFields(std::unordered_set<size_t>& fieldSet) const override
  {
    fieldSet.insert(fieldPos_);
  }

private:
  size_t fieldPos_;
};

