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

#include "db/page_pool.h"

#include <thread>
#include <chrono>
#include <list>

#include "util/ker_list.h"
#include "util/ref_util.h"
#include "util/log_util.h"
#include "util/date_time.h"
#include "global_variable.h"

#ifndef _WIN32
#include <sys/sysinfo.h>
#endif

PagePool::PagePool()
{
  INIT_LIST_HEAD(&readList_);
  INIT_LIST_HEAD(&freeList_);

  totalCacheSize_ = 0;
  useCacheSize_ = 0;

  lastAllocErrorTime_ = 0;
}

PagePool::~PagePool()
{
  for (auto hdrIter = hdrVec_.begin(); hdrIter != hdrVec_.end(); hdrIter++)
  {
#ifdef _WIN32
    delete [](*hdrIter);
#else
    free(*hdrIter);
#endif
  }

  for (auto memIter = cacheMemVec_.begin(); memIter != cacheMemVec_.end(); memIter++)
  {
#ifdef _WIN32
    VirtualFreeEx(GetCurrentProcess(), *memIter, 0, MEM_RELEASE);
#else
    free(*memIter);
#endif
  }
}

bool PagePool::InitPool()
{
  size_t totalPhyMem = 0;

#ifdef _WIN32
  MEMORYSTATUSEX memStatus;
  memStatus.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&memStatus);
  totalPhyMem = memStatus.ullTotalPhys;
#else
  struct sysinfo sInfo;
  sysinfo(&sInfo);
  totalPhyMem = sInfo.totalram;
#endif

  size_t cacheSize = PDB_MB_BYTES(pGlbSysCfg->GetCacheSize()); //从配置文件中读取出来的单位是M
  size_t maxCacheSize = ((totalPhyMem / 8) * 5); //最大为物理内存的5/8
  size_t minCacheSize = PDB_GB_BYTES(1); //最小1GB
  if (totalPhyMem < PDB_GB_BYTES(8))
    minCacheSize = totalPhyMem / 8; // 如果物理内存小于8G，最小缓存为内存的1/8

  if (cacheSize == 0)
  {
    if (totalPhyMem < PDB_GB_BYTES(4))
      cacheSize = (totalPhyMem / 8); // 内存小于4G时，推荐为物理内存的1/8
    else if (totalPhyMem < PDB_GB_BYTES(8))
      cacheSize = (totalPhyMem / 4); // 内存小于8G时，推荐为物理内存的1/4
    else
      cacheSize = (totalPhyMem / 2); // 使用推荐值, 物理内存的1/2
  }
  else if (cacheSize < minCacheSize)
    cacheSize = minCacheSize; // 使用最小缓存
  else if (cacheSize > maxCacheSize)
    cacheSize = maxCacheSize; // 使用最大缓存

  //以128M，向上取整
  if (cacheSize < kAllocSize)
    cacheSize = kAllocSize;

  cacheSize = ((cacheSize + (kAllocSize - 1))) & (~(kAllocSize - 1));
  totalCacheSize_ = cacheSize;
  LOG_INFO("set database cache ({}M)", (cacheSize / PDB_MB_BYTES(1)));
  pGlbSysCfg->UpdateCacheSize(static_cast<int32_t>(cacheSize / PDB_MB_BYTES(1)));

  if (_AllocCache() != PdbE_OK)
  {
    LOG_ERROR("failed to allocate memory for database page cache");
    return false;
  }

  return true;
}

PdbErr_t PagePool::GetPage(uint64_t pageCode, PageRef* pPageRef)
{
  PageHdr* pTmpPage = nullptr;
  
  std::unique_lock<std::mutex> poolLock(poolMutex_);
  while (glbRunning)
  {
    auto pageIter = pageMap_.find(pageCode);
    if (pageIter != pageMap_.end())
    {
      pTmpPage = pageIter->second;
      pPageRef->Attach(pTmpPage);
      list_move(&(pTmpPage->listHdr_), &readList_);
      return PdbE_OK;
    }

    pTmpPage = _GetFreePage();
    if (pTmpPage != nullptr)
    {
      PAGEHDR_CLEAR(pTmpPage);
      PAGEHDR_SET_PAGECODE(pTmpPage, pageCode);
      list_add(&(pTmpPage->listHdr_), &readList_);

      pPageRef->Attach(pTmpPage);
      pageMap_.insert(std::pair<uint64_t, PageHdr*>(pageCode, pTmpPage));
      return PdbE_OK;
    }

    poolVariable_.wait(poolLock);
  }

  //LOG_INFO("failed to get free page cache");
  return PdbE_TASK_CANCEL;
}

void PagePool::ClearPageForMask(uint64_t pageCode, uint64_t maskCode)
{
  uint64_t tmpCode = 0;

  std::unique_lock<std::mutex> poolLock(poolMutex_);
  for (auto pageIt = pageMap_.begin(); pageIt != pageMap_.end(); )
  {
    tmpCode = pageIt->first;
    pageIt++;
    if ((tmpCode & maskCode) == pageCode)
    {
      pageMap_.erase(tmpCode);
    }
  }

  poolVariable_.notify_all();
}

PageHdr* PagePool::_GetFreePage()
{
  PageHdr* pTmp = nullptr;

  if (list_empty(&freeList_) && useCacheSize_ < totalCacheSize_)
  {
    uint64_t nowTick = DateTime::NowTickCount();
    if (lastAllocErrorTime_ < (nowTick - kErrorCoolingTime))
    {
      if (_AllocCache() != PdbE_OK)
        lastAllocErrorTime_ = nowTick;
    }
  }

  if (!list_empty(&freeList_))
  {
    list_head* pTmpFree = (freeList_.next);
    list_del(pTmpFree);
    pTmp = list_entry(pTmpFree, PageHdr, listHdr_);
  }
  else
  {
    //从读链表中找一个页
    struct list_head* pIter = readList_.prev;
    while (pIter != &readList_)
    {
      PageHdr* pPage = list_entry(pIter, PageHdr, listHdr_);
      if (!PAGEHDR_IS_DIRTY(pPage) && PAGEHDR_GET_REF(pPage) == 0)
      {
        uint64_t pageCode = pPage->pageCode_; 
        pageMap_.erase(pageCode);
        list_del(pIter);
        PAGEHDR_CLEAR(pPage);
        pTmp = pPage;
        break;
      }

      pIter = pIter->prev;
    }
  }

  return pTmp;
}

void PagePool::_PutFreePage(PageHdr* pPage)
{
  list_add(&(pPage->listHdr_), &readList_);
}

#ifdef _WIN32
PdbErr_t PagePool::_AllocCache()
{
  DWORD lastError = NO_ERROR;
  DWORD allocType = MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN;

  MEMORYSTATUSEX memStatus;
  memStatus.dwLength = sizeof(MEMORYSTATUSEX);
  if (GlobalMemoryStatusEx(&memStatus) == FALSE)
  {
    lastError = GetLastError();
    LOG_ERROR("failed to get system memory info, err: {}", lastError);
    return lastError;
  }

  //判断是否有足够的空闲内存
  if (memStatus.ullTotalPhys > PDB_GB_BYTES(8))
  {
    //剩余物理内存必须大于物理内存的1/8
    if (memStatus.ullAvailPhys < (memStatus.ullTotalPhys / 8))
      return PdbE_NOMEM;
  }
  else
  {
    //剩余物理内存必须大于512M
    if (memStatus.ullAvailPhys < PDB_MB_BYTES(512))
      return PdbE_NOMEM;
  }

  PVOID pTmpMem = VirtualAllocEx(GetCurrentProcess(), NULL, kAllocSize, allocType, PAGE_READWRITE);
  if (pTmpMem == NULL)
  {
    lastError = GetLastError();
    LOG_ERROR("failed to VirtualAllocEx, err: {}", lastError);
    return lastError;
  }

  size_t pageCnt = kAllocSize / NORMAL_PAGE_SIZE;
  uint8_t* pHdrBuf = new (std::nothrow) uint8_t[(sizeof(PageHdr) * pageCnt)];
  if (pHdrBuf == nullptr)
  {
    VirtualFreeEx(GetCurrentThread(), pTmpMem, 0, MEM_RELEASE);
    LOG_ERROR("failed to allocate memory");
    return PdbE_NOMEM;
  }

  PageHdr* pHdrItem = (PageHdr*)pHdrBuf;
  uint8_t* pDataItem = (uint8_t*)pTmpMem;
  for (size_t i = 0; i < pageCnt; i++)
  {
    PAGEHDR_INIT(pHdrItem, pDataItem);
    list_add(&(pHdrItem->listHdr_), &freeList_);
    pHdrItem++;
    pDataItem += NORMAL_PAGE_SIZE;
  }

  cacheMemVec_.push_back(pTmpMem);
  hdrVec_.push_back(pHdrBuf);

  useCacheSize_ += kAllocSize;

  return PdbE_OK;
}

#else

PdbErr_t PagePool::_AllocCache()
{
  void* pTmpMem = malloc((kAllocSize + NORMAL_PAGE_SIZE));
  if (pTmpMem == nullptr)
  {
    LOG_ERROR("pagepool allocate cache failed");
    return PdbE_NOMEM;
  }

  size_t pageCnt = kAllocSize / NORMAL_PAGE_SIZE;
  uint8_t* pHdrBuf = (uint8_t*)malloc(sizeof(PageHdr) * pageCnt);
  if (pHdrBuf == nullptr)
  {
    free(pTmpMem);
    LOG_ERROR("failed to allocate memory");
    return PdbE_NOMEM;
  }

  PageHdr* pHdrItem = (PageHdr*)pHdrBuf;
  uint8_t* pDataItem = (uint8_t*)pTmpMem;
  size_t dataMod = reinterpret_cast<uintptr_t>(pTmpMem) & (NORMAL_PAGE_SIZE - 1);
  size_t slop = (dataMod == 0 ? 0 : NORMAL_PAGE_SIZE - dataMod);
  pDataItem += slop;

  for (size_t i = 0; i < pageCnt; i++)
  {
    PAGEHDR_INIT(pHdrItem, pDataItem);
    list_add(&(pHdrItem->listHdr_), &freeList_);
    pHdrItem++;
    pDataItem += NORMAL_PAGE_SIZE;
  }

  cacheMemVec_.push_back(pTmpMem);
  hdrVec_.push_back(pHdrBuf);

  useCacheSize_ += kAllocSize;
  return PdbE_OK;
}

#endif
