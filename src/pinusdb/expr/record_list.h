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
#include "expr/expr_value.h"
#include "expr/target_list.h"
#include "expr/pdb_db_int.h"

class RecordList
{
public:
  RecordList() {}
  ~RecordList()
  {
    for (auto iter = recVec_.begin(); iter != recVec_.end(); iter++)
    {
      delete *iter;
    }
  }

  const std::vector<ExprValueList*>& GetRecList() const { return recVec_; }

  static RecordList* AppendRecordList(RecordList* pRecList, ExprValueList* pRec)
  {
    if (pRecList == nullptr)
    {
      pRecList = new RecordList();
    }

    if (pRec != nullptr)
      pRecList->recVec_.push_back(pRec);

    return pRecList;
  }

  static void FreeRecordList(RecordList* pRecList)
  {
    if (pRecList != nullptr)
    {
      delete pRecList;
    }
  }

private:
  std::vector<ExprValueList*> recVec_;
};


