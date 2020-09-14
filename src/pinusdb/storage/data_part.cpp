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

#include "storage/data_part.h"

PdbErr_t DataPart::QueryAsc(const std::list<int64_t>& devIdList,
  const TableInfo* pTabInfo, IQuery* pQuery, uint64_t timeOut)
{
  PdbErr_t retVal = PdbE_OK;
  int64_t bgTs, edTs;
  DataPartQueryParam queryParam;
  pQuery->GetTstampRange(&bgTs, &edTs);

  if (bgTs >= edDayTs_ || edTs < bgDayTs_)
    return PdbE_OK;

  retVal = InitQueryParam(queryParam, pTabInfo, pQuery);
  if (retVal != PdbE_OK)
    return retVal;

  for (auto devIt = devIdList.begin(); devIt != devIdList.end(); devIt++)
  {
    retVal = QueryDevAsc(*devIt, queryParam, pQuery, timeOut, false, nullptr);
    if (retVal != PdbE_OK)
      break;

    if (pQuery->GetIsFullFlag())
      break;
  }

  return retVal;
}

PdbErr_t DataPart::QueryDesc(const std::list<int64_t>& devIdList,
  const TableInfo* pTabInfo, IQuery* pQuery, uint64_t timeOut)
{
  PdbErr_t retVal = PdbE_OK;
  int64_t bgTs, edTs;
  DataPartQueryParam queryParam;
  pQuery->GetTstampRange(&bgTs, &edTs);

  if (bgTs >= edDayTs_ || edTs < bgDayTs_)
    return PdbE_OK;

  retVal = InitQueryParam(queryParam, pTabInfo, pQuery);
  if (retVal != PdbE_OK)
    return retVal;

  for (auto devIt = devIdList.begin(); devIt != devIdList.end(); devIt++)
  {
    retVal = QueryDevDesc(*devIt, queryParam, pQuery, timeOut, false, nullptr);
    if (retVal != PdbE_OK)
      break;

    if (pQuery->GetIsFullFlag())
      break;
  }

  return retVal;
}

PdbErr_t DataPart::QueryFirst(std::list<int64_t>& devIdList,
  const TableInfo* pTabInfo, IQuery* pQuery, uint64_t timeOut)
{
  PdbErr_t retVal = PdbE_OK;
  int64_t bgTs, edTs;
  DataPartQueryParam queryParam;
  pQuery->GetTstampRange(&bgTs, &edTs);

  if (bgTs >= edDayTs_ || edTs < bgDayTs_)
    return PdbE_OK;

  retVal = InitQueryParam(queryParam, pTabInfo, pQuery);
  if (retVal != PdbE_OK)
    return retVal;

  for (auto devIt = devIdList.begin(); devIt != devIdList.end(); )
  {
    bool isAdd = false;
    retVal = QueryDevAsc(*devIt, queryParam, pQuery, timeOut, true, &isAdd);
    if (retVal != PdbE_OK)
      break;

    if (isAdd)
    {
      devIt = devIdList.erase(devIt);
    }
    else
    {
      devIt++;
    }
  }

  return retVal;
}

PdbErr_t DataPart::QueryLast(std::list<int64_t>& devIdList,
  const TableInfo* pTabInfo, IQuery* pQuery, uint64_t timeOut)
{
  PdbErr_t retVal = PdbE_OK;
  int64_t bgTs, edTs;
  DataPartQueryParam queryParam;
  pQuery->GetTstampRange(&bgTs, &edTs);

  if (bgTs >= edDayTs_ || edTs < bgDayTs_)
    return PdbE_OK;

  retVal = InitQueryParam(queryParam, pTabInfo, pQuery);
  if (retVal != PdbE_OK)
    return retVal;

  for (auto devIt = devIdList.begin(); devIt != devIdList.end();)
  {
    bool isAdd = false;
    retVal = QueryDevDesc(*devIt, queryParam, pQuery, timeOut, true, &isAdd);
    if (retVal != PdbE_OK)
      break;

    if (isAdd)
    {
      devIt = devIdList.erase(devIt);
    }
    else
    {
      devIt++;
    }
  }

  return retVal;
}

PdbErr_t DataPart::QuerySnapshot(std::list<int64_t>& devIdList,
  const TableInfo* pTabInfo, IQuery* pQuery, uint64_t timeOut)
{
  PdbErr_t retVal = PdbE_OK;
  DataPartQueryParam queryParam;

  retVal = InitQueryParam(queryParam, pTabInfo, pQuery);
  if (retVal != PdbE_OK)
    return retVal;

  queryParam.InitTstampRange(DateTime::MinMicrosecond, DateTime::MaxMicrosecond);
  for (auto devIt = devIdList.begin(); devIt != devIdList.end();)
  {
    if (*devIt > pQuery->GetMaxDevId())
      break;

    bool isAdd = false;
    retVal = QueryDevSnapshot(*devIt, queryParam, pQuery, timeOut, &isAdd);
    if (retVal != PdbE_OK)
      break;

    if (isAdd)
    {
      devIt = devIdList.erase(devIt);
    }
    else
    {
      devIt++;
    }
  }

  return retVal;
}

PdbErr_t DataPart::InitQueryParam(DataPartQueryParam& queryParam, const TableInfo* pTabInfo, IQuery* pQuery)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType;
  size_t fieldPos;
  int64_t bgTs, edTs;
  if (pTabInfo == nullptr || pQuery == nullptr)
    return PdbE_INVALID_PARAM;

  pQuery->GetTstampRange(&bgTs, &edTs);

  //bool preWhere = SupportPreWhere();
  const std::vector<FieldInfo>& dataFieldInfoVec = GetFieldInfoVec();
  const std::vector<size_t>& dataFieldPosVec = GetFieldPosVec();
  if (dataFieldInfoVec.size() != dataFieldPosVec.size())
    return PdbE_INVALID_PARAM;

  queryParam.InitTstampRange(bgTs, edTs);
  queryParam.InitQueryFieldCnt(pTabInfo->GetFieldCnt());

  std::unordered_set<size_t> fieldSet;
  pQuery->GetUseFields(fieldSet);

  for (size_t idx = 0; idx < dataFieldInfoVec.size(); idx++)
  {
    uint64_t fieldNameCrc = dataFieldInfoVec[idx].GetFieldNameCrc();
    retVal = pTabInfo->GetFieldInfo(fieldNameCrc, &fieldPos, &fieldType);
    if (retVal == PdbE_OK && fieldPos != PDB_DEVID_INDEX)
    {
      int32_t tmpType = dataFieldInfoVec[idx].GetFieldType();
      if (fieldType == tmpType || (PDB_TYPE_IS_REAL(tmpType) && fieldType == PDB_FIELD_TYPE::TYPE_DOUBLE))
      {
        if (fieldSet.find(fieldPos) != fieldSet.end())
        {
          queryParam.AddQueryField(tmpType, fieldPos, dataFieldPosVec[idx]);
        }
      }
    }
  }

  return PdbE_OK;
}