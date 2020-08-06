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
#include "pdb_error.h"

ResultObject::ResultObject(const std::vector<QueryField*>& fieldVec,
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

PdbErr_t ResultObject::GetRecord(DBVal* pVals, size_t valCnt)
{
  if (fieldVec_.size() != valCnt)
    return PdbE_INVALID_PARAM;

  for (size_t idx = 0; idx < valCnt; idx++)
  {
    fieldVec_[idx]->GetResult((pVals + idx));
  }

  return PdbE_OK;
}
