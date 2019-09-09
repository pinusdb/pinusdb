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

typedef struct _NormalDataHead
{
  uint32_t pageCrc_;      //页的校验码
  int32_t  pageNo_;       //页号
  int64_t  devId_;        //设备ID
  int64_t  idxTs_;        //索引时间戳
  uint16_t recCnt_;       //当前存储的记录条数
  uint16_t chipBytes_;    //回收字节数
  uint16_t freeBytes_;    //空闲字节数
  uint16_t padding_;      //
}NormalDataHead;

#define UPDATE_NORMAL_DATA_PAGE_CRC(pDataHead) do \
{ \
  pDataHead->pageCrc_ = StringTool::CRC32(pDataHead, (NORMAL_PAGE_SIZE - sizeof(uint32_t)), sizeof(uint32_t)); \
}while(false)

class NormalDataPage
{
public:
  NormalDataPage();
  ~NormalDataPage();

  PdbErr_t Load(PageHdr* pPage);
  void Init(PageHdr* pPage, int32_t pageNo, int64_t devId, int64_t idxTs);

  int32_t GetPageNo() const;
  int64_t GetDevId() const;
  uint16_t GetRecCnt() const;
  uint16_t GetChipBytes() const;
  uint16_t GetFreeBytes() const;
  int64_t GetIdxTs() const;

  void UpdateCrc();

  PdbErr_t InsertRec(int64_t ts, const PdbByte* pRec, size_t recLen, bool replace);
  PdbErr_t GetRecData(size_t idx, const PdbByte** ppRec);
  PdbErr_t GetRecTstamp(size_t idx, int64_t* pTstamp);
  PdbErr_t GetLastTstamp(int64_t* pTstamp);

  //分裂最多指定的字节
  PdbErr_t SplitMost(NormalDataPage* pNewPage, int32_t bytes);

private:
  PdbErr_t UpdateIdxTs();
  PdbErr_t CleanUp();
  PdbErr_t InsertForPos(int idx, const uint8_t* pRec, size_t recLen);

private:
  PageHdr* pPage_;
};
