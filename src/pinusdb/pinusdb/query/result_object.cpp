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

#include "query/result_object.h"
#include "expr/parse.h"
#include "util/string_tool.h"
#include "pdb_error.h"

ResultObject::ResultObject(const std::vector<ResultField*>& fieldVec,
  int64_t devId, int64_t tstamp)
{
  for (auto fieldIter = fieldVec.begin(); fieldIter != fieldVec.end(); fieldIter++)
  {
    fieldVec_.push_back((*fieldIter)->NewField(devId, tstamp));
  }
}

ResultObject::~ResultObject()
{
  for (auto fieldIt = fieldVec_.begin(); fieldIt != fieldVec_.end(); fieldIt++)
  {
    delete (*fieldIt);
  }
}

PdbErr_t ResultObject::AppendData(const DBVal* pVals, size_t valCnt)
{
  PdbErr_t retVal = PdbE_OK;

  for (auto fieldIt = fieldVec_.begin(); fieldIt != fieldVec_.end(); fieldIt++)
  {
    retVal = (*fieldIt)->AppendData(pVals, valCnt);
    if (retVal != PdbE_OK)
      return retVal;
  }

  return PdbE_OK;
}

PdbErr_t ResultObject::GetResultObj(DBObj* pObj) const
{
  if (pObj == nullptr)
    return PdbE_INVALID_PARAM;

  PdbErr_t retVal = PdbE_OK;

  DBVal dbVal;

  for (auto fieldIter = fieldVec_.begin(); fieldIter != fieldVec_.end(); fieldIter++)
  {
    retVal = (*fieldIter)->GetResult(&dbVal);
    if (retVal != PdbE_OK)
      return retVal;
  
    retVal = pObj->AppendVal(&dbVal);
    if (retVal != PdbE_OK)
      return retVal;
  }

  return PdbE_OK;
}
