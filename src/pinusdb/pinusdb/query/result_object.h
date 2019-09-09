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

#pragma once

#include "query/result_field.h"
#include "expr/expr_item.h"
#include "expr/sql_parser.h"
#include "table/data_column.h"
#include "table/db_obj.h"
#include "query/data_table.h"
#include <string>
#include <vector>

class ResultObject
{
public:
  ResultObject(const std::vector<ResultField*>& fieldVec, 
    int64_t devId, int64_t tstamp);
  virtual ~ResultObject();

  PdbErr_t AppendData(const DBVal* pVals, size_t valCnt);

  PdbErr_t GetResultObj(DBObj* pObj) const;

private:
  std::vector<ResultField*> fieldVec_;
};

