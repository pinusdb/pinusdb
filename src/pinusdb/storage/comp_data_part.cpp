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
#include "zlib.h"

#define HIS_BLK_LEN  (PDB_KB_BYTES(80))

CompDataPart::CompDataPart()
{
  lastQueryTime_ = 0;
  pData_ = nullptr;
  pDevId_ = nullptr;
  devCnt_ = 0;
}

CompDataPart::~CompDataPart()
{
  Close();
}

PdbErr_t CompDataPart::Open(int32_t partCode, const char* pDataPath)
{
  dataPath_ = pDataPath;

  pData_ = nullptr;
  pDevId_ = nullptr;

  bgDayTs_ = partCode * MillisPerDay;
  edDayTs_ = bgDayTs_ + MillisPerDay;
  return PdbE_OK;
}

void CompDataPart::Close()
{
  pData_ = nullptr;
  pDevId_ = nullptr;
  devCnt_ = 0;
  dataMemMap_.Close();
}

PdbErr_t CompDataPart::RecoverDW(const char* pPageBuf)
{
  return PdbE_FILE_READONLY;
}

PdbErr_t CompDataPart::InsertRec(uint32_t metaCode, int64_t devId, int64_t tstamp,
  bool replace, const char* pRec, size_t recLen)
{
  return PdbE_FILE_READONLY;
}

PdbErr_t CompDataPart::UnMap()
{
  if (GetRefCnt() == 0 && ((lastQueryTime_ + 600000) < DateTime::NowTickCount()) && pData_ != nullptr)
  {
    dataMemMap_.Close();
    pData_ = nullptr;
    pDevId_ = nullptr;
    devCnt_ = 0;
    LOG_DEBUG("unmap compress data ({})", dataPath_.c_str());
  }

  return PdbE_OK;
}

PdbErr_t CompDataPart::QueryDevAsc(int64_t devId, void* pQueryParam,
  IQuery* pQuery, uint64_t timeOut, bool queryFirst, bool* pIsAdd)
{
  PdbErr_t retVal = PdbE_OK;
  CompDevId compDevId;
  CompDataIter* pHisIter = (CompDataIter*)pQueryParam;
  int64_t bgTs = pHisIter->GetBgTs();
  int64_t edTs = pHisIter->GetEdTs();
  size_t fieldCnt = pHisIter->GetFieldCnt();
  int64_t curTs = 0;
  bool firstPage = true;
  bool isAdd = false;
  int32_t curIdxPos = 0;

  if (pData_ == nullptr)
  {
    retVal = InitMemMap();
    if (retVal != PdbE_OK)
      return retVal;
  }

  lastQueryTime_ = DateTime::NowTickCount();

  retVal = GetIdx(devId, bgTs, &compDevId, &curIdxPos);
  if (retVal == PdbE_DEV_NOT_FOUND)
    return PdbE_OK;
  if (retVal != PdbE_OK)
    return retVal;

  const CompBlkIdx* pBlkIdx = (const CompBlkIdx*)(pData_ + compDevId.bgPos_);
  for (; curIdxPos < compDevId.blkIdxCnt_; curIdxPos++)
  {
    retVal = pHisIter->Load((pData_ + pBlkIdx[curIdxPos].blkPos_), pBlkIdx[curIdxPos].blkLen_);
    if (retVal != PdbE_OK)
      return retVal;

    if (firstPage)
    {
      retVal = pHisIter->SeekTo(bgTs);
      firstPage = false;
    }
    else
    {
      retVal = pHisIter->SeekToFirst();
    }
    if (retVal != PdbE_OK)
      return retVal;

    while (pHisIter->Valid())
    {
      DBVal* pVals = pHisIter->GetRecord();
      retVal = pQuery->AppendData(pVals, fieldCnt, &isAdd);
      if (retVal != PdbE_OK)
        return retVal;

      if (queryFirst && isAdd)
      {
        if (pIsAdd != nullptr)
          *pIsAdd = true;

        return PdbE_OK;
      }

      if (pQuery->GetIsFullFlag())
        return PdbE_OK;

      curTs = DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX);
      if (curTs >= edTs)
        return PdbE_OK;

      pHisIter->Next();
    }

    if (DateTime::NowTickCount() > timeOut)
      return PdbE_QUERY_TIME_OUT;
  }

  return PdbE_OK;
}

PdbErr_t CompDataPart::QueryDevDesc(int64_t devId, void* pQueryParam,
  IQuery* pQuery, uint64_t timeOut, bool queryLast, bool* pIsAdd)
{
  PdbErr_t retVal = PdbE_OK;
  CompDevId compDevId;
  CompDataIter* pHisIter = (CompDataIter*)pQueryParam;
  int64_t bgTs = pHisIter->GetBgTs();
  int64_t edTs = pHisIter->GetEdTs();
  size_t fieldCnt = pHisIter->GetFieldCnt();
  int64_t curTs = 0;
  bool firstPage = true;
  bool isAdd = false;
  int32_t curIdxPos = 0;

  if (pData_ == nullptr)
  {
    retVal = InitMemMap();
    if (retVal != PdbE_OK)
      return retVal;
  }

  lastQueryTime_ = DateTime::NowTickCount();

  retVal = GetIdx(devId, edTs, &compDevId, &curIdxPos);
  if (retVal == PdbE_DEV_NOT_FOUND)
    return PdbE_OK;
  if (retVal != PdbE_OK)
    return retVal;

  const CompBlkIdx* pBlkIdx = (const CompBlkIdx*)(pData_ + compDevId.bgPos_);
  for (; curIdxPos >= 0; curIdxPos--)
  {
    retVal = pHisIter->Load((pData_ + pBlkIdx[curIdxPos].blkPos_), pBlkIdx[curIdxPos].blkLen_);
    if (retVal != PdbE_OK)
      return retVal;

    if (firstPage)
    {
      retVal = pHisIter->SeekTo(edTs);
      firstPage = false;
    }
    else
    {
      retVal = pHisIter->SeekToLast();
    }
    if (retVal != PdbE_OK)
      return retVal;

    while (pHisIter->Valid())
    {
      DBVal* pVals = pHisIter->GetRecord();
      retVal = pQuery->AppendData(pVals, fieldCnt, &isAdd);
      if (retVal != PdbE_OK)
        return retVal;

      if (queryLast && isAdd)
      {
        if (pIsAdd != nullptr)
          *pIsAdd = true;

        return PdbE_OK;
      }

      if (pQuery->GetIsFullFlag())
        return PdbE_OK;

      curTs = DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX);
      if (curTs <= bgTs)
        return PdbE_OK;

      pHisIter->Prev();
    }

    if (DateTime::NowTickCount() > timeOut)
      return PdbE_QUERY_TIME_OUT;
  }

  return PdbE_OK;
}

PdbErr_t CompDataPart::QueryDevSnapshot(int64_t devId, void* pQueryParam,
  IQuery* pQuery, uint64_t timeOut, bool* pIsAdd)
{
  PdbErr_t retVal = PdbE_OK;
  CompDevId compDevId;
  CompDataIter* pHisIter = (CompDataIter*)pQueryParam;
  size_t fieldCnt = pHisIter->GetFieldCnt();

  if (pData_ == nullptr)
  {
    retVal = InitMemMap();
    if (retVal != PdbE_OK)
      return retVal;
  }

  lastQueryTime_ = DateTime::NowTickCount();

  retVal = GetIdx(devId, MaxMillis, &compDevId, nullptr);
  if (retVal == PdbE_DEV_NOT_FOUND)
  {
    if (pIsAdd != nullptr)
      *pIsAdd = false;

    return PdbE_OK;
  }

  if (retVal != PdbE_OK)
    return retVal;

  const CompBlkIdx* pBlkIdx = (const CompBlkIdx*)(pData_ + compDevId.bgPos_);
  int32_t blkPos = compDevId.blkIdxCnt_ - 1;
  retVal = pHisIter->Load((pData_ + pBlkIdx[blkPos].blkPos_), pBlkIdx[blkPos].blkLen_);
  if (retVal != PdbE_OK)
    return retVal;

  retVal = pHisIter->SeekToLast();
  if (retVal != PdbE_OK)
    return retVal;

  if (pHisIter->Valid())
  {
    DBVal* pVals = pHisIter->GetRecord();
    retVal = pQuery->AppendData(pVals, fieldCnt, nullptr);
    if (retVal != PdbE_OK)
      return retVal;
  }

  if (pIsAdd != nullptr)
    *pIsAdd = true;

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

    const char* pTmpBase = (char*)dataMemMap_.GetBaseAddr();
    size_t dataSize = dataMemMap_.MemMapSize();

    const CompDataFooter* pFooter = (const CompDataFooter*)(pTmpBase + dataSize - sizeof(CompDataFooter));
    pDevId_ = (const CompDevId*)(pTmpBase + pFooter->devIdsPos_);
    devCnt_ = static_cast<int32_t>(pFooter->devCnt_);
    pData_ = pTmpBase;
    lastQueryTime_ = DateTime::NowTickCount();

    if (fieldVec_.empty())
    {
      FieldInfo finfo;
      const DataFileMeta* pDataMeta = (const DataFileMeta*)pTmpBase;
      for (size_t i = 0; i < pDataMeta->fieldCnt_; i++)
      {
        retVal = finfo.SetFieldInfo(pDataMeta->fieldRec_[i].fieldName_,
          pDataMeta->fieldRec_[i].fieldType_, false);
        if (retVal != PdbE_OK)
        {
          LOG_ERROR("failed to init data file ({}), invalid field name", dataPath_.c_str());
          return retVal;
        }

        fieldVec_.push_back(finfo);
      }
    }
  }

  LOG_DEBUG("init mmap compress data ({})", dataPath_.c_str());
  return PdbE_OK;
}

PdbErr_t CompDataPart::GetIdx(int64_t devId, int64_t ts, CompDevId* pCompDevId, int* pCurIdx)
{
  int32_t lwr = 0;
  int32_t upr = devCnt_ - 1;
  int32_t idx = 0;
  int32_t tmpTs = 0;
  if (ts < bgDayTs_)
    tmpTs = 0;
  else if (ts >= edDayTs_)
    tmpTs = static_cast<int32_t>(MillisPerDay - 1);
  else
    tmpTs = static_cast<int32_t>(ts - bgDayTs_);

  while (lwr <= upr)
  {
    idx = (lwr + upr) / 2;
    if (pDevId_[idx].devId_ == devId)
      break;
    else if (pDevId_[idx].devId_ < devId)
      lwr = idx + 1;
    else
      upr = idx - 1;
  }

  if (pDevId_[idx].devId_ != devId)
    return PdbE_DEV_NOT_FOUND;

  if (pCompDevId != nullptr)
    *pCompDevId = pDevId_[idx];

  int32_t blkCnt = pDevId_[idx].blkIdxCnt_;
  const CompBlkIdx* pBlkIdx = (const CompBlkIdx*)(pData_ + pDevId_[idx].bgPos_);
  lwr = 0;
  upr = blkCnt - 1;
  idx = 0;

  while (lwr <= upr)
  {
    idx = (lwr + upr) / 2;
    if (pBlkIdx[idx].bgTsForDay_ > tmpTs)
    {
      upr = idx - 1;
    }
    else
    {
      if ((idx + 1) == blkCnt)
        break;

      if (pBlkIdx[(idx + 1)].bgTsForDay_ > tmpTs)
        break;

      lwr = idx + 1;
    }
  }

  if (pCurIdx != nullptr)
    *pCurIdx = idx;

  return PdbE_OK;
}

void* CompDataPart::InitQueryParam(const TableInfo* pQueryInfo, int64_t bgTs, int64_t edTs)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t queryType = 0;
  size_t queryPos = 0;
  std::vector<int> fieldPosVec;

  if (pData_ == nullptr)
  {
    retVal = InitMemMap();
    if (retVal != PdbE_OK)
      return nullptr;
  }

  fieldPosVec.resize(fieldVec_.size());
  CompDataIter* pCompIter = new (std::nothrow)CompDataIter();
  if (pCompIter == nullptr)
    return nullptr;

  for (size_t idx = 0; idx < fieldPosVec.size(); idx++)
  {
    fieldPosVec[idx] = -1;
  }

  for (size_t idx = 0; idx < fieldVec_.size(); idx++)
  {
    retVal = pQueryInfo->GetFieldInfo(fieldVec_[idx].GetFieldNameCrc(), &queryPos, &queryType);
    if (retVal == PdbE_OK)
    {
      int32_t tmpType = fieldVec_[idx].GetFieldType();
      if (PDB_TYPE_IS_REAL(tmpType))
        tmpType = PDB_FIELD_TYPE::TYPE_DOUBLE;

      if (tmpType == queryType)
        fieldPosVec[idx] = static_cast<int>(queryPos);
    }
  }

  retVal = pCompIter->Init(fieldVec_, fieldPosVec.data(), pQueryInfo->GetFieldCnt(), bgTs, edTs);
  if (retVal != PdbE_OK)
  {
    delete pCompIter;
    return nullptr;
  }

  return pCompIter;
}

void CompDataPart::ClearQueryParam(void* pQueryParam)
{
  CompDataIter* pHisIter = (CompDataIter*)pQueryParam;
  delete pHisIter;
}

CompDataPart::CompDataIter::CompDataIter()
{
  fieldCnt_ = 0;
  pTypes_ = nullptr;
  pFieldPos_ = nullptr;
  pQueryVals_ = nullptr;
  pRawBuf_ = nullptr;
  bgTs_ = 0;
  edTs_ = 0;

  recCnt_ = 0;
  curIdx_ = 0;
  totalValCnt_ = 0;
  pAllVals_ = nullptr;
}

CompDataPart::CompDataIter::~CompDataIter()
{
  if (pAllVals_ != nullptr)
    delete[] pAllVals_;
}

PdbErr_t CompDataPart::CompDataIter::Init(const std::vector<FieldInfo>& fieldVec,
  int* pFieldPos, size_t queryFieldCnt, int64_t bgTs, int64_t edTs)
{
  mateCnt_ = 0;
  fieldCnt_ = fieldVec.size();
  pTypes_ = (int*)arena_.AllocateAligned((sizeof(int) * fieldCnt_));
  pFieldPos_ = (int*)arena_.AllocateAligned((sizeof(int) * fieldCnt_));
  pQueryVals_ = (DBVal*)arena_.AllocateAligned((sizeof(DBVal) * fieldCnt_));
  pRawBuf_ = arena_.AllocateAligned(HIS_BLK_LEN);

  if (pTypes_ == nullptr || pFieldPos_ == nullptr || pQueryVals_ == nullptr || pRawBuf_ == nullptr)
    return PdbE_NOMEM;

  for (size_t idx = 0; idx < queryFieldCnt; idx++)
  {
    DBVAL_ELE_SET_NULL(pQueryVals_, idx);
  }

  for (size_t idx = 0; idx < fieldCnt_; idx++)
  {
    pTypes_[idx] = fieldVec[idx].GetFieldType();
    pFieldPos_[idx] = pFieldPos[idx];

    if (pFieldPos_[idx] > 0) // ÅÅ³ýdevId
      mateCnt_++;
  }

  bgTs_ = bgTs;
  edTs_ = edTs;
  
  recCnt_ = 0;
  curIdx_ = 0;
  totalValCnt_ = 0;

  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::Load(const char* pBuf, size_t bufLen)
{
  PdbErr_t retVal = PdbE_OK;
  uint32_t u32 = 0;
  const CompBlockHead* pCompBlkHead = (const CompBlockHead*)pBuf;
  if (pCompBlkHead->magic_ != HIS_BLOCK_DATA_MAGIC ||
    pCompBlkHead->dataLen_ != (bufLen - sizeof(CompBlockHead)))
  {
    return PdbE_PAGE_ERROR;
  }

  uLongf rawLen = HIS_BLK_LEN;
  if (uncompress((uint8_t*)pRawBuf_, &rawLen, 
    (uint8_t*)(pBuf + sizeof(CompBlockHead)), pCompBlkHead->dataLen_) != Z_OK)
  {
    return PdbE_PAGE_ERROR;
  }

  const CompPageHead* pCompPageHead = (const CompPageHead*)pRawBuf_;
  if (pCompPageHead->fieldCnt_ != fieldCnt_)
  {
    return PdbE_PAGE_ERROR;
  }

  recCnt_ = pCompPageHead->recCnt_;
  if (pAllVals_ == nullptr || totalValCnt_ < (mateCnt_ * recCnt_))
  {
    if (pAllVals_ != nullptr)
      delete[] pAllVals_;

    pAllVals_ = new (std::nothrow) DBVal[mateCnt_ * recCnt_];
    if (pAllVals_ == nullptr)
      return PdbE_NOMEM;

    totalValCnt_ = mateCnt_ * recCnt_;
  }

  const char* pBlkLimit = pRawBuf_ + rawLen;
  const char* pTmp = pRawBuf_ + sizeof(CompPageHead);
  const char* pValsLimit = nullptr;

  //devid
  DBVAL_ELE_SET_INT64(pQueryVals_, PDB_DEVID_INDEX, pCompPageHead->devId_);

  DBVal* pValsBg = pAllVals_;
  //tstamp
  pTmp = Coding::VarintDecode32(pTmp, pBlkLimit, &u32);
  pValsLimit = pTmp + u32;
  DecodeTstampVals(pValsBg, pTmp, pValsLimit);
  pValsBg += recCnt_;

  for (size_t fieldIdx = (PDB_TSTAMP_INDEX + 1); fieldIdx < fieldCnt_; fieldIdx++)
  {
    pTmp = pValsLimit;
    pTmp = Coding::VarintDecode32(pTmp, pBlkLimit, &u32);
    pValsLimit = pTmp + u32;

    if (pFieldPos_[fieldIdx] > 0)
    {
      switch (pTypes_[fieldIdx])
      {
      case PDB_FIELD_TYPE::TYPE_BOOL:
        retVal = DecodeBoolVals(pValsBg, pTmp, pValsLimit);
        break;
      case PDB_FIELD_TYPE::TYPE_INT64:
        retVal = DecodeBigIntVals(pValsBg, pTmp, pValsLimit);
        break;
      case PDB_FIELD_TYPE::TYPE_DATETIME:
        retVal = DecodeDateTimeVals(pValsBg, pTmp, pValsLimit);
        break;
      case PDB_FIELD_TYPE::TYPE_DOUBLE:
        retVal = DecodeDoubleVals(pValsBg, pTmp, pValsLimit);
        break;
      case PDB_FIELD_TYPE::TYPE_STRING:
        retVal = DecodeStringVals(pValsBg, pTmp, pValsLimit);
        break;
      case PDB_FIELD_TYPE::TYPE_BLOB:
        retVal = DecodeBlobVals(pValsBg, pTmp, pValsLimit);
        break;
      case PDB_FIELD_TYPE::TYPE_REAL2:
        retVal = DecodeRealVals(pValsBg, pTmp, pValsLimit, DBVAL_REAL2_MULTIPLE);
        break;
      case PDB_FIELD_TYPE::TYPE_REAL3:
        retVal = DecodeRealVals(pValsBg, pTmp, pValsLimit, DBVAL_REAL3_MULTIPLE);
        break;
      case PDB_FIELD_TYPE::TYPE_REAL4:
        retVal = DecodeRealVals(pValsBg, pTmp, pValsLimit, DBVAL_REAL4_MULTIPLE);
        break;
      case PDB_FIELD_TYPE::TYPE_REAL6:
        retVal = DecodeRealVals(pValsBg, pTmp, pValsLimit, DBVAL_REAL6_MULTIPLE);
        break;
      default:
        retVal = PdbE_INVALID_FIELD_TYPE;
      }

      if (retVal != PdbE_OK)
        return retVal;

      pValsBg += recCnt_;
    }
  }

  return PdbE_OK;
}

bool CompDataPart::CompDataIter::Valid() const
{
  return curIdx_ >= 0 && curIdx_ < recCnt_;
}

PdbErr_t CompDataPart::CompDataIter::SeekTo(int64_t tstamp)
{
  int32_t lwr = 0;
  int32_t upr = recCnt_ - 1;
  int32_t idx = 0;
  int64_t curTs = 0;

  if (recCnt_ == 0)
    return PdbE_OK;

  while (lwr <= upr)
  {
    idx = (lwr + upr) / 2;
    curTs = DBVAL_ELE_GET_DATETIME(pAllVals_, idx);

    if (tstamp == curTs)
      break;
    else if (tstamp < curTs)
      upr = idx - 1;
    else
      lwr = idx + 1;
  }

  curIdx_ = idx;
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::SeekToFirst()
{
  curIdx_ = 0;
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::SeekToLast()
{
  curIdx_ = recCnt_ - 1;
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::Next()
{
  curIdx_++;
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::Prev()
{
  curIdx_--;
  return PdbE_OK;
}

DBVal* CompDataPart::CompDataIter::GetRecord()
{
  if (curIdx_ >= 0 && curIdx_ < recCnt_)
  {
    const DBVal* pValsBg = pAllVals_;

    for (size_t idx = PDB_TSTAMP_INDEX; idx < fieldCnt_; idx++)
    {
      if (pFieldPos_[idx] > 0)
      {
        pQueryVals_[pFieldPos_[idx]] = pValsBg[curIdx_];
        pValsBg += recCnt_;
      }
    }

    return pQueryVals_;
  }

  return nullptr;
}

PdbErr_t CompDataPart::CompDataIter::DecodeTstampVals(DBVal* pValBg, const char* pBuf, const char* pLimit)
{
  uint64_t u64 = 0;
  int64_t tstamp = 0;
  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode64(pBuf, pLimit, &u64);
    tstamp += u64;
    DBVAL_ELE_SET_DATETIME(pValBg, i, tstamp);
  }
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeBoolVals(DBVal* pValBg, const char* pBuf, const char* pLimit)
{
  for (int i = 0; i < recCnt_; i++)
  {
    DBVAL_ELE_SET_BOOL(pValBg, i, (BIT_MAP_IS_SET(pBuf, i) != 0));
  }
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeBigIntVals(DBVal* pValBg, const char* pBuf, const char* pLimit)
{
  uint64_t u64 = 0;
  int64_t i64 = 0;
  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode64(pBuf, pLimit, &u64);
    i64 += Coding::ZigzagDecode64(u64);

    DBVAL_ELE_SET_INT64(pValBg, i, i64);
  }
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeDateTimeVals(DBVal* pValBg, const char* pBuf, const char* pLimit)
{
  uint64_t u64 = 0;
  int64_t i64 = 0;
  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode64(pBuf, pLimit, &u64);
    i64 += Coding::ZigzagDecode64(u64);

    DBVAL_ELE_SET_DATETIME(pValBg, i, i64);
  }
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeDoubleVals(DBVal* pValBg, const char* pBuf, const char* pLimit)
{
  uint64_t u64Val = 0;
  uint64_t u64Tmp = 0;
  uint8_t u8 = 0;

  for (int i = 0; i < recCnt_; i++)
  {
    u64Tmp = 0;
    u8 = *pBuf;
    pBuf++;

    if ((u8 & 0x4) != 0)
    {
      pBuf = Coding::VarintDecode64(pBuf, pLimit, &u64Tmp);
    }

    u64Tmp = ((u64Tmp << 2) | (u8 & 0x3));
    u64Tmp <<= (((u8 & 0xF8) >> 3) * 2);

    u64Val ^= u64Tmp;
    DBVAL_ELE_SET_DOUBLE_FOR_UINT64(pValBg, i, u64Val);
  }

  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeStringVals(DBVal* pValBg, const char* pBuf, const char* pLimit)
{
  uint32_t u32 = 0;

  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode32(pBuf, pLimit, &u32);
    DBVAL_ELE_SET_STRING(pValBg, i, pBuf, u32);
    pBuf += u32;
  }

  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeBlobVals(DBVal* pValBg, const char* pBuf, const char* pLimit)
{
  uint32_t u32 = 0;

  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode32(pBuf, pLimit, &u32);
    DBVAL_ELE_SET_BLOB(pValBg, i, pBuf, u32);
    pBuf += u32;
  }

  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeRealVals(DBVal* pValBg, const char* pBuf, const char* pLimit, double multiple)
{
  int64_t i64 = 0;
  uint64_t u64 = 0;

  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode64(pBuf, pLimit, &u64);
    i64 += Coding::ZigzagDecode64(u64);
    DBVAL_ELE_SET_DOUBLE(pValBg, i, (i64 / multiple));
  }

  return PdbE_OK;
}




