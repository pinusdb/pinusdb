#include "storage/data_part.h"

PdbErr_t DataPart::QueryAsc(const std::list<int64_t>& devIdList, int64_t bgTs, int64_t edTs,
  int* pTypes, size_t fieldCnt, IResultFilter* pResult, uint64_t timeOut)
{
  PdbErr_t retVal = PdbE_OK;

  if (bgTs >= edDayTs_ || edTs < bgDayTs_)
    return PdbE_OK;

  void* pQueryParam = InitQueryParam(pTypes, fieldCnt, bgTs, edTs);
  if (pQueryParam == nullptr)
    return PdbE_NOMEM;

  for (auto devIt = devIdList.begin(); devIt != devIdList.end(); devIt++)
  {
    retVal = QueryDevAsc(*devIt, pQueryParam, pResult, timeOut, false, nullptr);
    if (retVal != PdbE_OK)
      break;

    if (pResult->GetIsFullFlag())
      break;
  }

  ClearQueryParam(pQueryParam);
  return retVal;
}

PdbErr_t DataPart::QueryDesc(const std::list<int64_t>& devIdList, int64_t bgTs, int64_t edTs,
  int* pTypes, size_t fieldCnt, IResultFilter* pResult, uint64_t timeOut)
{
  PdbErr_t retVal = PdbE_OK;

  if (bgTs >= edDayTs_ || edTs < bgDayTs_)
    return PdbE_OK;

  void* pQueryParam = InitQueryParam(pTypes, fieldCnt, bgTs, edTs);
  if (pQueryParam == nullptr)
    return PdbE_NOMEM;

  for (auto devIt = devIdList.begin(); devIt != devIdList.end(); devIt++)
  {
    retVal = QueryDevDesc(*devIt, pQueryParam, pResult, timeOut, false, nullptr);
    if (retVal != PdbE_OK)
      break;

    if (pResult->GetIsFullFlag())
      break;
  }

  ClearQueryParam(pQueryParam);
  return retVal;
}

PdbErr_t DataPart::QueryFirst(std::list<int64_t>& devIdList, int64_t bgTs, int64_t edTs,
  int* pTypes, size_t fieldCnt, IResultFilter* pResult, uint64_t timeOut)
{
  PdbErr_t retVal = PdbE_OK;
  bool isAdd = false;

  if (bgTs >= edDayTs_ || edTs < bgDayTs_)
    return PdbE_OK;

  void* pQueryParam = InitQueryParam(pTypes, fieldCnt, bgTs, edTs);
  if (pQueryParam == nullptr)
    return PdbE_NOMEM;

  for (auto devIt = devIdList.begin(); devIt != devIdList.end(); )
  {
    isAdd = false;
    retVal = QueryDevAsc(*devIt, pQueryParam, pResult, timeOut, true, &isAdd);
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

  ClearQueryParam(pQueryParam);
  return retVal;
}

PdbErr_t DataPart::QueryLast(std::list<int64_t>& devIdList, int64_t bgTs, int64_t edTs,
  int* pTypes, size_t fieldCnt, IResultFilter* pResult, uint64_t timeOut)
{
  PdbErr_t retVal = PdbE_OK;
  bool isAdd = false;
  
  if (bgTs >= edDayTs_ || edTs < bgDayTs_)
    return PdbE_OK;

  void* pQueryParam = InitQueryParam(pTypes, fieldCnt, bgTs, edTs);
  if (pQueryParam == nullptr)
    return PdbE_NOMEM;

  for (auto devIt = devIdList.begin(); devIt != devIdList.end();)
  {
    isAdd = false;
    retVal = QueryDevDesc(*devIt, pQueryParam, pResult, timeOut, true, &isAdd);
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

  ClearQueryParam(pQueryParam);
  return retVal;
}

