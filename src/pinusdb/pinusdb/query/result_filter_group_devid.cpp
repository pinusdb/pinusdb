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

#include "query/result_filter_group_devid.h"
#include "expr/parse.h"
#include "util/string_tool.h"

ResultFilterGroupDevID::ResultFilterGroupDevID()
{
  this->lastGroupId_ = 0;
  this->pLastObj_ = nullptr;
}

ResultFilterGroupDevID::~ResultFilterGroupDevID()
{
}

PdbErr_t ResultFilterGroupDevID::InitGrpDevResult(const std::list<int64_t>& devIdList)
{
  ResultObject* pRstObj = nullptr;

  for (auto devIdIter = devIdList.begin(); devIdIter != devIdList.end(); devIdIter++)
  {
    pRstObj = new ResultObject(fieldVec_, *devIdIter, 0);
    if (pRstObj == nullptr)
      return PdbE_NOMEM;

    objVec_.push_back(pRstObj);
    objMap_.insert(std::pair<uint64_t, ResultObject*>((uint64_t)*devIdIter, pRstObj));
  }

  return PdbE_OK;
}

PdbErr_t ResultFilterGroupDevID::AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded)
{
  bool resultVal = true;
  PdbErr_t retVal = PdbE_OK;

  int64_t devId = DBVAL_ELE_GET_INT64(pVals, PDB_DEVID_INDEX); 

  uint64_t groupId = (uint64_t)devId;
  ResultObject* pRstObj = nullptr;

  if (pIsAdded != nullptr)
    *pIsAdded = false;

  if (lastGroupId_ == groupId && pLastObj_ != nullptr)
  {
    pRstObj = pLastObj_;
  }
  else
  {
    auto objIter = objMap_.find(groupId);
    if (objIter == objMap_.end())
      return PdbE_OK;

    lastGroupId_ = groupId;
    pLastObj_ = objIter->second;
    pRstObj = objIter->second;
  }

  retVal = this->condiFilter_.RunCondition(pVals, valCnt, resultVal);
  if (retVal != PdbE_OK)
    return retVal;

  if (resultVal)
  {
    retVal = pRstObj->AppendData(pVals, valCnt);
    if (pIsAdded != nullptr)
    {
      *pIsAdded = (retVal == PdbE_OK);
    }
  }

  return PdbE_OK;
}

PdbErr_t ResultFilterGroupDevID::BuildCustomFilter(const QueryParam* pQueryParam,
  const TableInfo* pTabInfo, Arena* pArena)
{
  PdbErr_t retVal = PdbE_OK;

  const ExprList* pColList = pQueryParam->pSelList_;
  const GroupOpt* pGroup = pQueryParam->pGroup_;

  retVal = BuildGroupResultField(pColList->GetExprList(), pTabInfo, pGroup, pArena);
  return retVal;
}

bool ResultFilterGroupDevID::IsQueryLast() const
{
  for (auto fieldIt = fieldVec_.begin(); fieldIt != fieldVec_.end(); fieldIt++)
  {
    if (!((*fieldIt)->IsLastFunc()))
      return false;
  }

  return true;
}

bool ResultFilterGroupDevID::IsQueryFirst() const
{
  for (auto fieldIt = fieldVec_.begin(); fieldIt != fieldVec_.end(); fieldIt++)
  {
    if (!((*fieldIt)->IsFirstFunc()))
      return false;
  }

  return true;
}
