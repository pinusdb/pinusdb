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
}

NormalDataPage::~NormalDataPage()
{
}

PdbErr_t NormalDataPage::Load(PageHdr* pPage)
{
  this->pPage_ = pPage;
  return PdbE_OK;
}

void NormalDataPage::Init(PageHdr* pPage, int32_t pageNo,
  int64_t devId, int64_t idxTs)
{
  pPage_ = pPage;
  memset(PAGEHDR_GET_PAGEDATA(pPage_), 0, NORMAL_PAGE_SIZE);
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  pHead->pageNo_ = pageNo;
  pHead->devId_ = devId;
  pHead->recCnt_ = 0;
  pHead->chipBytes_ = 0;
  pHead->freeBytes_ = static_cast<uint16_t>(NORMAL_PAGE_SIZE - sizeof(NormalDataHead));
  pHead->idxTs_ = idxTs;
  PAGEHDR_SET_INIT_TRUE(pPage);
}

int32_t NormalDataPage::GetPageNo() const
{
  assert(pPage_ != nullptr);
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  return pHead->pageNo_;
}
int64_t NormalDataPage::GetDevId() const
{
  assert(pPage_ != nullptr);
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  return pHead->devId_;
}
uint16_t NormalDataPage::GetRecCnt() const
{
  assert(pPage_ != nullptr);
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  return pHead->recCnt_;
}
uint16_t NormalDataPage::GetChipBytes() const
{
  assert(pPage_ != nullptr);
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  return pHead->chipBytes_;
}
uint16_t NormalDataPage::GetFreeBytes() const
{
  assert(pPage_ != nullptr);
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  return pHead->freeBytes_;
}
int64_t NormalDataPage::GetIdxTs() const
{
  assert(pPage_ != nullptr);
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  return pHead->idxTs_;
}

void NormalDataPage::UpdateCrc()
{
  assert(pPage_ != nullptr);
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  UPDATE_NORMAL_DATA_PAGE_CRC(pHead);
}

PdbErr_t NormalDataPage::InsertRec(int64_t ts, const PdbByte* pRec, size_t recLen, bool replace)
{
  PdbErr_t retVal = PdbE_OK;
  assert(pPage_ != nullptr);
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));

  if (pHead->freeBytes_ < (recLen + NORMAL_REC_IDX_LEN))
  {
    if ((pHead->freeBytes_ + pHead->chipBytes_) >= (recLen + NORMAL_REC_IDX_LEN))
      CleanUp();

    if (pHead->freeBytes_ < (recLen + NORMAL_REC_IDX_LEN))
      return PdbE_PAGE_FILL;
  }

  int lwr = 0;
  int upr = pHead->recCnt_ - 1;
  int idx = 0;
  int64_t curTs = 0;

  if (pHead->recCnt_ > 0)
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

  size_t recOff = sizeof(NormalDataHead) + 
    pHead->recCnt_ * NORMAL_REC_IDX_LEN + pHead->freeBytes_ - recLen;
  memcpy((PAGEHDR_GET_PAGEDATA(pPage_) + recOff), pRec, recLen);

  uint16_t* pIdxArr = (uint16_t*)(PAGEHDR_GET_PAGEDATA(pPage_) + sizeof(NormalDataHead));
  int insertPos = idx;
  if (curTs == ts)
  {
    //替换
    uint16_t oldRecLen = 0;
    const uint8_t* pOldRec = nullptr;
    retVal = GetRecData(insertPos, &pOldRec);
    if (retVal != PdbE_OK)
      return retVal;

    oldRecLen = Coding::FixedDecode16(pOldRec);
    pHead->chipBytes_ += oldRecLen;

    pIdxArr[insertPos] = static_cast<uint16_t>(recOff);
    pHead->freeBytes_ -= static_cast<uint16_t>(recLen);
    PAGEHDR_DATA_UPDATE(pPage_);
  }
  else
  {
    if (ts > curTs && insertPos < pHead->recCnt_)
      insertPos++;

    for (int i = pHead->recCnt_; i > insertPos; i--)
    {
      pIdxArr[i] = pIdxArr[i - 1];
    }
    pIdxArr[insertPos] = static_cast<uint16_t>(recOff);
    pHead->freeBytes_ -= static_cast<uint16_t>(recLen + NORMAL_REC_IDX_LEN);
    pHead->recCnt_++;
    PAGEHDR_DATA_UPDATE(pPage_);
  }

  return PdbE_OK;
}

PdbErr_t NormalDataPage::GetRecData(size_t idx, const PdbByte** ppRec)
{
  assert(pPage_ != nullptr);
  const PdbByte* pPageData = PAGEHDR_GET_PAGEDATA(pPage_);
  const uint16_t* pPageIdxs = (uint16_t*)(pPageData + sizeof(NormalDataHead));
  const NormalDataHead* pHead = (NormalDataHead*)pPageData;
  if (idx >= pHead->recCnt_)
    return PdbE_INVALID_PARAM;

  if (ppRec != nullptr)
    *ppRec = pPageData + pPageIdxs[idx];

  return PdbE_OK;
}

PdbErr_t NormalDataPage::GetRecTstamp(size_t idx, int64_t* pTstamp)
{
  assert(pPage_ != nullptr);
  const PdbByte* pPageData = PAGEHDR_GET_PAGEDATA(pPage_);
  const PdbByte* pPageEd = pPageData + NORMAL_PAGE_SIZE;
  const uint16_t* pPageIdxs = (uint16_t*)(pPageData + sizeof(NormalDataHead));
  const NormalDataHead* pHead = (NormalDataHead*)pPageData;
  if (idx >= pHead->recCnt_)
    return PdbE_INVALID_PARAM;

  if (pTstamp != nullptr)
  {
    const PdbByte* pRecData = pPageData + pPageIdxs[idx];
    uint64_t uint64val = 0;
    Coding::VarintDecode64((pRecData + NORMAL_REC_LEN_LEN), pPageEd, &uint64val);
    *pTstamp = uint64val;
  }

  return PdbE_OK;
}

PdbErr_t NormalDataPage::GetLastTstamp(int64_t* pTstamp)
{
  assert(pPage_ != nullptr);
  const PdbByte* pPageData = PAGEHDR_GET_PAGEDATA(pPage_);
  const PdbByte* pPageEd = pPageData + NORMAL_PAGE_SIZE;
  const uint16_t* pPageIdxs = (uint16_t*)(pPageData + sizeof(NormalDataHead));
  const NormalDataHead* pHead = (NormalDataHead*)pPageData;

  if (pTstamp != nullptr)
  {
    const PdbByte* pRecData = pPageData + pPageIdxs[pHead->recCnt_ - 1];
    uint64_t uint64val = 0;
    Coding::VarintDecode64((pRecData + NORMAL_REC_LEN_LEN), pPageEd, &uint64val);
    *pTstamp = uint64val;
  }

  return PdbE_OK;

}

//分裂指定的大小到新页, pNewPage是一个新页
PdbErr_t NormalDataPage::SplitMost(NormalDataPage* pNewPage, int32_t bytes)
{
  PdbErr_t retVal = PdbE_OK;

  assert(pPage_ != nullptr);
  assert(pNewPage != nullptr);
  PdbByte* pRecBg = nullptr;
  PdbByte* pPageData = PAGEHDR_GET_PAGEDATA(pPage_);
  uint16_t* pPageIdxs = (uint16_t*)(pPageData + sizeof(NormalDataHead));
  NormalDataHead* pHead = (NormalDataHead*)pPageData;

  int32_t recLen = 0;
  int32_t transBytes = 0;
  int idx = pHead->recCnt_ - 1;
  while (idx > 0)
  {
    pRecBg = pPageData + pPageIdxs[idx];
    recLen = Coding::FixedDecode16(pRecBg);

    if ((transBytes + recLen + NORMAL_REC_IDX_LEN) > bytes)
      break;

    retVal = pNewPage->InsertForPos(0, (const uint8_t*)pRecBg, recLen);
    if (retVal != PdbE_OK)
      return retVal;

    transBytes += (recLen + NORMAL_REC_IDX_LEN);
    idx--;
  }

  pHead->recCnt_ = idx + 1;
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

  PdbByte* pPageData = PAGEHDR_GET_PAGEDATA(pPage_);
  NormalDataHead* pPageHead = (NormalDataHead*)pPageData;
  uint16_t* pPageIdxs = (uint16_t*)(pPageData + sizeof(NormalDataHead));
  uint16_t recLen = 0;

  char* pTmpEnd = pTmpBuf + NORMAL_PAGE_SIZE;
  uint16_t* pTmpIdxs = (uint16_t*)(pTmpBuf + sizeof(NormalDataHead));

  for (int i = 0; i < pPageHead->recCnt_; i++)
  {
    const PdbByte* pBg = pPageData + pPageIdxs[i];
    recLen = Coding::FixedDecode16(pBg);

    pTmpEnd -= recLen;
    memcpy(pTmpEnd, pBg, recLen);
    pTmpIdxs[i] = static_cast<uint16_t>(pTmpEnd - pTmpBuf);
  }

  NormalDataHead* pTmpHead = (NormalDataHead*)pTmpBuf;
  pTmpHead->pageNo_ = pPageHead->pageNo_;
  pTmpHead->devId_ = pPageHead->devId_;
  pTmpHead->idxTs_ = pPageHead->idxTs_;
  pTmpHead->recCnt_ = pPageHead->recCnt_;
  pTmpHead->chipBytes_ = 0;
  pTmpHead->freeBytes_ = static_cast<uint16_t>(pTmpEnd - pTmpBuf - sizeof(NormalDataHead) - pPageHead->recCnt_ * NORMAL_REC_IDX_LEN);

  memcpy(pPageData, pTmpBuf, NORMAL_PAGE_SIZE);
  return PdbE_OK;
}

PdbErr_t NormalDataPage::UpdateIdxTs()
{
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));
  int64_t idxTs = 0;
  GetRecTstamp(0, &idxTs);
  pHead->idxTs_ = idxTs;
  return PdbE_OK;
}

PdbErr_t NormalDataPage::InsertForPos(int idx, const uint8_t* pRec, size_t recLen)
{
  assert(pPage_ != nullptr);
  NormalDataHead* pHead = (NormalDataHead*)(PAGEHDR_GET_PAGEDATA(pPage_));

  if (pHead->freeBytes_ < (recLen + NORMAL_REC_IDX_LEN))
    return PdbE_PAGE_FILL;

  size_t recOff = sizeof(NormalDataHead) + pHead->recCnt_ * NORMAL_REC_IDX_LEN + pHead->freeBytes_ - recLen;
  memcpy((PAGEHDR_GET_PAGEDATA(pPage_) + recOff), pRec, recLen);

  uint16_t* pIdxArr = (uint16_t*)(PAGEHDR_GET_PAGEDATA(pPage_) + sizeof(NormalDataHead));

  for (int i = pHead->recCnt_; i > idx; i--)
  {
    pIdxArr[i] = pIdxArr[i - 1];
  }
  pIdxArr[idx] = static_cast<uint16_t>(recOff);
  pHead->freeBytes_ -= static_cast<uint16_t>(recLen + NORMAL_REC_IDX_LEN);
  pHead->recCnt_++;
  PAGEHDR_DATA_UPDATE(pPage_);

  return PdbE_OK;
}
