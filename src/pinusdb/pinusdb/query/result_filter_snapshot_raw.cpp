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

#include "query/result_filter_snapshot_raw.h"
#include "expr/parse.h"
#include "query/raw_field.h"
#include <algorithm>

bool ResultObjectComp(const ResultObject* pObjA, const ResultObject* pObjB)
{
  return pObjA->GetDevId() < pObjB->GetDevId();
}

ResultFilterSnapshotRaw::ResultFilterSnapshotRaw()
{
  this->isFull_ = false;
  this->maxDevId_ = 0;
}

ResultFilterSnapshotRaw::~ResultFilterSnapshotRaw()
{
}

PdbErr_t ResultFilterSnapshotRaw::AppendData(const DBVal* pVals, size_t valCnt, bool* pIsAdded)
{
  bool resultVal = true;
  PdbErr_t retVal = PdbE_OK;

  retVal = this->condiFilter_.RunCondition(pVals, valCnt, resultVal);
  if (retVal != PdbE_OK)
    return retVal;

  if (pIsAdded != nullptr)
    *pIsAdded = false;

  if (resultVal)
  {
    ResultObject* pRstObj = new ResultObject(fieldVec_, 0, 0);
    if (pRstObj == nullptr)
      return PdbE_NOMEM;

    retVal = pRstObj->AppendData(pVals, valCnt);
    if (retVal != PdbE_OK)
    {
      delete pRstObj;
      return retVal;
    }

    objVec_.push_back(pRstObj);
    if (DBVAL_ELE_GET_INT64(pVals, PDB_DEVID_INDEX) > maxDevId_)
    {
      maxDevId_ = DBVAL_ELE_GET_INT64(pVals, PDB_DEVID_INDEX);
    }

    if (pIsAdded != nullptr)
      *pIsAdded = true;
  }

  return PdbE_OK;
}

PdbErr_t ResultFilterSnapshotRaw::BuildCustomFilter(const QueryParam* pQueryParam,
  const TableInfo* pTabInfo, Arena* pArena)
{
  const ExprList* pColList = pQueryParam->pSelList_;
  if (pQueryParam->pGroup_ != nullptr)
    return PdbE_INVALID_PARAM;

  return BuildResultField(pColList->GetExprList(), pTabInfo, pArena);
}

void ResultFilterSnapshotRaw::CleanUpResult()
{
  std::sort(objVec_.begin(), objVec_.end(), ResultObjectComp);
  ResultObject* pTmpObj = nullptr;
  if (objVec_.size() <= queryOffset_)
  {
    //Empty Result
    queryRecord_ = 0;
  }
  else if (objVec_.size() < (queryOffset_ + queryRecord_))
  {
    queryRecord_ = objVec_.size() - queryOffset_;
  }

  if (queryOffset_ > 0)
  {
    for (size_t idx = 0; idx < queryRecord_; idx++)
    {
      pTmpObj = objVec_[idx];
      objVec_[idx] = objVec_[queryOffset_ + idx];
      objVec_[queryOffset_ + idx] = pTmpObj;
    }
  }

  for (size_t idx = queryRecord_; idx < objVec_.size(); idx++)
  {
    delete objVec_[idx];
  }

  objVec_.resize(queryRecord_);
}

PdbErr_t ResultFilterSnapshotRaw::BuildResultField(const std::vector<ExprItem*>& colItemVec,
  const TableInfo* pTabInfo, Arena* pArena)
{
  PdbErr_t retVal = PdbE_OK;

  int32_t fieldType = 0;
  size_t  fieldPos = 0;
  std::string fieldName;

  for (auto colItem = colItemVec.begin(); colItem != colItemVec.end(); colItem++)
  {
    if ((*colItem)->GetOp() == TK_STAR)
    {
      //将所有列添加到结果集
      size_t tmpFieldCnt = pTabInfo->GetFieldCnt();

      for (size_t i = 0; i < tmpFieldCnt; i++)
      {
        pTabInfo->GetFieldInfo(i, &fieldType);
        fieldName = pTabInfo->GetFieldName(i);


        retVal = AddRawField(i, fieldName, fieldType, pArena);
        if (retVal != PdbE_OK)
          return retVal;
      }
    }
    else if ((*colItem)->GetOp() == TK_ID)
    {
      fieldName = (*colItem)->GetValueStr();
      retVal = pTabInfo->GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
      if (retVal != PdbE_OK)
        return retVal;

      std::string aliasName = (*colItem)->GetAliasName();
      if (aliasName.size() == 0)
        aliasName = fieldName;

      retVal = AddRawField(fieldPos, aliasName, fieldType, pArena);
      if (retVal != PdbE_OK)
        return retVal;
    }
    else
    {
      return PdbE_SQL_RESULT_ERROR;
    }
  }

  return PdbE_OK;
}

