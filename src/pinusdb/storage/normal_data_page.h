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

#pragma once
#include "internal.h"
#include "storage/page_hdr.h"
#include "table/db_value.h"
#include "util/coding.h"

typedef struct _NormalDataHead
{
  char _pageCrc_[4];        //页的校验码, uint32_t
  char _pageNo_[4];         //页号, uint32_t
  char _devId_[8];          //设备ID, int64_t
  char _idxTs_[8];          //索引时间戳, int64_t
  char _recCnt_[2];         //记录条数, uint16_t
  char _chipBytes_[2];      //回收字节数, uint16_t
  char _freeBytes_[2];      //空闲字节数
  char padding_[2];        //
}NormalDataHead;

#define NormalDataHead_GetPageCrc(pHead)                Coding::FixedDecode32((pHead)->_pageCrc_)
#define NormalDataHead_SetPageCrc(pHead, pageCrc)       Coding::FixedEncode32((pHead)->_pageCrc_, (pageCrc))
#define NormalDataHead_GetPageNo(pHead)                 Coding::FixedDecode32((pHead)->_pageNo_)
#define NormalDataHead_SetPageNo(pHead, pageNo)         Coding::FixedEncode32((pHead)->_pageNo_, (pageNo))
#define NormalDataHead_GetPageOffset(pHead)             (static_cast<int64_t>(Coding::FixedDecode32((pHead)->_pageNo_)) * NORMAL_PAGE_SIZE)
#define NormalDataHead_GetDevId(pHead)                  Coding::FixedDecode64((pHead)->_devId_)
#define NormalDataHead_SetDevId(pHead, devId)           Coding::FixedEncode64((pHead)->_devId_, (devId))
#define NormalDataHead_GetIdxTs(pHead)                  Coding::FixedDecode64((pHead)->_idxTs_)
#define NormalDataHead_SetIdxTs(pHead, idxTs)           Coding::FixedEncode64((pHead)->_idxTs_, (idxTs))
#define NormalDataHead_GetRecCnt(pHead)                 Coding::FixedDecode16((pHead)->_recCnt_)
#define NormalDataHead_SetRecCnt(pHead, recCnt)         Coding::FixedEncode16((pHead)->_recCnt_, (recCnt))
#define NormalDataHead_GetChipBytes(pHead)              Coding::FixedDecode16((pHead)->_chipBytes_)
#define NormalDataHead_SetChipBytes(pHead, chipBytes)   Coding::FixedEncode16((pHead)->_chipBytes_, (chipBytes))
#define NormalDataHead_GetFreeBytes(pHead)              Coding::FixedDecode16((pHead)->_freeBytes_)
#define NormalDataHead_SetFreeBytes(pHead, freeBytes)   Coding::FixedEncode16((pHead)->_freeBytes_, (freeBytes))

#define UPDATE_NORMAL_DATA_PAGE_CRC(pDataHead) do \
{ \
  NormalDataHead_SetPageCrc(pDataHead, StringTool::CRC32(pDataHead, (NORMAL_PAGE_SIZE - sizeof(uint32_t)), sizeof(uint32_t))); \
}while(false)

class NormalDataPage
{
public:
  NormalDataPage();
  ~NormalDataPage();

  PdbErr_t Load(PageHdr* pPage);
  void Init(PageHdr* pPage, int32_t pageNo, int64_t devId, int64_t idxTs);

  int32_t GetPageNo() const { return pageNo_; }
  int64_t GetDevId() const { return devId_; }
  uint16_t GetRecCnt() const { return static_cast<uint16_t>(recCnt_); }
  uint16_t GetChipBytes() const { return static_cast<uint16_t>(chipBytes_); }
  uint16_t GetFreeBytes() const { return static_cast<uint16_t>(freeBytes_); }
  int64_t GetIdxTs() const { return idxTs_; }

  void UpdateCrc();

  PdbErr_t InsertRec(int64_t ts, const char* pRec, size_t recLen, bool replace);
  PdbErr_t GetRecData(size_t idx, const char** ppRec) const;
  PdbErr_t GetRecTstamp(size_t idx, int64_t* pTstamp) const;
  PdbErr_t GetLastTstamp(int64_t* pTstamp) const;

  //分裂最多指定的字节
  PdbErr_t SplitMost(NormalDataPage* pNewPage, int32_t bytes);

private:
  PdbErr_t UpdateIdxTs();
  PdbErr_t CleanUp();
  PdbErr_t InsertForPos(int idx, const uint8_t* pRec, size_t recLen);

private:
  PageHdr* pPage_;
  int64_t devId_;
  int64_t idxTs_;
  int32_t pageNo_;
  size_t recCnt_;
  size_t chipBytes_;
  size_t freeBytes_;
};
