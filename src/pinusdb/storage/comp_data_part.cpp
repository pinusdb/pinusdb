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

#include "storage/comp_data_part.h"
#include "util/string_tool.h"
#include "util/coding.h"
#include "util/log_util.h"
#include "query/block_values.h"
#include "zlib.h"

#define HIS_BLK_LEN  (PDB_KB_BYTES(80))

CompDataPart::CompDataPart()
{
  lastQueryTime_ = 0;
  pData_ = nullptr;
  fieldIdxCnt_ = 0;
}

CompDataPart::~CompDataPart()
{
  Close();
}

PdbErr_t CompDataPart::Open(int32_t partCode, const char* pDataPath)
{
  dataPath_ = pDataPath;

  pData_ = nullptr;
  fieldIdxCnt_ = 0;

  bgDayTs_ = partCode * DateTime::MicrosecondPerDay;
  edDayTs_ = bgDayTs_ + DateTime::MicrosecondPerDay;

  PdbErr_t retVal = InitMemMap();

  return retVal;
}

void CompDataPart::Close()
{
  pData_ = nullptr;
  fieldIdxCnt_ = 0;
  dataMemMap_.Close();
}

PdbErr_t CompDataPart::UnMap()
{
  if (GetRefCnt() == 0 && ((lastQueryTime_ + 600000) < DateTime::NowTickCount()) && pData_ != nullptr)
  {
    dataMemMap_.Close();
    pData_ = nullptr;
    LOG_DEBUG("unmap compress data ({})", dataPath_.c_str());
  }

  return PdbE_OK;
}

PdbErr_t CompDataPart::QueryDevAsc(int64_t devId, const DataPartQueryParam& queryParam,
  IQuery* pQuery, uint64_t timeOut, bool queryFirst, bool* pIsAdd)
{
  return QueryDevData<true, false>(devId, queryParam, pQuery, timeOut, queryFirst, pIsAdd);
}

PdbErr_t CompDataPart::QueryDevDesc(int64_t devId, const DataPartQueryParam& queryParam,
  IQuery* pQuery, uint64_t timeOut, bool queryLast, bool* pIsAdd)
{
  return QueryDevData<false, false>(devId, queryParam, pQuery, timeOut, queryLast, pIsAdd);
}

PdbErr_t CompDataPart::QueryDevSnapshot(int64_t devId, const DataPartQueryParam& queryParam,
  IQuery* pQuery, uint64_t timeOut, bool* pIsAdd)
{
  return QueryDevData<false, true>(devId, queryParam, pQuery, timeOut, true, pIsAdd);
}

template<bool IsAsc, bool IsSnapshot>
PdbErr_t CompDataPart::QueryDevData(int64_t devId, const DataPartQueryParam& queryParam,
  IQuery* pQuery, uint64_t timeOut, bool querySingle, bool* pIsAdd)
{
  PdbErr_t retVal = PdbE_OK;
  std::vector<size_t> idxVec;
  int64_t bgTs = queryParam.GetBgTs();
  int64_t edTs = queryParam.GetEdTs();
  bool isAdd = false;

  if PDB_CONSTEXPR(IsSnapshot)
  {
    bgTs = DateTime::MinMicrosecond;
    edTs = DateTime::MaxMicrosecond;
  }

  retVal = InitMemMap();
  if (retVal != PdbE_OK)
    return retVal;

  retVal = GetDevIdxs(devId, bgTs, edTs, idxVec);
  if (retVal == PdbE_DEV_NOT_FOUND || retVal == PdbE_IDX_NOT_FOUND)
  {
    if (pIsAdd != nullptr)
      *pIsAdd = false;

    return PdbE_OK;
  }

  if (retVal != PdbE_OK)
    return retVal;

  if PDB_CONSTEXPR(!IsAsc)
  {
    std::reverse(idxVec.begin(), idxVec.end());
  }

  for (auto idxIt = idxVec.begin(); idxIt != idxVec.end(); idxIt++)
  {
    retVal = TraversalDataPage<IsAsc>(*idxIt, queryParam, pQuery, &isAdd);
    if (querySingle)
    {
      if (retVal == PdbE_OK && isAdd)
      {
        if (pIsAdd != nullptr)
          *pIsAdd = true;

        return PdbE_OK;
      }
    }

    if (retVal == PdbE_RESULT_FULL)
      return PdbE_OK;

    if (retVal != PdbE_OK)
      return retVal;

    if (pQuery->GetIsFullFlag())
      return PdbE_OK;

    if PDB_CONSTEXPR(IsSnapshot)
    {
      if (pIsAdd != nullptr)
        *pIsAdd = true;

      return PdbE_OK;
    }

    if (DateTime::NowTickCount() > timeOut)
      return PdbE_QUERY_TIME_OUT;
  }

  return PdbE_OK;
}


PdbErr_t CompDataPart::InitMemMap()
{
  PdbErr_t retVal = PdbE_OK;
  std::unique_lock<std::mutex> initLock(initMutex_);
  if (pData_ == nullptr)
  {
    retVal = dataMemMap_.Open(dataPath_.c_str(), true);
    if (retVal != PdbE_OK)
      return retVal;

    pData_ = (char*)dataMemMap_.GetBaseAddr();
    size_t dataSize = dataMemMap_.MemMapSize();

    if (fieldInfoVec_.empty())
    {
      FieldInfo finfo;
      const DataFileMeta* pDataMeta = (const DataFileMeta*)pData_;
      size_t fieldCnt = Coding::FixedDecode32(pDataMeta->fieldCnt_);
      for (size_t i = 0; i < fieldCnt; i++)
      {
        int fieldType = Coding::FixedDecode32(pDataMeta->fieldRec_[i].fieldType_);
        retVal = finfo.SetFieldInfo(pDataMeta->fieldRec_[i].fieldName_, fieldType, false);
        if (retVal != PdbE_OK)
        {
          LOG_ERROR("failed to init data file ({}), invalid field name", dataPath_.c_str());
          return retVal;
        }
        fieldInfoVec_.push_back(finfo);
        fieldPosVec_.push_back(i);
      }

      const CmpFooter* pFooter = (const CmpFooter*)(pData_ + dataSize - sizeof(CmpFooter));
      fieldIdxCnt_ = Coding::FixedDecode32(pFooter->tsIdxCnt_);
      size_t tsIdxPos = Coding::FixedDecode64(pFooter->tsIdxPos_);
      idxPosVec_.reserve(fieldInfoVec_.size());
      idxPosVec_.push_back(0); //devid
      idxPosVec_.push_back(tsIdxPos); // tstamp
      size_t tmpPos = tsIdxPos + sizeof(TsBlkIdx) * fieldIdxCnt_;
      for (size_t idx = (PDB_TSTAMP_INDEX + 1); idx < fieldInfoVec_.size(); idx++)
      {
        idxPosVec_.push_back(tmpPos);
        tmpPos += sizeof(CmpBlkIdx) * fieldIdxCnt_;
      }

      const TsBlkIdx* pTsIdx = (const TsBlkIdx*)(pData_ + idxPosVec_[PDB_TSTAMP_INDEX]);
      TsIdxItem idxItem;
      idxItem.bgPos_ = 0;
      idxItem.idxCnt_ = 0;
      int64_t devId = 0;
      for (size_t idx = 0; idx < fieldIdxCnt_; idx++)
      {
        int64_t tmpId = Coding::FixedDecode64(pTsIdx->devId_);
        if (devId != tmpId)
        {
          if (devId > 0)
          {
            tsIdxMap_.insert(std::pair<int64_t, TsIdxItem>(devId, idxItem));
          }

          devId = tmpId;
          idxItem.bgPos_ = static_cast<uint32_t>(idx);
          idxItem.idxCnt_ = 1;
        }
        else
        {
          idxItem.idxCnt_++;
        }

        pTsIdx++;
      }
      if (devId > 0)
      {
        tsIdxMap_.insert(std::pair<int64_t, TsIdxItem>(devId, idxItem));
      }
    }
  }

  LOG_DEBUG("init mmap compress data ({})", dataPath_.c_str());
  return PdbE_OK;
}

PdbErr_t CompDataPart::GetDevIdxs(int64_t devId, int64_t bgts, int64_t edTs, std::vector<size_t>& idxVec)
{
  auto tsIdxIter = tsIdxMap_.find(devId);
  if (tsIdxIter == tsIdxMap_.end())
    return PdbE_DEV_NOT_FOUND;

  const TsBlkIdx* pTsIdxs = (const TsBlkIdx*)(pData_ + idxPosVec_[PDB_TSTAMP_INDEX])
    + tsIdxIter->second.bgPos_;

  int32_t idxCnt = tsIdxIter->second.idxCnt_;
  int32_t lwr = 0;
  int32_t upr = idxCnt - 1;
  int32_t idx = 0;

  while (lwr <= upr)
  {
    idx = (lwr + upr) / 2;
    if (Coding::FixedDecode64(pTsIdxs[idx].bgTs_) > bgts)
    {
      upr = idx - 1;
    }
    else
    {
      if ((idx + 1) == idxCnt)
        break;

      if (Coding::FixedDecode64(pTsIdxs[idx + 1].bgTs_) > bgts)
        break;

      lwr = idx + 1;
    }
  }

  while (idx < idxCnt)
  {
    if (Coding::FixedDecode64(pTsIdxs[idx].bgTs_) > edTs)
      break;

    idxVec.push_back(((size_t)tsIdxIter->second.bgPos_ + idx));
    idx++;
  }

  return PdbE_OK;
}

template<bool IsAsc>
PdbErr_t CompDataPart::TraversalDataPage(size_t idxPos,
  const DataPartQueryParam& queryParam, IQuery* pQuery, bool* pIsAdd)
{
  PdbErr_t retVal = PdbE_OK;
  Arena arena;
  if (pQuery == nullptr)
    return PdbE_INVALID_PARAM;

  lastQueryTime_ = DateTime::NowTickCount();
  const TsBlkIdx* pTsIdx = (const TsBlkIdx*)(pData_ + idxPosVec_[PDB_TSTAMP_INDEX])
    + idxPos;
  size_t recCnt = Coding::FixedDecode32(pTsIdx->recCnt_);
  int64_t devId = Coding::FixedDecode64(pTsIdx->devId_);

  size_t fieldCnt = queryParam.GetQueryFieldCnt();
  BlockValues blockValue(fieldCnt);
  blockValue.SetRecordSize(recCnt);

  DBVal* pDevVals = (DBVal*)arena.AllocateAligned(sizeof(DBVal) * recCnt);
  for (size_t idx = 0; idx < recCnt; idx++)
  {
    DBVAL_ELE_SET_INT64(pDevVals, idx, devId);
  }
  blockValue.SetColumnValues(PDB_DEVID_INDEX, pDevVals);

  std::string rawBuf;
  rawBuf.reserve(PDB_KB_BYTES(512));
  const std::vector<FieldQueryMapping>& fieldMapVec = queryParam.GetQueryFieldVec();
  for (size_t idx = 0; idx < fieldMapVec.size(); idx++)
  {
    size_t storeFieldPos = fieldMapVec[idx].storeFieldPos_;
    DBVal* pVals = (DBVal*)arena.AllocateAligned(sizeof(DBVal) * recCnt);

    const CmpBlockHead* pBlkHead = nullptr;
    if (storeFieldPos == PDB_TSTAMP_INDEX)
    {
      pBlkHead = (const CmpBlockHead*)(pData_ + Coding::FixedDecode64(pTsIdx->blkPos_));
    }
    else
    {
      const CmpBlkIdx* pBlkIdx = (const CmpBlkIdx*)(pData_ + idxPosVec_[storeFieldPos]) + idxPos;
      pBlkHead = (const CmpBlockHead*)(pData_ + Coding::FixedDecode64(pBlkIdx->blkPos_));
    }

    if (Coding::FixedDecode32(pBlkHead->recCnt_) != recCnt)
      return PdbE_RECORD_FAIL;

    size_t dataLen = Coding::FixedDecode32(pBlkHead->dataLen_);

    char* pRawBuf = &rawBuf[0];
    uLongf destLen = rawBuf.capacity();
    if (uncompress((Bytef*)pRawBuf, &destLen, (uint8_t*)(pBlkHead + 1), dataLen) != Z_OK)
    {
      return PdbE_PAGE_ERROR;
    }

    switch (fieldMapVec[idx].fieldType_)
    {
    case PDB_FIELD_TYPE::TYPE_BOOL:
      retVal = DecodeBoolVals<IsAsc>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    case PDB_FIELD_TYPE::TYPE_INT8:
      retVal = DecodeIntVals<IsAsc, PDB_FIELD_TYPE::TYPE_INT8>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    case PDB_FIELD_TYPE::TYPE_INT16:
      retVal = DecodeIntVals<IsAsc, PDB_FIELD_TYPE::TYPE_INT16>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    case PDB_FIELD_TYPE::TYPE_INT32:
      retVal = DecodeIntVals<IsAsc, PDB_FIELD_TYPE::TYPE_INT32>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    case PDB_FIELD_TYPE::TYPE_INT64:
      retVal = DecodeIntVals<IsAsc, PDB_FIELD_TYPE::TYPE_INT64>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    case PDB_FIELD_TYPE::TYPE_DATETIME:
      retVal = DecodeIntVals<IsAsc, PDB_FIELD_TYPE::TYPE_DATETIME>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    case PDB_FIELD_TYPE::TYPE_FLOAT:
      retVal = DecodeFloatVals<IsAsc>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    case PDB_FIELD_TYPE::TYPE_DOUBLE:
      retVal = DecodeDoubleVals<IsAsc>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    case PDB_FIELD_TYPE::TYPE_STRING:
      retVal = DecodeBlockVals<IsAsc, PDB_FIELD_TYPE::TYPE_STRING>(pVals, recCnt, pRawBuf, (pRawBuf + destLen), arena);
      break;
    case PDB_FIELD_TYPE::TYPE_BLOB:
      retVal = DecodeBlockVals<IsAsc, PDB_FIELD_TYPE::TYPE_BLOB>(pVals, recCnt, pRawBuf, (pRawBuf + destLen), arena);
      break;
    case PDB_FIELD_TYPE::TYPE_REAL2:
      retVal = DecodeIntVals<IsAsc, PDB_FIELD_TYPE::TYPE_REAL2>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    case PDB_FIELD_TYPE::TYPE_REAL3:
      retVal = DecodeIntVals<IsAsc, PDB_FIELD_TYPE::TYPE_REAL3>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    case PDB_FIELD_TYPE::TYPE_REAL4:
      retVal = DecodeIntVals<IsAsc, PDB_FIELD_TYPE::TYPE_REAL4>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    case PDB_FIELD_TYPE::TYPE_REAL6:
      retVal = DecodeIntVals<IsAsc, PDB_FIELD_TYPE::TYPE_REAL6>(pVals, recCnt, pRawBuf, (pRawBuf + destLen));
      break;
    }

    if (retVal != PdbE_OK)
      return retVal;

    blockValue.SetColumnValues(fieldMapVec[idx].queryFieldPos_, pVals);
  }

  return pQuery->AppendArray(blockValue, pIsAdd);
}


template<bool IsAsc>
PdbErr_t CompDataPart::DecodeBoolVals(DBVal* pValsBg, size_t valCnt, const char* pBuf, const char* pLimit)
{
  if (pBuf + ((valCnt + 7) / 8) > pLimit)
    return PdbE_INVALID_PARAM;

  DBVal* pVal = IsAsc ? pValsBg : (pValsBg + valCnt - 1);
  for (size_t idx = 0; idx < valCnt; idx++)
  {
    DBVAL_SET_BOOL(pVal, (BIT_MAP_IS_SET(pBuf, idx) != 0));

    if PDB_CONSTEXPR(IsAsc)
      pVal++;
    else
      pVal--;
  }

  return PdbE_OK;
}

template<bool IsAsc, int FieldType>
PdbErr_t CompDataPart::DecodeIntVals(DBVal* pValsBg, size_t valCnt, const char* pBuf, const char* pLimit)
{
  DBVal* pVal = IsAsc ? pValsBg : (pValsBg + valCnt - 1);
  int64_t preVal = 0;
  int64_t preDelta = 0;
  int64_t delta = 0;
  int64_t curVal = 0;
  uint64_t tmpVal = 0;

#define SetInnerIntValue do { \
  if PDB_CONSTEXPR (FieldType == PDB_FIELD_TYPE::TYPE_INT8) \
    DBVAL_SET_INT8(pVal, static_cast<int8_t>(curVal)); \
  else if PDB_CONSTEXPR (FieldType == PDB_FIELD_TYPE::TYPE_INT16) \
    DBVAL_SET_INT16(pVal, static_cast<int16_t>(curVal)); \
  else if PDB_CONSTEXPR (FieldType == PDB_FIELD_TYPE::TYPE_INT32) \
    DBVAL_SET_INT32(pVal, static_cast<int32_t>(curVal)); \
  else if PDB_CONSTEXPR (FieldType == PDB_FIELD_TYPE::TYPE_INT64) \
    DBVAL_SET_INT64(pVal, curVal); \
  else if PDB_CONSTEXPR (FieldType == PDB_FIELD_TYPE::TYPE_DATETIME) \
    DBVAL_SET_DATETIME(pVal, curVal); \
  else if PDB_CONSTEXPR (FieldType == PDB_FIELD_TYPE::TYPE_REAL2) \
    DBVAL_SET_DOUBLE(pVal, (curVal / 100.0)); \
  else if PDB_CONSTEXPR (FieldType == PDB_FIELD_TYPE::TYPE_REAL3) \
    DBVAL_SET_DOUBLE(pVal, (curVal / 1000.0)); \
  else if PDB_CONSTEXPR (FieldType == PDB_FIELD_TYPE::TYPE_REAL4) \
    DBVAL_SET_DOUBLE(pVal, (curVal / 10000.0)); \
  else if PDB_CONSTEXPR (FieldType == PDB_FIELD_TYPE::TYPE_REAL6) \
    DBVAL_SET_DOUBLE(pVal, (curVal / 1000000.0)); \
} while(false)

  pBuf = Coding::VarintDecode64(pBuf, pLimit, &tmpVal);
  curVal = Coding::ZigzagDecode64(tmpVal);
  SetInnerIntValue;

  if PDB_CONSTEXPR(IsAsc)
    pVal++;
  else
    pVal--;

  if (pBuf == nullptr)
    return PdbE_RECORD_FAIL;

  if (valCnt == 1)
    return PdbE_OK;

  preVal = curVal;
  pBuf = Coding::VarintDecode64(pBuf, pLimit, &tmpVal);
  delta = Coding::ZigzagDecode64(tmpVal);
  curVal = preVal + delta;
  SetInnerIntValue;

  if PDB_CONSTEXPR(IsAsc)
    pVal++;
  else
    pVal--;

  if (pBuf == nullptr)
    return PdbE_RECORD_FAIL;
  
  preDelta = curVal - preVal;
  preVal = curVal;
  for (size_t idx = 2; idx < valCnt; idx++)
  {
    pBuf = Coding::VarintDecode64(pBuf, pLimit, &tmpVal);
    delta = Coding::ZigzagDecode64(tmpVal);
    curVal = preVal + preDelta + delta;

    SetInnerIntValue;

    if PDB_CONSTEXPR(IsAsc)
      pVal++;
    else
      pVal--;

    if (pBuf == nullptr)
      return PdbE_RECORD_FAIL;

    preDelta = curVal - preVal;
    preVal = curVal;
  }

#undef SetInnerIntValue

  return PdbE_OK;
}

template<bool IsAsc>
PdbErr_t CompDataPart::DecodeFloatVals(DBVal* pValsBg, size_t valCnt, const char* pBuf, const char* pLimit)
{
  uint32_t u32Val = 0;
  uint32_t u32Tmp = 0;
  uint8_t u8 = 0;

  DBVal* pVal = IsAsc ? pValsBg : (pValsBg + valCnt - 1);

  for (size_t idx = 0; idx < valCnt; idx++)
  {
    u32Tmp = 0;
    u8 = *pBuf++;

    if ((u8 & 0x8) != 0)
    {
      pBuf = Coding::VarintDecode32(pBuf, pLimit, &u32Tmp);
      if (pBuf == nullptr)
        return PdbE_RECORD_FAIL;
    }

    u32Tmp = ((u32Tmp) << 3) | (u8 & 0x7);
    u32Tmp <<= (((u8 & 0xF0) >> 4) * 2);

    u32Val ^= u32Tmp;
    DBVAL_SET_FLOAT(pVal, Coding::DecodeFloat(u32Val));

    if PDB_CONSTEXPR(IsAsc)
      pVal++;
    else
      pVal--;
  }

  return PdbE_OK;
}

template<bool IsAsc>
PdbErr_t CompDataPart::DecodeDoubleVals(DBVal* pValsBg, size_t valCnt, const char* pBuf, const char* pLimit)
{
  uint64_t u64Val = 0;
  uint64_t u64Tmp = 0;
  uint8_t u8 = 0;

  DBVal* pVal = IsAsc ? pValsBg : (pValsBg + valCnt - 1);

  for (size_t idx = 0; idx < valCnt; idx++)
  {
    u64Tmp = 0;
    u8 = *pBuf++;

    if ((u8 & 0x4) != 0)
    {
      pBuf = Coding::VarintDecode64(pBuf, pLimit, &u64Tmp);
      if (pBuf == nullptr)
        return PdbE_RECORD_FAIL;
    }

    u64Tmp = (((u64Tmp) << 2) | (u8 & 0x3));
    u64Tmp <<= (((u8 & 0xF8) >> 3) * 2);

    u64Val ^= u64Tmp;
    DBVAL_SET_DOUBLE(pVal, Coding::DecodeDouble(u64Val));

    if PDB_CONSTEXPR(IsAsc)
      pVal++;
    else
      pVal--;
  }

  return PdbE_OK;
}

template<bool IsAsc, int FieldType>
PdbErr_t CompDataPart::DecodeBlockVals(DBVal* pValsBg, size_t valCnt, const char* pBuf, const char* pLimit, Arena& arena)
{
  uint32_t u32 = 0;
  char* pData = nullptr;

  DBVal* pVal = IsAsc ? pValsBg : (pValsBg + valCnt - 1);
  for (size_t idx = 0; idx < valCnt; idx++)
  {
    pBuf = Coding::VarintDecode32(pBuf, pLimit, &u32);
    if (u32 > 0)
    {
      pData = arena.Allocate(u32);
      memcpy(pData, pBuf, u32);

      if (FieldType == PDB_FIELD_TYPE::TYPE_STRING)
        DBVAL_SET_STRING(pVal, pData, u32);
      else
        DBVAL_SET_BLOB(pVal, pData, u32);
    }
    else
    {
      if (FieldType == PDB_FIELD_TYPE::TYPE_STRING)
        DBVAL_SET_STRING(pVal, nullptr, 0);
      else
        DBVAL_SET_BLOB(pVal, nullptr, 0);
    }

    pBuf += u32;

    if PDB_CONSTEXPR(IsAsc)
      pVal++;
    else
      pVal--;
  }

  return PdbE_OK;
}


const std::vector<FieldInfo>& CompDataPart::GetFieldInfoVec() const
{
  return fieldInfoVec_;
}

const std::vector<size_t>& CompDataPart::GetFieldPosVec() const
{
  return fieldPosVec_;
}
