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

#include <string>
#include <vector>
#include "expr/sql_parser.h"
#include "query/result_object.h"
#include "query/condition_filter.h"
#include "util/arena.h"
#include "table/table_info.h"
#include "query/block_values.h"
#include "util/coding.h"

class IQuery
{
public:
  IQuery() {}
  virtual ~IQuery() {}

  virtual PdbErr_t AppendSingle(const DBVal* pVals, size_t valCnt, bool* pIsAdded) = 0;
  virtual PdbErr_t AppendArray(BlockValues& blockValues, bool* pIsAdded) = 0;
  virtual bool GetIsFullFlag() const = 0;
  virtual PdbErr_t GetResult(std::string& dataBuf, uint32_t* pFieldCnt, uint32_t* pRecordCnt) = 0;

  virtual void GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const = 0;
  virtual void GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const = 0;
  virtual bool FilterDevId(int64_t devId) const = 0;
  virtual bool IsEmptySet() const = 0;
  virtual size_t GetQueryOffset() const = 0;
  virtual size_t GetQueryRecord() const = 0;
  virtual void GetUseFields(std::unordered_set<size_t>& fieldSet) const = 0;
  virtual PdbErr_t BuildQuery(const QueryParam* pQueryParam, const TableInfo* pTabInfo) = 0;

  //snapshot สนำร
  virtual int64_t GetMaxDevId() const { return INT64_MAX; }

protected:
  void PushBackToResult(std::string& resultData,DBVal val)
  {
    resultData.push_back(static_cast<char>(DBVAL_GET_TYPE(&val)));
    switch (DBVAL_GET_TYPE(&val))
    {
    case PDB_VALUE_TYPE::VAL_NULL:
      break;
    case PDB_VALUE_TYPE::VAL_BOOL:
      resultData.push_back(static_cast<char>(DBVAL_GET_BOOL(&val) ? PDB_BOOL_TRUE : PDB_BOOL_FALSE));
      break;
    case PDB_VALUE_TYPE::VAL_INT8:
      Coding::PutVarint64(&resultData, Coding::ZigzagEncode64(DBVAL_GET_INT8(&val)));
      break;
    case PDB_VALUE_TYPE::VAL_INT16:
      Coding::PutVarint64(&resultData, Coding::ZigzagEncode64(DBVAL_GET_INT16(&val)));
      break;
    case PDB_VALUE_TYPE::VAL_INT32:
      Coding::PutVarint64(&resultData, Coding::ZigzagEncode64(DBVAL_GET_INT32(&val)));
      break;
    case PDB_VALUE_TYPE::VAL_INT64:
      Coding::PutVarint64(&resultData, Coding::ZigzagEncode64(DBVAL_GET_INT64(&val)));
      break;
    case PDB_VALUE_TYPE::VAL_DATETIME:
      Coding::PutVarint64(&resultData, Coding::ZigzagEncode64(DBVAL_GET_DATETIME(&val)));
      break;
    case PDB_VALUE_TYPE::VAL_FLOAT:
      Coding::PutFixed32(&resultData, Coding::EncodeFloat(DBVAL_GET_FLOAT(&val)));
      break;
    case PDB_VALUE_TYPE::VAL_DOUBLE:
      Coding::PutFixed64(&resultData, Coding::EncodeDouble(DBVAL_GET_DOUBLE(&val)));
      break;
    case PDB_VALUE_TYPE::VAL_STRING:
    case PDB_VALUE_TYPE::VAL_BLOB:
      Coding::PutVarint64(&resultData, static_cast<uint64_t>(DBVAL_GET_LEN(&val)));
      resultData.append(DBVAL_GET_STRING(&val), DBVAL_GET_LEN(&val));
      break;
    }

  }
};


