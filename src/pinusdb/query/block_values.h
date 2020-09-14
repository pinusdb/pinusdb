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

#include "internal.h"
#include "table/db_value.h"
#include <vector>

class BlockValues
{
public:
  BlockValues(size_t columnSize)
  {
    columnVec_.resize(columnSize);
    Reset();
  }

  void Reset()
  {
    recordSize_ = 0;
    resultSize_ = 0;
    filterVec_.clear();
    for (size_t idx = 0; idx < columnVec_.size(); idx++)
      columnVec_[idx] = nullptr;
  }

  PdbErr_t MergeFilter(std::vector<DBVal>& filterVec)
  {
    if (filterVec.size() != resultSize_)
      return PdbE_INVALID_PARAM;

    filterVec_.reserve(recordSize_);
    const DBVal* pTmpVal = filterVec.data();
    const DBVal* pTmpValEnd = pTmpVal + filterVec.size();
    size_t tmpResultSize = 0;
    if (filterVec_.empty())
    {
      while (pTmpVal < pTmpValEnd)
      {
        if (DBVAL_IS_BOOL(pTmpVal) && DBVAL_GET_BOOL(pTmpVal))
        {
          filterVec_.push_back(1);
          tmpResultSize++;
        }
        else
        {
          filterVec_.push_back(0);
        }

        pTmpVal++;
      }
    }
    else
    {
      uint8_t* pRetVal = filterVec_.data();
      uint8_t* pRetValEnd = pRetVal + filterVec_.size();
      while (pTmpVal < pTmpValEnd)
      {
        while (pRetVal < pRetValEnd && *pRetVal == 0) pRetVal++;

        if (pRetVal >= pRetValEnd)
          return PdbE_INVALID_PARAM;

        if (DBVAL_IS_BOOL(pTmpVal) && DBVAL_GET_BOOL(pTmpVal))
        {
          tmpResultSize++;
        }
        else
        {
          *pRetVal = 0;
        }

        pTmpVal++;
        pRetVal++;
      }
    }

    resultSize_ = tmpResultSize;
    return PdbE_OK;
  }

  void SetRecordSize(size_t recordSize) 
  {
    recordSize_ = recordSize; 
    resultSize_ = recordSize; 
    filterVec_.clear(); 
  }

  void SetColumnValues(size_t columnIdx, const DBVal* pVals) { columnVec_[columnIdx] = pVals; }
  size_t GetColumnSize() const { return columnVec_.size(); }
  size_t GetRecordSize() const { return recordSize_; }
  size_t GetResultSize() const { return resultSize_; }

  const uint8_t* GetFilter() const 
  { 
    if (filterVec_.empty()) 
      return nullptr; 
    else 
      return filterVec_.data(); 
  }

  const DBVal* GetColumnValues(size_t columnIdx) const { return columnVec_[columnIdx]; }

  PdbErr_t GetRecordValue(size_t recordIdx, std::vector<DBVal>& recordVec)
  {
    if (recordIdx >= resultSize_)
      return PdbE_INVALID_PARAM;

    size_t valIdx = recordIdx;
    if (!filterVec_.empty())
    {
      if (resultIdxVec_.size() != resultSize_)
      {
        resultIdxVec_.clear();
        resultIdxVec_.reserve(resultSize_);
        for (size_t idx = 0; idx < recordSize_; idx++)
        {
          if (filterVec_[idx] != PDB_BOOL_FALSE)
          {
            resultIdxVec_.push_back(idx);
          }
        }
      }

      valIdx = resultIdxVec_[recordIdx];
    }

    recordVec.clear();
    DBVal nullVal;
    DBVAL_SET_NULL(&nullVal);
    for (size_t idx = 0; idx < columnVec_.size(); idx++)
    {
      if (columnVec_[idx] == nullptr)
      {
        recordVec.push_back(nullVal);
      }
      else
      {
        recordVec.push_back(columnVec_[idx][valIdx]);
      }
    }

    return PdbE_OK;
  }

private:
  size_t recordSize_;
  size_t resultSize_;
  std::vector<size_t> resultIdxVec_;
  std::vector<uint8_t> filterVec_;
  std::vector<const DBVal*> columnVec_;
};
