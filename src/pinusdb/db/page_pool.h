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

#include <unordered_map>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <list>
#include <atomic>
#include "storage/page_hdr.h"
#include "util/ref_util.h"

#define PDB_PAGEPOOL_DATAFILE_MASK    (0xFFFFFFFF00000000)
#define PDB_PAGEPOOL_TABLE_MASK       (0xFF00000000000000)

class PagePool
{
public:
  PagePool();
  ~PagePool();

  bool InitPool();
  size_t GetPoolPageCnt() { return totalCacheSize_ / NORMAL_PAGE_SIZE; }
  PdbErr_t GetPage(uint64_t pageCode, PageRef* pPageRef);

  void ClearPageForMask(uint64_t pageCode, uint64_t maskCode);
  void NotifyAll() { poolVariable_.notify_all(); }

private:
  PageHdr* _GetFreePage();
  void _PutFreePage(PageHdr* pPage);
  PdbErr_t _AllocCache();

private:
  enum {
    kAllocSize = PDB_MB_BYTES(128),  //一次内存申请大小 128M
    kErrorCoolingTime = (60 * 1000), // 内存申请失败后的冷却时间
  };

private:
  std::mutex poolMutex_;
  std::condition_variable poolVariable_;

  struct list_head readList_;
  struct list_head freeList_;

  size_t totalCacheSize_;  //总的缓存大小
  size_t useCacheSize_;    //已使用的缓存大小 

  uint64_t lastAllocErrorTime_; //上次申请失败的时间

  std::vector<uint8_t*> hdrVec_;
  std::vector<void*> cacheMemVec_;
  std::unordered_map<uint64_t, PageHdr*> pageMap_;
};

