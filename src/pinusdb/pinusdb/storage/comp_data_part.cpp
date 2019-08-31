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

PdbErr_t CompDataPart::Open(int32_t partCode, const TableInfo* pTabInfo,
  const char* pDataPath)
{
  PdbErr_t retVal = PdbE_OK;
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

PdbErr_t CompDataPart::InsertRec(int64_t devId, int64_t tstamp,
  bool replace, const uint8_t* pRec, size_t recLen)
{
  return PdbE_FILE_READONLY;
}

PdbErr_t CompDataPart::UnMap()
{
  if (refCnt_ == 0 && ((lastQueryTime_ + 600000) < GetTickCount64()) && pData_ != nullptr)
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
  IResultFilter* pResult, uint64_t timeOut, bool queryFirst, bool* pIsAdd)
{
  PdbErr_t retVal = PdbE_OK;
  CompDevId compDevId;
  CompDataIter* pHisIter = (CompDataIter*)pQueryParam;
  int64_t bgTs = pHisIter->GetBgTs();
  int64_t edTs = pHisIter->GetEdTs();
  size_t fieldCnt = pHisIter->GetFieldCnt();
  size_t rawLen = 0;
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

  lastQueryTime_ = GetTickCount64();

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
      retVal = pResult->AppendData(pVals, fieldCnt, &isAdd);
      if (retVal != PdbE_OK)
        return retVal;

      if (queryFirst && isAdd)
      {
        if (pIsAdd != nullptr)
          *pIsAdd = true;

        return PdbE_OK;
      }

      if (pResult->GetIsFullFlag())
        return PdbE_OK;

      curTs = DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX);
      if (curTs >= edTs)
        return PdbE_OK;

      pHisIter->Next();
    }

    if (GetTickCount64() > timeOut)
      return PdbE_QUERY_TIME_OUT;
  }

  return PdbE_OK;
}

PdbErr_t CompDataPart::QueryDevDesc(int64_t devId, void* pQueryParam,
  IResultFilter* pResult, uint64_t timeOut, bool queryLast, bool* pIsAdd)
{
  PdbErr_t retVal = PdbE_OK;
  CompDevId compDevId;
  CompDataIter* pHisIter = (CompDataIter*)pQueryParam;
  int64_t bgTs = pHisIter->GetBgTs();
  int64_t edTs = pHisIter->GetEdTs();
  size_t fieldCnt = pHisIter->GetFieldCnt();
  size_t rawLen = 0;
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

  lastQueryTime_ = GetTickCount64();

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
      retVal = pResult->AppendData(pVals, fieldCnt, &isAdd);
      if (retVal != PdbE_OK)
        return retVal;

      if (queryLast && isAdd)
      {
        if (pIsAdd != nullptr)
          *pIsAdd = true;

        return PdbE_OK;
      }

      if (pResult->GetIsFullFlag())
        return PdbE_OK;

      curTs = DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX);
      if (curTs <= bgTs)
        return PdbE_OK;

      pHisIter->Prev();
    }

    if (GetTickCount64() > timeOut)
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

    const uint8_t* pTmpBase = dataMemMap_.GetBaseAddr();
    size_t dataSize = dataMemMap_.MemMapSize();

    const CompDataFooter* pFooter = (const CompDataFooter*)(pTmpBase + dataSize - sizeof(CompDataFooter));
    pDevId_ = (const CompDevId*)(pTmpBase + pFooter->devIdsPos_);
    devCnt_ = static_cast<int32_t>(pFooter->devCnt_);
    pData_ = pTmpBase;
    lastQueryTime_ = GetTickCount64();
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


void* CompDataPart::InitQueryParam(int* pTypes, size_t fieldCnt, int64_t bgTs, int64_t edTs)
{
  CompDataIter* pHisIter = new (std::nothrow)CompDataIter();
  if (pHisIter == nullptr)
    return nullptr;

  if (pHisIter->Init(pTypes, fieldCnt, bgTs, edTs) != PdbE_OK)
  {
    delete pHisIter;
    return nullptr;
  }

  return pHisIter;
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

PdbErr_t CompDataPart::CompDataIter::Init(int* pTypes, size_t fieldCnt, int64_t bgTs, int64_t edTs)
{
  fieldCnt_ = fieldCnt;
  pTypes_ = (int*)arena_.AllocateAligned((sizeof(int) * fieldCnt));
  pRawBuf_ = (uint8_t*)arena_.AllocateAligned(HIS_BLK_LEN);

  if (pTypes_ == nullptr || pRawBuf_ == nullptr)
    return PdbE_NOMEM;

  memcpy(pTypes_, pTypes, (sizeof(int) * fieldCnt));
  bgTs_ = bgTs;
  edTs_ = edTs;

  recCnt_ = 0;
  curIdx_ = 0;
  totalValCnt_ = 0;

  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::Load(const uint8_t* pBuf, size_t bufLen)
{
  uint32_t u32 = 0;
  const CompBlockHead* pCompBlkHead = (const CompBlockHead*)pBuf;
  if (pCompBlkHead->magic_ != HIS_BLOCK_DATA_MAGIC ||
    pCompBlkHead->dataLen_ != (bufLen - sizeof(CompBlockHead)))
  {
    return PdbE_PAGE_ERROR;
  }

  uLongf rawLen = HIS_BLK_LEN;
  if (uncompress(pRawBuf_, &rawLen, (pBuf + sizeof(CompBlockHead)), pCompBlkHead->dataLen_) != Z_OK)
  {
    return PdbE_PAGE_ERROR;
  }

  const CompPageHead* pCompPageHead = (const CompPageHead*)pRawBuf_;

  if (pCompPageHead->fieldCnt_ != fieldCnt_)
  {
    return PdbE_PAGE_ERROR;
  }

  recCnt_ = pCompPageHead->recCnt_;
  
  if (pAllVals_ == nullptr || totalValCnt_ < (fieldCnt_ * recCnt_))
  {
    if (pAllVals_ != nullptr)
      delete[] pAllVals_;

    pAllVals_ = new (std::nothrow) DBVal[fieldCnt_ * recCnt_];
    if (pAllVals_ == nullptr)
      return PdbE_NOMEM;

    totalValCnt_ = fieldCnt_ * recCnt_;
  }

  const uint8_t* pBlkLimit = pRawBuf_ + rawLen;
  const uint8_t* pTmp = pRawBuf_ + sizeof(CompPageHead);
  const uint8_t* pValsLimit = nullptr;

  //devid
  SetDevIdVals(pCompPageHead->devId_);

  //tstamp
  pTmp = Coding::VarintDecode32(pTmp, pBlkLimit, &u32);
  pValsLimit = pTmp + u32;
  DecodeTstampVals(pTmp, pValsLimit);

  for (int fieldIdx = (PDB_TSTAMP_INDEX + 1); fieldIdx < fieldCnt_; fieldIdx++)
  {
    pTmp = pValsLimit;
    pTmp = Coding::VarintDecode32(pTmp, pBlkLimit, &u32);
    pValsLimit = pTmp + u32;

    switch (pTypes_[fieldIdx])
    {
    case PDB_FIELD_TYPE::TYPE_BOOL:
      DecodeBoolVals(fieldIdx, pTmp, pValsLimit);
      break;
    case PDB_FIELD_TYPE::TYPE_INT64:
      DecodeBigIntVals(fieldIdx, pTmp, pValsLimit);
      break;
    case PDB_FIELD_TYPE::TYPE_DATETIME:
      DecodeDateTimeVals(fieldIdx, pTmp, pValsLimit);
      break;
    case PDB_FIELD_TYPE::TYPE_DOUBLE:
      DecodeDoubleVals(fieldIdx, pTmp, pValsLimit);
      break;
    case PDB_FIELD_TYPE::TYPE_STRING:
      DecodeStringVals(fieldIdx, pTmp, pValsLimit);
      break;
    case PDB_FIELD_TYPE::TYPE_BLOB:
      DecodeBlobVals(fieldIdx, pTmp, pValsLimit);
      break;
    case PDB_FIELD_TYPE::TYPE_REAL2:
      DecodeRealVals(fieldIdx, pTmp, pValsLimit, DBVAL_REAL2_MULTIPLE);
      break;
    case PDB_FIELD_TYPE::TYPE_REAL3:
      DecodeRealVals(fieldIdx, pTmp, pValsLimit, DBVAL_REAL3_MULTIPLE);
      break;
    case PDB_FIELD_TYPE::TYPE_REAL4:
      DecodeRealVals(fieldIdx, pTmp, pValsLimit, DBVAL_REAL4_MULTIPLE);
      break;
    case PDB_FIELD_TYPE::TYPE_REAL6:
      DecodeRealVals(fieldIdx, pTmp, pValsLimit, DBVAL_REAL6_MULTIPLE);
      break;
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
    curTs = DBVAL_ELE_GET_DATETIME(pAllVals_, (idx * fieldCnt_ + PDB_TSTAMP_INDEX));

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
    return pAllVals_ + curIdx_ * fieldCnt_;
  }

  return nullptr;
}

PdbErr_t CompDataPart::CompDataIter::SetDevIdVals(int64_t devId)
{
  for (int i = 0; i < recCnt_; i++)
  {
    DBVAL_ELE_SET_INT64(pAllVals_, (i * fieldCnt_), devId);
  }
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeTstampVals(const uint8_t* pBuf, const uint8_t* pLimit)
{
  uint64_t u64 = 0;
  int64_t tstamp = 0;
  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode64(pBuf, pLimit, &u64);
    tstamp += u64;
    DBVAL_ELE_SET_DATETIME(pAllVals_, (i * fieldCnt_ + PDB_TSTAMP_INDEX), tstamp);
  }
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeBoolVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit)
{
  for (int i = 0; i < recCnt_; i++)
  {
    DBVAL_ELE_SET_BOOL(pAllVals_, (i * fieldCnt_ + fieldIdx), (BIT_MAP_IS_SET(pBuf, i) != 0));
  }
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeBigIntVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit)
{
  uint64_t u64 = 0;
  int64_t i64 = 0;
  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode64(pBuf, pLimit, &u64);
    i64 += Coding::ZigzagDecode64(u64);

    DBVAL_ELE_SET_INT64(pAllVals_, (i * fieldCnt_ + fieldIdx), i64);
  }
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeDateTimeVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit)
{
  uint64_t u64 = 0;
  int64_t i64 = 0;
  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode64(pBuf, pLimit, &u64);
    i64 += Coding::ZigzagDecode64(u64);

    DBVAL_ELE_SET_DATETIME(pAllVals_, (i * fieldCnt_ + fieldIdx), i64);
  }
  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeDoubleVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit)
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
    DBVAL_ELE_SET_DOUBLE_FOR_UINT64(pAllVals_, (i * fieldCnt_ + fieldIdx), u64Val);
  }

  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeStringVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit)
{
  uint32_t u32 = 0;

  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode32(pBuf, pLimit, &u32);
    DBVAL_ELE_SET_STRING(pAllVals_, (i * fieldCnt_ + fieldIdx), pBuf, u32);
    pBuf += u32;
  }

  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeBlobVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit)
{
  uint32_t u32 = 0;

  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode32(pBuf, pLimit, &u32);
    DBVAL_ELE_SET_BLOB(pAllVals_, (i * fieldCnt_ + fieldIdx), pBuf, u32);
    pBuf += u32;
  }

  return PdbE_OK;
}

PdbErr_t CompDataPart::CompDataIter::DecodeRealVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit, double multiple)
{
  int64_t i64 = 0;
  uint64_t u64 = 0;

  for (int i = 0; i < recCnt_; i++)
  {
    pBuf = Coding::VarintDecode64(pBuf, pLimit, &u64);
    i64 += Coding::ZigzagDecode64(u64);
    DBVAL_ELE_SET_DOUBLE(pAllVals_, (i * fieldCnt_ + fieldIdx), (i64 / multiple));
  }

  return PdbE_OK;
}




