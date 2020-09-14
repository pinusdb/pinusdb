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

#include "storage/normal_data_page.h"
#include "util/string_tool.h"
#include "util/log_util.h"
#include "util/arena.h"
#include "util/coding.h"
#include <assert.h>

#define NORMAL_REC_IDX_LEN     2
#define NORMAL_REC_LEN_LEN     2

//////////////////////////////////////////////////////////

NormalDataPage::NormalDataPage()
{
  this->pPage_ = nullptr;
  this->devId_ = 0;
  this->idxTs_ = 0;
  this->pageNo_ = 0;
  this->recCnt_ = 0;
  this->chipBytes_ = 0;
  this->freeBytes_ = 0;
}

NormalDataPage::~NormalDataPage()
{
}

PdbErr_t NormalDataPage::Load(PageHdr* pPage)
{
  pPage_ = pPage;
  if (pPage_ != nullptr)
  {
    NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
    devId_ = NormalDataHead_GetDevId(pHead);
    idxTs_ = NormalDataHead_GetIdxTs(pHead);
    pageNo_ = NormalDataHead_GetPageNo(pHead);
    recCnt_ = NormalDataHead_GetRecCnt(pHead);
    chipBytes_ = NormalDataHead_GetChipBytes(pHead);
    freeBytes_ = NormalDataHead_GetFreeBytes(pHead);
  }
  else
  {
    devId_ = 0;
    idxTs_ = 0;
    pageNo_ = 0;
    recCnt_ = 0;
    chipBytes_ = 0;
    freeBytes_ = 0;
  }
  return PdbE_OK;
}

void NormalDataPage::Init(PageHdr* pPage, int32_t pageNo,
  int64_t devId, int64_t idxTs)
{
  pPage_ = pPage;
  memset(PAGEHDR_GET_PAGEDATA(pPage_), 0, NORMAL_PAGE_SIZE);
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  devId_ = devId;
  idxTs_ = idxTs;
  pageNo_ = pageNo;
  recCnt_ = 0;
  chipBytes_ = 0;
  freeBytes_ = static_cast<uint16_t>(NORMAL_PAGE_SIZE - sizeof(NormalDataHead));
  NormalDataHead_SetPageNo(pHead, pageNo_);
  NormalDataHead_SetDevId(pHead, devId_);
  NormalDataHead_SetRecCnt(pHead, static_cast<uint16_t>(recCnt_));
  NormalDataHead_SetChipBytes(pHead, static_cast<uint16_t>(chipBytes_));
  NormalDataHead_SetFreeBytes(pHead, static_cast<uint16_t>(freeBytes_));
  NormalDataHead_SetIdxTs(pHead, idxTs_);

  PAGEHDR_SET_INIT_TRUE(pPage);
}

void NormalDataPage::UpdateCrc()
{
  if (pPage_)
  {
    NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
    UPDATE_NORMAL_DATA_PAGE_CRC(pHead);
  }
}

PdbErr_t NormalDataPage::InsertRec(int64_t ts, const char* pRec, size_t recLen, bool replace)
{
  PdbErr_t retVal = PdbE_OK;
  if (!pPage_)
  {
    return PdbE_INVALID_PARAM;
  }

  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  if (freeBytes_ < (recLen + NORMAL_REC_IDX_LEN))
  {
    if ((freeBytes_ + chipBytes_) >= (recLen + NORMAL_REC_IDX_LEN))
      CleanUp();

    if (freeBytes_ < (recLen + NORMAL_REC_IDX_LEN))
      return PdbE_PAGE_FILL;
  }

  int lwr = 0;
  int upr = static_cast<int>(recCnt_ - 1);
  int idx = 0;
  int64_t curTs = 0;

  if (recCnt_ > 0)
  {
    idx = upr;
    retVal = GetRecTstamp(idx, &curTs);
    if (retVal != PdbE_OK)
      return retVal;

    if (ts < curTs)
    {
      while (lwr <= upr)
      {
        idx = (lwr + upr) / 2;
        retVal = GetRecTstamp(idx, &curTs);
        if (retVal != PdbE_OK)
          return retVal;

        if (curTs == ts)
          break;
        else if (curTs < ts)
          lwr = idx + 1;
        else
          upr = idx - 1;
      }
    }
  }

  if (curTs == ts && !replace)
    return PdbE_RECORD_EXIST; //记录已经存在

  size_t recOff = sizeof(NormalDataHead) + recCnt_ * NORMAL_REC_IDX_LEN + freeBytes_ - recLen;
  memcpy((PAGEHDR_GET_PAGEDATA(pPage_) + recOff), pRec, recLen);

  char* pIdxArr = PAGEHDR_GET_PAGEDATA(pPage_) + sizeof(NormalDataHead);
  size_t insertPos = idx;
  if (curTs == ts)
  {
    //替换
    const char* pOldRec = nullptr;
    retVal = GetRecData(insertPos, &pOldRec);
    if (retVal != PdbE_OK)
      return retVal;

    chipBytes_ += Coding::FixedDecode16(pOldRec);
    Coding::FixedEncode16(pIdxArr + insertPos * NORMAL_REC_IDX_LEN, static_cast<uint16_t>(recOff));
    freeBytes_ -= recLen;
  }
  else
  {
    if (ts > curTs && insertPos < recCnt_)
      insertPos++;

    char* pTmpIdx = pIdxArr + insertPos * NORMAL_REC_IDX_LEN;
    if (insertPos < recCnt_)
    {
      std::memmove(pTmpIdx + NORMAL_REC_IDX_LEN, pTmpIdx, ((recCnt_ - insertPos) * NORMAL_REC_IDX_LEN));
    }

    Coding::FixedEncode16(pTmpIdx, static_cast<uint16_t>(recOff));
    freeBytes_ -= (recLen + NORMAL_REC_IDX_LEN);
    recCnt_++;
  }

  NormalDataHead_SetRecCnt(pHead, static_cast<uint16_t>(recCnt_));
  NormalDataHead_SetChipBytes(pHead, static_cast<uint16_t>(chipBytes_));
  NormalDataHead_SetFreeBytes(pHead, static_cast<uint16_t>(freeBytes_));
  PAGEHDR_DATA_UPDATE(pPage_);
  return PdbE_OK;
}

PdbErr_t NormalDataPage::GetRecData(size_t idx, const char** ppRec) const
{
  if (!pPage_ || idx >= recCnt_)
    return PdbE_INVALID_PARAM;

  const char* pPageData = PAGEHDR_GET_PAGEDATA(pPage_);
  const char* pPageIdxs = (char*)(pPageData + sizeof(NormalDataHead));
  
  if (ppRec != nullptr)
    *ppRec = pPageData + Coding::FixedDecode16(pPageIdxs + idx * NORMAL_REC_IDX_LEN);

  return PdbE_OK;
}

PdbErr_t NormalDataPage::GetRecTstamp(size_t idx, int64_t* pTstamp) const
{
  if (!pPage_ || idx >= recCnt_)
    return PdbE_INVALID_PARAM;

  const char* pPageData = PAGEHDR_GET_PAGEDATA(pPage_);
  const char* pPageIdxs = (char*)(pPageData + sizeof(NormalDataHead));

  if (pTstamp != nullptr)
  {
    const char* pRecData = pPageData + Coding::FixedDecode16(pPageIdxs + idx * NORMAL_REC_IDX_LEN);
    *pTstamp = Coding::FixedDecode64(pRecData + NORMAL_REC_LEN_LEN);
  }

  return PdbE_OK;
}

PdbErr_t NormalDataPage::GetLastTstamp(int64_t* pTstamp) const
{
  if (!pPage_ || recCnt_ == 0)
    return PdbE_INVALID_PARAM;

  const char* pPageData = PAGEHDR_GET_PAGEDATA(pPage_);
  const char* pPageIdxs = (char*)(pPageData + sizeof(NormalDataHead));

  if (pTstamp != nullptr)
  {
    const char* pRecData = pPageData + Coding::FixedDecode16(pPageIdxs + (recCnt_ - 1) * NORMAL_REC_IDX_LEN);
    *pTstamp = Coding::FixedDecode64(pRecData + NORMAL_REC_LEN_LEN);
  }

  return PdbE_OK;
}

//分裂指定的大小到新页, pNewPage是一个新页
PdbErr_t NormalDataPage::SplitMost(NormalDataPage* pNewPage, int32_t bytes)
{
  PdbErr_t retVal = PdbE_OK;

  if (!pPage_ || !pNewPage)
    return PdbE_INVALID_PARAM;

  char* pRecBg = nullptr;
  char* pPageData = PAGEHDR_GET_PAGEDATA(pPage_);
  char* pPageIdxs = pPageData + sizeof(NormalDataHead);

  int32_t recLen = 0;
  int32_t transBytes = 0;
  size_t idx = recCnt_ - 1;
  while (idx > 0)
  {
    pRecBg = pPageData + Coding::FixedDecode16(pPageIdxs + (idx * NORMAL_REC_IDX_LEN));
    recLen = Coding::FixedDecode16(pRecBg);

    if ((transBytes + recLen + NORMAL_REC_IDX_LEN) > bytes)
      break;

    retVal = pNewPage->InsertForPos(0, (const uint8_t*)pRecBg, recLen);
    if (retVal != PdbE_OK)
      return retVal;

    transBytes += (recLen + NORMAL_REC_IDX_LEN);
    idx--;
  }

  recCnt_ = idx + 1;
  CleanUp();
  PAGEHDR_DATA_UPDATE(pPage_);
  PAGEHDR_SET_ONLY_MEM(pNewPage->pPage_);
  //更新pNewPage的开始时间
  pNewPage->UpdateIdxTs();

  return PdbE_OK;
}

PdbErr_t NormalDataPage::CleanUp()
{
  //整理实际上并没有修改数据的内容
  Arena arena;
  char* pTmpBuf = arena.Allocate(NORMAL_PAGE_SIZE);
  if (pTmpBuf == nullptr)
    return PdbE_NOMEM;

  memset(pTmpBuf, 0, NORMAL_PAGE_SIZE);

  char* pPageData = PAGEHDR_GET_PAGEDATA(pPage_);
  char* pRecIdxs = pPageData + sizeof(NormalDataHead);
  uint16_t recLen = 0;

  char* pTmpEnd = pTmpBuf + NORMAL_PAGE_SIZE;
  char* pTmpIdxs = pTmpBuf + sizeof(NormalDataHead);

  for (size_t i = 0; i < recCnt_; i++)
  {
    const char* pBg = pPageData + Coding::FixedDecode16(pRecIdxs + i * NORMAL_REC_IDX_LEN);
    recLen = Coding::FixedDecode16(pBg);

    pTmpEnd -= recLen;
    memcpy(pTmpEnd, pBg, recLen);
    Coding::FixedEncode16((pTmpIdxs + i * NORMAL_REC_IDX_LEN), static_cast<uint16_t>(pTmpEnd - pTmpBuf));
  }

  NormalDataHead* pTmpHead = (NormalDataHead*)pTmpBuf;
  chipBytes_ = 0;
  freeBytes_ = pTmpEnd - pTmpBuf - sizeof(NormalDataHead) - recCnt_ * NORMAL_REC_IDX_LEN;
  NormalDataHead_SetPageNo(pTmpHead, pageNo_);
  NormalDataHead_SetDevId(pTmpHead, devId_);
  NormalDataHead_SetIdxTs(pTmpHead, idxTs_);
  NormalDataHead_SetRecCnt(pTmpHead, recCnt_);
  NormalDataHead_SetChipBytes(pTmpHead, static_cast<uint16_t>(chipBytes_));
  NormalDataHead_SetFreeBytes(pTmpHead, static_cast<uint16_t>(freeBytes_));

  memcpy(pPageData, pTmpBuf, NORMAL_PAGE_SIZE);
  return PdbE_OK;
}

PdbErr_t NormalDataPage::UpdateIdxTs()
{
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  GetRecTstamp(0, &idxTs_);
  NormalDataHead_SetIdxTs(pHead, idxTs_);
  return PdbE_OK;
}

PdbErr_t NormalDataPage::InsertForPos(int idx, const uint8_t* pRec, size_t recLen)
{
  if (pPage_ == nullptr)
    return PdbE_INVALID_PARAM;

  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));

  if (freeBytes_ < (recLen + NORMAL_REC_IDX_LEN))
    return PdbE_PAGE_FILL;

  size_t recOff = sizeof(NormalDataHead) + recCnt_ * NORMAL_REC_IDX_LEN + freeBytes_ - recLen;
  memcpy((PAGEHDR_GET_PAGEDATA(pPage_) + recOff), pRec, recLen);

  char* pRecIdxs = PAGEHDR_GET_PAGEDATA(pPage_) + sizeof(NormalDataHead);
  char* pTmpIdx = pRecIdxs + (size_t)idx * NORMAL_REC_IDX_LEN;
  if (idx < recCnt_)
  {
    std::memmove(pTmpIdx + NORMAL_REC_IDX_LEN, pTmpIdx, ((recCnt_ - idx) * NORMAL_REC_IDX_LEN));
  }

  Coding::FixedEncode16(pTmpIdx, static_cast<uint16_t>(recOff));
  freeBytes_ -= (recLen + NORMAL_REC_IDX_LEN);
  recCnt_++;
  NormalDataHead_SetRecCnt(pHead, static_cast<uint16_t>(recCnt_));
  NormalDataHead_SetChipBytes(pHead, static_cast<uint16_t>(chipBytes_));
  NormalDataHead_SetFreeBytes(pHead, static_cast<uint16_t>(freeBytes_));
  PAGEHDR_DATA_UPDATE(pPage_);
  return PdbE_OK;
}
