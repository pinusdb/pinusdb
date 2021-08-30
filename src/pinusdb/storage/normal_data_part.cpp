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

#include "storage/normal_data_part.h"
#include "storage/normal_data_page.h"
#include "db/page_pool.h"
#include "util/coding.h"
#include "global_variable.h"
#include "util/log_util.h"
#include "util/date_time.h"
#include <string.h>

#define NORMAL_DATA_GROW_SIZE  (128 * 1024 * 1024)
#define NORMAL_PAGE_OFFSET(pageNo)  (((int64_t)pageNo) * NORMAL_PAGE_SIZE)

bool SplitPageComp(const PageHdr* pA, const PageHdr* pB)
{
  return PAGEHDR_GET_SPLIT_GRP(pA) < PAGEHDR_GET_SPLIT_GRP(pB);
}

bool DataPageComp(const PageHdr* pA, const PageHdr* pB)
{
  return PAGEHDR_GET_PAGENO(pA) < PAGEHDR_GET_PAGENO(pB);
}

NormalDataPart::NormalDataPart()
{
  partMetaCode_ = 0;
  readOnly_ = false;
  pageCodeMask_ = 0;
  nextPageNo_ = 1;
}

NormalDataPart::~NormalDataPart()
{
  dataFile_.Close();
  normalIdx_.Close();
}

PdbErr_t NormalDataPart::Create(const char* pIdxPath, const char* pDataPath,
  const TableInfo* pTabInfo, uint32_t partCode)
{
  PdbErr_t retVal = PdbE_OK;
  OSFile dataFile;
  Arena arena;
  char* pTmpMeta = arena.Allocate(sizeof(DataFileMeta));
  DataFileMeta* pMeta = (DataFileMeta*)pTmpMeta;
  size_t fieldCnt = pTabInfo->GetFieldCnt();

  memset(pTmpMeta, 0, sizeof(DataFileMeta));
  strncpy(pMeta->headStr_, NORMAL_DATA_FILE_HEAD_STR, sizeof(pMeta->headStr_));
  Coding::FixedEncode32(pMeta->pageSize_, NORMAL_PAGE_SIZE);
  Coding::FixedEncode32(pMeta->fieldCnt_, static_cast<uint32_t>(fieldCnt));
  Coding::FixedEncode32(pMeta->partCode_, partCode);
  Coding::FixedEncode32(pMeta->tableType_, PDB_PART_TYPE_NORMAL_VAL);

  int32_t fieldType = 0;
  const char* pFieldName = nullptr;
  for (size_t i = 0; i < fieldCnt; i++)
  {
    pTabInfo->GetFieldRealInfo(i, &fieldType, nullptr);
    pFieldName = pTabInfo->GetFieldName(i);

    strncpy(pMeta->fieldRec_[i].fieldName_, pFieldName, PDB_FILED_NAME_LEN);
    Coding::FixedEncode32(pMeta->fieldRec_[i].fieldType_, fieldType);
  }

  uint32_t crc = StringTool::CRC32(pTmpMeta, (sizeof(DataFileMeta) - 4));
  Coding::FixedEncode32(pMeta->crc_, crc);

  ////////////////////////////////////////////////////////////////////////////
  //创建数据表，写入数据
  retVal = dataFile.OpenNew(pDataPath);
  if (retVal != PdbE_OK)
    return retVal;

  do {
    retVal = dataFile.GrowTo(NORMAL_DATA_GROW_SIZE);
    if (retVal != PdbE_OK)
      break;

    retVal = dataFile.Write(pTmpMeta, sizeof(DataFileMeta), 0);
    if (retVal != PdbE_OK)
      break;

    retVal = NormalPartIdx::Create(pIdxPath, partCode);
    if (retVal != PdbE_OK)
      break;

  } while (false);

  dataFile.Close();
  if (retVal != PdbE_OK)
  {
    FileTool::RemoveFile(pDataPath);
  }

  return retVal;
}

PdbErr_t NormalDataPart::Open(uint8_t tabCode, uint32_t partCode,
  const char* pIdxPath, const char* pDataPath)
{
  PdbErr_t retVal = PdbE_OK;
  Arena arena;
  idxPath_ = pIdxPath;
  dataPath_ = pDataPath;
  readOnly_ = false;

  char* pTmpMeta = arena.AllocateAligned(sizeof(DataFileMeta), PDB_KB_BYTES(8));
  if (pTmpMeta == nullptr)
    return PdbE_NOMEM;

  retVal = dataFile_.OpenNoBuf(pDataPath, false);
  if (retVal != PdbE_OK)
    return retVal;

  retVal = dataFile_.Read(pTmpMeta, sizeof(DataFileMeta), 0);
  if (retVal != PdbE_OK)
    return retVal;

  DataFileMeta* pDataMeta = (DataFileMeta*)pTmpMeta;
  uint32_t crc = Coding::FixedDecode32(pDataMeta->crc_);
  if (StringTool::CRC32(pTmpMeta, (sizeof(DataFileMeta) - 4)) != crc)
  {
    LOG_ERROR("failed to open data file ({}), file meta crc error", pDataPath);
    return PdbE_PAGE_ERROR;
  }

  size_t fixedSize = 0;
  uint64_t metaCode64 = 0;
  uint32_t fieldCnt = Coding::FixedDecode32(pDataMeta->fieldCnt_);
  FieldInfo finfo;
  for (uint32_t i = 0; i < fieldCnt; i++)
  {
    int32_t fieldType = Coding::FixedDecode32(pDataMeta->fieldRec_[i].fieldType_);
    retVal = finfo.SetFieldInfo(pDataMeta->fieldRec_[i].fieldName_, fieldType, false);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to open data file ({}), invalid field info", pDataPath);
      return retVal;
    }

    fieldInfoVec_.push_back(finfo);
    if (i == PDB_DEVID_INDEX)
    {
      fieldPosVec_.push_back(0);
    }
    else if (i == PDB_TSTAMP_INDEX)
    {
      fieldPosVec_.push_back(2);
      fixedSize = 10;
    }
    else
    {
      fieldPosVec_.push_back(fixedSize);
      switch (fieldType)
      {
      case PDB_FIELD_TYPE::TYPE_BOOL: fixedSize += 1; break;
      case PDB_FIELD_TYPE::TYPE_INT8: fixedSize += 1; break;
      case PDB_FIELD_TYPE::TYPE_INT16: fixedSize += 2; break;
      case PDB_FIELD_TYPE::TYPE_INT32: fixedSize += 4; break;
      case PDB_FIELD_TYPE::TYPE_INT64: fixedSize += 8; break;
      case PDB_FIELD_TYPE::TYPE_DATETIME: fixedSize += 8; break;
      case PDB_FIELD_TYPE::TYPE_FLOAT: fixedSize += 4; break;
      case PDB_FIELD_TYPE::TYPE_DOUBLE: fixedSize += 8; break;
      case PDB_FIELD_TYPE::TYPE_STRING: fixedSize += 2; break;
      case PDB_FIELD_TYPE::TYPE_BLOB: fixedSize += 2; break;
      case PDB_FIELD_TYPE::TYPE_REAL2: fixedSize += 8; break;
      case PDB_FIELD_TYPE::TYPE_REAL3: fixedSize += 8; break;
      case PDB_FIELD_TYPE::TYPE_REAL4: fixedSize += 8; break;
      case PDB_FIELD_TYPE::TYPE_REAL6: fixedSize += 8; break;
      }
    }

    metaCode64 = StringTool::CRC64NoCase(pDataMeta->fieldRec_[i].fieldName_,
      strlen(pDataMeta->fieldRec_[i].fieldName_), 0, metaCode64);
    metaCode64 = StringTool::CRC64(&(pDataMeta->fieldRec_[i].fieldType_), sizeof(int), 0, metaCode64);
  }
  partMetaCode_ = CRC64_TO_CRC32(metaCode64);

  retVal = normalIdx_.Open(pIdxPath, false);
  if (retVal != PdbE_OK)
    return retVal;

  uint32_t idxPartCode = normalIdx_.GetPartCode();
  if (idxPartCode != partCode)
  {
    LOG_ERROR("failed to open data file ({}), date code error", pDataPath);
    return PdbE_PAGE_ERROR;
  }

  if (idxPartCode != Coding::FixedDecode32(pDataMeta->partCode_))
  {
    LOG_ERROR("failed to open data file, mismatch between index file ({}:{}) and data file ({}:{}) ",
      pIdxPath, idxPartCode, pDataPath, pDataMeta->partCode_);
    return PdbE_PAGE_ERROR;
  }

  bgDayTs_ = (int64_t)idxPartCode * DateTime::MicrosecondPerDay;
  edDayTs_ = bgDayTs_ + DateTime::MicrosecondPerDay;
  pageCodeMask_ = MAKE_DATAPART_MASK(tabCode, idxPartCode);
 
  nextPageNo_ = normalIdx_.GetMaxPageNo() + 1;
  return PdbE_OK;
}

void NormalDataPart::Close()
{
  normalIdx_.Close();
  dataFile_.Close();
}

PdbErr_t NormalDataPart::RecoverDW(const char* pPageBuf)
{
  PdbErr_t retVal = PdbE_OK;
  MemPageIdx pageIdx;
  std::string idxBuf;
  const char* pPageData = nullptr;
  NormalDataHead* pPageHead = nullptr;

  for (size_t i = 0; i < SYNC_PAGE_CNT; i++)
  {
    pPageData = pPageBuf + (NORMAL_PAGE_SIZE * i);
    pPageHead = (NormalDataHead*)pPageData;

    if (NormalDataHead_GetPageCrc(pPageHead) == 0)
      break;

    retVal = dataFile_.Write(pPageData, NORMAL_PAGE_SIZE, NormalDataHead_GetPageOffset(pPageHead));
    if (retVal != PdbE_OK)
      return retVal;

    int64_t devId = NormalDataHead_GetDevId(pPageHead);
    int64_t idxTs = NormalDataHead_GetIdxTs(pPageHead);
    int32_t pageNo = NormalDataHead_GetPageNo(pPageHead);
    retVal = normalIdx_.GetIndex(devId, idxTs, &pageIdx);
    if (retVal != PdbE_OK || pageIdx.idxTs_ != idxTs)
    {
      normalIdx_.AddIdx(devId, idxTs, pageNo);
      normalIdx_.AppendIdx(idxBuf, devId, idxTs, pageNo);

      //更新下个分配的页号
      if (pageIdx.pageNo_ >= nextPageNo_)
        nextPageNo_ = pageIdx.pageNo_ + 1;
    }
  }

  //写入索引信息
  retVal = normalIdx_.WriteIdx(idxBuf);
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  return PdbE_OK;
}

PdbErr_t NormalDataPart::InsertRec(uint32_t metaCode, int64_t devId, int64_t tstamp,
  bool replace, const char* pRec, size_t recLen)
{
  PdbErr_t retVal = PdbE_OK;
  if (readOnly_)
    return PdbE_FILE_READONLY;

  if (metaCode != partMetaCode_)
    return PdbE_TABLE_FIELD_MISMATCH;

  std::mutex* pDevMutex = pGlbMutexManager->GetDevMutex(devId);
  MemPageIdx pageIdx;
  NormalDataPage dataPage;
  PageRef pageRef;
  PageRef newPageRef;
  PageHdr* pPage = nullptr;
  int32_t newPageNo = 0;

  std::unique_lock<std::mutex> devLock(*pDevMutex);
  retVal = normalIdx_.GetIndex(devId, tstamp, &pageIdx);
  if (retVal == PdbE_DEV_NOT_FOUND)
  {
    retVal = AllocPage(&pageRef);
    if (retVal != PdbE_OK)
    {
      LOG_WARNING("failed to AllocPage for Insert, ret: {}", retVal);
      return retVal;
    }

    pPage = pageRef.GetPageHdr();
    newPageNo = PAGEHDR_GET_PAGENO(pPage);
    dataPage.Init(pPage, newPageNo, devId, bgDayTs_);

    retVal = dataPage.InsertRec(tstamp, pRec, recLen, true);
    if (retVal != PdbE_OK)
    {
      LOG_WARNING("failed to InsertRec, err: {}", retVal);
      return retVal;
    }

    PAGEHDR_SET_ONLY_MEM(pPage);

    do {
      std::unique_lock<std::mutex> dirtyLock(dirtyMutex_);
      dirtyList_.push_back(pPage);
    } while (false);

    retVal = normalIdx_.AddIdx(devId, bgDayTs_, newPageNo);
    if (retVal != PdbE_OK)
    {
      LOG_WARNING("failed to add index (dataPath:{}, devId:{}, tstamp:{}, pageNo{}), err:{}",
        dataPath_.c_str(), devId, bgDayTs_, newPageNo, retVal);
      return retVal;
    }
  }
  else if (retVal == PdbE_OK)
  {
    std::mutex* pPageMutex = pGlbMutexManager->GetPageMutex(pageIdx.pageNo_);
    std::unique_lock<std::mutex> pageLock(*pPageMutex);
    retVal = GetPage(pageIdx.pageNo_, &pageRef);
    if (retVal != PdbE_OK)
    {
      LOG_WARNING("failed to insert record, get file ({}) data page (devId:{}, pageNo:{}) data error {}",
        dataPath_.c_str(), devId, pageIdx.pageNo_, retVal);
      return retVal;
    }

    pPage = pageRef.GetPageHdr();
    bool oldPageDirty = PAGEHDR_IS_DIRTY(pPage);
    retVal = dataPage.Load(pPage);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to insert record, file ({}) data page (devId:{}, pageNo:{}) error, {}",
        dataPath_.c_str(), devId, pageIdx.pageNo_, retVal);
      return retVal;
    }

    retVal = dataPage.InsertRec(tstamp, pRec, recLen, true);
    if (retVal == PdbE_OK)
    {
      if (!oldPageDirty)
      {
        std::unique_lock<std::mutex> dirtyLock(dirtyMutex_);
        dirtyList_.push_back(pPage);
      }
      return PdbE_OK;
    }

    if (retVal != PdbE_PAGE_FILL)
    {
      LOG_ERROR("failed to insert record, err:{}", retVal);
      return retVal;
    }
    
    //分裂旧页或直接插入新页
    int64_t lastTs = 0;
    retVal = dataPage.GetLastTstamp(&lastTs);
    if (retVal != PdbE_OK)
      return retVal;

    PageHdr* pNewPage = nullptr;
    NormalDataPage newDataPage;
    retVal = AllocPage(&newPageRef);
    if (retVal != PdbE_OK)
      return retVal;
    
    pNewPage = newPageRef.GetPageHdr();
    newPageNo = static_cast<int32_t>(pNewPage->pageCode_ & 0xFFFFFFFF);
    newDataPage.Init(pNewPage, newPageNo, devId, tstamp);
    if (tstamp > lastTs)
    {
      //新的数据页
      retVal = newDataPage.InsertRec(tstamp, pRec, recLen, true);
      if (retVal != PdbE_OK)
        return retVal;

      PAGEHDR_SET_ONLY_MEM(pNewPage);

      do {
        std::unique_lock<std::mutex> dirtyLock(dirtyMutex_);
        dirtyList_.push_back(pNewPage);
      } while (false);
      
      retVal = normalIdx_.AddIdx(devId, tstamp, newPageNo);
      if (retVal != PdbE_OK)
        return retVal;

      //如果旧页未同步,设置旧页为页满
      if (oldPageDirty)
      {
        PAGEHDR_SET_IS_FULL(pPage);
      }
    }
    else
    {
      //分裂原来的数据页
      if (normalIdx_.GetNextIndex(devId, tstamp, &pageIdx) != PdbE_OK)
      {
        //如果是最后一个页，将少部分分裂到新页
        retVal = dataPage.SplitMost(&newDataPage, (24 * 1024));
      }
      else
      {
        //中间的页，分裂一半到新页
        retVal = dataPage.SplitMost(&newDataPage, static_cast<int32_t>(NORMAL_PAGE_SIZE / 2));
      }

      if (retVal != PdbE_OK)
        return retVal;

      if (tstamp > newDataPage.GetIdxTs())
      {
        //插入到新的页
        retVal = newDataPage.InsertRec(tstamp, pRec, recLen, true);
      }
      else
      {
        //插入到旧的页
        retVal = dataPage.InsertRec(tstamp, pRec, recLen, true);
      }

      if (!PAGEHDR_ONLY_MEM(pPage))
      {
        //设置页是已分裂过的, 仅在内存中的数据设置分裂无意义
        PAGEHDR_SET_SPLIT_GRP(pPage, PAGEHDR_GET_PAGENO(pPage));
        PAGEHDR_SET_SPLIT_GRP(pNewPage, PAGEHDR_GET_PAGENO(pPage));
      }
      
      //设置分裂后的页面，尽量快的刷盘
      PAGEHDR_SET_IS_FULL(pPage);
      PAGEHDR_SET_IS_FULL(pNewPage);

      normalIdx_.AddIdx(devId, newDataPage.GetIdxTs(), newPageNo);
      
      do {
        std::unique_lock<std::mutex> dirtyLock(dirtyMutex_);
        dirtyList_.push_back(pNewPage);
        if (!oldPageDirty)
          dirtyList_.push_back(pPage);
      } while (false);
    }
  }

  return retVal;
}

PdbErr_t NormalDataPart::DumpToCompPart(const char* pDataPath)
{
  PdbErr_t retVal = PdbE_OK;
  std::vector<int64_t> allDevIdVec;
  normalIdx_.GetAllDevId(allDevIdVec);

  CompPartBuilder partBuilder;
  retVal = partBuilder.Create(normalIdx_.GetPartCode(), pDataPath, fieldInfoVec_);
  if (retVal != PdbE_OK)
    return retVal;

  for (auto devIt = allDevIdVec.begin(); devIt != allDevIdVec.end(); devIt++)
  {
    retVal = DumpToCompPartId(*devIt, &partBuilder);
    if (retVal != PdbE_OK)
      break;
  }

  if (retVal != PdbE_OK)
  {
    partBuilder.Abandon();
    return retVal;
  }

  retVal = partBuilder.Finish();
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  return PdbE_OK;
}

bool NormalDataPart::SwitchToReadOnly()
{
  readOnly_ = true;
  return true;
}

PdbErr_t NormalDataPart::SyncDirtyPages(bool syncAll, OSFile* pDwFile)
{
  PdbErr_t splitRet = PdbE_OK;
  PdbErr_t otherRet = PdbE_OK;
  std::vector<PageHdr*> tmpHdrVec;

  if (syncAll)
  {
    std::unique_lock<std::mutex> dirtyLock(dirtyMutex_);
    for (auto pageIter = dirtyList_.begin(); pageIter != dirtyList_.end(); pageIter++)
    {
      PAGEHDR_ROUND_ROBIN(*pageIter);
    }
  }
  else
  {
    std::unique_lock<std::mutex> dirtyLock(dirtyMutex_);
    for (auto pageIter = dirtyList_.begin(); pageIter != dirtyList_.end(); pageIter++)
    {
      if (PAGEHDR_GET_IS_FULL(*pageIter) || PAGEHDR_GET_SPLIT_GRP(*pageIter) != 0)
      {
        PAGEHDR_ROUND_ROBIN(*pageIter);
      }
    }
  }

  //写入分裂的页
  do {
    tmpHdrVec.clear();

    do {
      std::unique_lock<std::mutex> dirtyLock(dirtyMutex_);
      for (auto pageIter = dirtyList_.begin(); pageIter != dirtyList_.end();)
      {
        if (PAGEHDR_NEED_SYNC(*pageIter) && (PAGEHDR_GET_SPLIT_GRP(*pageIter) != 0))
        {
          tmpHdrVec.push_back(*pageIter);
          pageIter = dirtyList_.erase(pageIter);
        }
        else
        {
          pageIter++;
        }
      }
    } while (false);

    if (tmpHdrVec.empty())
      break;

    std::sort(tmpHdrVec.begin(), tmpHdrVec.end(), SplitPageComp);
    splitRet = WritePages(tmpHdrVec, pDwFile);

  } while (false);

  //写入其他页
  do {
    tmpHdrVec.clear();

    do {
      std::unique_lock<std::mutex> dirtyLock(dirtyMutex_);
      for (auto pageIter = dirtyList_.begin(); pageIter != dirtyList_.end();)
      {
        if (PAGEHDR_NEED_SYNC(*pageIter))
        {
          tmpHdrVec.push_back(*pageIter);
          pageIter = dirtyList_.erase(pageIter);
        }
        else
        {
          pageIter++;
        }
      }
    } while (false);

    if (tmpHdrVec.empty())
      break;

    std::sort(tmpHdrVec.begin(), tmpHdrVec.end(), DataPageComp);
    otherRet = WritePages(tmpHdrVec, pDwFile);
    
  } while (false);

  if (splitRet != PdbE_OK)
    return splitRet;
  if (otherRet != PdbE_OK)
    return otherRet;

  return PdbE_OK;
}

PdbErr_t NormalDataPart::AbandonDirtyPages()
{
  //AbandonDirtyPages调用时，不会有程序访问，
  //所以不用加 dirtyMutex_ 锁，也防止dirtyMutex_ 与 页锁交叉
  std::mutex* pPageMutex = nullptr;
  for (auto pageIter = dirtyList_.begin(); pageIter != dirtyList_.end(); pageIter++)
  {
    pPageMutex = pGlbMutexManager->GetPageMutex(PAGEHDR_GET_PAGENO(*pageIter));
    std::unique_lock<std::mutex> pageLock(*pPageMutex);
    PAGEHDR_ROUND_ROBIN(*pageIter);
    PAGEHDR_SYNC_DATA(*pageIter);
  }

  dirtyList_.clear();
  return PdbE_OK;
}

PdbErr_t NormalDataPart::QueryDevAsc(int64_t devId, const DataPartQueryParam& queryParam,
  IQuery* pQuery, uint64_t timeOut, bool queryFirst, bool* pIsAdd)
{
  return QueryDevData<true, false>(devId, queryParam, pQuery, timeOut, queryFirst, pIsAdd);
}

PdbErr_t NormalDataPart::QueryDevDesc(int64_t devId, const DataPartQueryParam& queryParam,
  IQuery* pQuery, uint64_t timeOut, bool queryLast, bool* pIsAdd)
{
  return QueryDevData<false, false>(devId, queryParam, pQuery, timeOut, queryLast, pIsAdd);
}

PdbErr_t NormalDataPart::QueryDevSnapshot(int64_t devId, const DataPartQueryParam& queryParam,
  IQuery* pQuery, uint64_t timeOut, bool* pIsAdd)
{
  return QueryDevData<false, true>(devId, queryParam, pQuery, timeOut, true, pIsAdd);
}

template<bool IsAsc, bool IsSnapshot>
PdbErr_t NormalDataPart::QueryDevData(int64_t devId, const DataPartQueryParam& queryParam,
  IQuery* pQuery, uint64_t timeOut, bool querySingle, bool* pIsAdd)
{
  PdbErr_t retVal = PdbE_OK;
  MemPageIdx pageIdx;
  int64_t bgTs = queryParam.GetBgTs();
  int64_t edTs = queryParam.GetEdTs();
  int64_t lastTs = 0;
  PageRef pageRef;
  std::mutex* pDevMutex = pGlbMutexManager->GetDevMutex(devId);
  std::mutex* pPageMutex = nullptr;
  bool isAdd = false;

  if PDB_CONSTEXPR(IsSnapshot)
  {
    bgTs = DateTime::MinMicrosecond;
    edTs = DateTime::MaxMicrosecond;
  }
  
  if PDB_CONSTEXPR(IsAsc)
    retVal = normalIdx_.GetIndex(devId, bgTs, &pageIdx);
  else
    retVal = normalIdx_.GetIndex(devId, edTs, &pageIdx);

  if (retVal == PdbE_DEV_NOT_FOUND || retVal == PdbE_IDX_NOT_FOUND)
  {
    if (pIsAdd != nullptr)
      *pIsAdd = false;

    return PdbE_OK;
  }
  
  if (retVal != PdbE_OK)
    return retVal;
  
  while (true)
  {
    if PDB_CONSTEXPR(IsAsc)
    {
      if (pageIdx.idxTs_ > edTs)
        return PdbE_OK;
    }

    pPageMutex = pGlbMutexManager->GetPageMutex(pageIdx.pageNo_);
  
    do {
      //将数据读到内存
      std::unique_lock<std::mutex> pageLock(*pPageMutex);
      retVal = GetPage(pageIdx.pageNo_, &pageRef);
      if (retVal != PdbE_OK)
        return retVal;
    } while (false);
  
    std::unique_lock<std::mutex> devLock(*pDevMutex);
    std::unique_lock<std::mutex> pageLock(*pPageMutex);
    retVal = GetPage(pageIdx.pageNo_, &pageRef);
    if (retVal != PdbE_OK)
      return retVal;

    PageHdr* pPageHdr = pageRef.GetPageHdr();
    NormalDataPage normalPage;
    retVal = normalPage.Load(pPageHdr);
    if (retVal != PdbE_OK)
      return retVal;
  
    if (querySingle)
    {
      retVal = TraversalDataPage<true, IsAsc, IsSnapshot>(&normalPage, devId, queryParam, pQuery, &isAdd);
      if (retVal == PdbE_OK && isAdd)
      {
        if (pIsAdd != nullptr)
          *pIsAdd = true;
  
        return PdbE_OK;
      }
    }
    else
    {
      retVal = TraversalDataPage<false, IsAsc, IsSnapshot>(&normalPage, devId, queryParam, pQuery, &isAdd);
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

    if PDB_CONSTEXPR(IsAsc)
    {
      retVal = normalPage.GetLastTstamp(&lastTs);
      if (retVal != PdbE_OK)
        return retVal;

      if (lastTs >= edTs)
        return PdbE_OK;

      retVal = normalIdx_.GetNextIndex(devId, lastTs, &pageIdx);
    }
    else
    {
      retVal = normalPage.GetRecTstamp(0, &lastTs);
      if (retVal != PdbE_OK)
        return retVal;

      if (lastTs <= bgTs)
        return PdbE_OK;

      retVal = normalIdx_.GetPrevIndex(devId, lastTs, &pageIdx);
    }

    if (retVal == PdbE_IDX_NOT_FOUND || retVal == PdbE_DEV_NOT_FOUND)
      return PdbE_OK;

    if (retVal != PdbE_OK)
      return retVal;

    if (DateTime::NowTickCount() > timeOut)
      return PdbE_QUERY_TIME_OUT;
  }
}


template<bool QuerySingle, bool IsAsc, bool IsSnapshot>
PdbErr_t NormalDataPart::TraversalDataPage(const NormalDataPage* pDataPage, int64_t devId,
  const DataPartQueryParam& queryParam, IQuery* pQuery, bool* pIsAdd)
{
  PdbErr_t retVal = PdbE_OK;
  bool isAdd = false;
  if (pDataPage == nullptr || pQuery == nullptr)
    return PdbE_INVALID_PARAM;

  size_t valCnt = queryParam.GetQueryFieldCnt();
  std::vector<DBVal> valVec;
  valVec.reserve(valCnt);
  DBVal* pVals = valVec.data();
  const char* pRec = nullptr;

  DBVAL_ELE_SET_INT64(pVals, PDB_DEVID_INDEX, devId);
  for (size_t idx = PDB_TSTAMP_INDEX; idx < valCnt; idx++)
    DBVAL_ELE_SET_NULL(pVals, idx);

  const std::vector<FieldQueryMapping>& fieldMapVec = queryParam.GetQueryFieldVec();
  const FieldQueryMapping* pFieldMaps = fieldMapVec.data();
  size_t fieldMapCnt = fieldMapVec.size();

  int32_t idx = 0;
  int32_t recCnt = pDataPage->GetRecCnt();
  int64_t tstamp = 0;
  int64_t bgTs = queryParam.GetBgTs();
  int64_t edTs = queryParam.GetEdTs();

  auto SeekToFunc = [&](int64_t seekTs) -> int32_t {
    int32_t lwr = 0;
    int32_t upr = recCnt - 1;
    int32_t tmpIdx = 0;
    int64_t tmpTs = 0;

    while (lwr <= upr)
    {
      tmpIdx = (lwr + upr) / 2;
      retVal = pDataPage->GetRecTstamp(tmpIdx, &tmpTs);
      if (retVal != PdbE_OK)
        return -1;

      if (seekTs == tmpTs)
        break;
      else if (seekTs < tmpTs)
        upr = tmpIdx - 1;
      else
        lwr = tmpIdx + 1;
    }
    
    return tmpIdx;
  };

  if PDB_CONSTEXPR(IsSnapshot)
  {
    idx = recCnt - 1;
  }
  else if PDB_CONSTEXPR(IsAsc)
  {
    idx = 0;
    retVal = pDataPage->GetRecTstamp(0, &tstamp);
    if (retVal != PdbE_OK)
      return retVal;

    if (tstamp > edTs)
    {
      idx = recCnt;
    }
    else if (tstamp < bgTs)
    {
      //seek
      idx = SeekToFunc(bgTs);
    }
  }
  else
  {
    idx = recCnt - 1;
    retVal = pDataPage->GetLastTstamp(&tstamp);
    if (retVal != PdbE_OK)
      return retVal;

    if (tstamp < bgTs)
    {
      idx = -1;
    }
    else if (tstamp > edTs)
    {
      //seek
      idx = SeekToFunc(edTs);
    }
  }

  while (idx >= 0 && idx < recCnt)
  {
    const char* pBlockBg = nullptr;
    retVal = pDataPage->GetRecData(idx, &pRec);
    if (retVal != PdbE_OK)
      return retVal;

    size_t blockPos = 0;
    int64_t realVal = 0;
    uint32_t blockLen = 0;
    uint32_t recLen = Coding::FixedDecode16(pRec);
    for (size_t fieldIdx = 0; fieldIdx < fieldMapCnt; fieldIdx++)
    {
      size_t queryFieldPos = pFieldMaps[fieldIdx].queryFieldPos_;
      size_t storeFieldPos = pFieldMaps[fieldIdx].storeFieldPos_;
      switch (pFieldMaps[fieldIdx].fieldType_)
      {
      case PDB_FIELD_TYPE::TYPE_BOOL:
        DBVAL_ELE_SET_BOOL(pVals, queryFieldPos, (pRec[storeFieldPos] == PDB_BOOL_TRUE));
        break;
      case PDB_FIELD_TYPE::TYPE_INT8:
        DBVAL_ELE_SET_INT8(pVals, queryFieldPos, static_cast<int8_t>(pRec[storeFieldPos]));
        break;
      case PDB_FIELD_TYPE::TYPE_INT16:
        DBVAL_ELE_SET_INT16(pVals, queryFieldPos, Coding::FixedDecode16(pRec + storeFieldPos));
        break;
      case PDB_FIELD_TYPE::TYPE_INT32:
        DBVAL_ELE_SET_INT32(pVals, queryFieldPos, Coding::FixedDecode32(pRec + storeFieldPos));
        break;
      case PDB_FIELD_TYPE::TYPE_INT64:
        DBVAL_ELE_SET_INT64(pVals, queryFieldPos, Coding::FixedDecode64(pRec + storeFieldPos));
        break;
      case PDB_FIELD_TYPE::TYPE_DATETIME:
        DBVAL_ELE_SET_DATETIME(pVals, queryFieldPos, Coding::FixedDecode64(pRec + storeFieldPos));
        break;
      case PDB_FIELD_TYPE::TYPE_FLOAT:
        DBVAL_ELE_SET_FLOAT(pVals, queryFieldPos, Coding::DecodeFloat(Coding::FixedDecode32(pRec + storeFieldPos)));
        break;
      case PDB_FIELD_TYPE::TYPE_DOUBLE:
        DBVAL_ELE_SET_DOUBLE(pVals, queryFieldPos, Coding::DecodeDouble(Coding::FixedDecode64(pRec + storeFieldPos)));
        break;
      case PDB_FIELD_TYPE::TYPE_STRING:
        blockLen = 0;
        blockPos = Coding::FixedDecode16(pRec + storeFieldPos);
        if (blockPos > 0)
        {
          pBlockBg = Coding::VarintDecode32((pRec + blockPos), (pRec + recLen), &blockLen);
          if (pBlockBg == nullptr || (pBlockBg + blockLen) > (pRec + recLen))
          {
            return PdbE_RECORD_FAIL;
          }
          DBVAL_ELE_SET_STRING(pVals, queryFieldPos, pBlockBg, blockLen);
        }
        else
        {
          DBVAL_ELE_SET_STRING(pVals, queryFieldPos, nullptr, 0);
        }
        break;
      case PDB_FIELD_TYPE::TYPE_BLOB:
        blockLen = 0;
        blockPos = Coding::FixedDecode16(pRec + storeFieldPos);
        if (blockPos > 0)
        {
          pBlockBg = Coding::VarintDecode32((pRec + blockPos), (pRec + recLen), &blockLen);
          if (pBlockBg == nullptr || (pBlockBg + blockLen) > (pRec + recLen))
          {
            return PdbE_RECORD_FAIL;
          }
          DBVAL_ELE_SET_BLOB(pVals, queryFieldPos, pBlockBg, blockLen);
        }
        else
        {
          DBVAL_ELE_SET_BLOB(pVals, queryFieldPos, nullptr, 0);
        }
        break;
      case PDB_FIELD_TYPE::TYPE_REAL2:
        realVal = Coding::FixedDecode64(pRec + storeFieldPos);
        DBVAL_ELE_SET_DOUBLE(pVals, queryFieldPos, (static_cast<double>(realVal) / DBVAL_REAL2_MULTIPLE));
        break;
      case PDB_FIELD_TYPE::TYPE_REAL3:
        realVal = Coding::FixedDecode64(pRec + storeFieldPos);
        DBVAL_ELE_SET_DOUBLE(pVals, queryFieldPos, (static_cast<double>(realVal) / DBVAL_REAL3_MULTIPLE));
        break;
      case PDB_FIELD_TYPE::TYPE_REAL4:
        realVal = Coding::FixedDecode64(pRec + storeFieldPos);
        DBVAL_ELE_SET_DOUBLE(pVals, queryFieldPos, (static_cast<double>(realVal) / DBVAL_REAL4_MULTIPLE));
        break;
      case PDB_FIELD_TYPE::TYPE_REAL6:
        realVal = Coding::FixedDecode64(pRec + storeFieldPos);
        DBVAL_ELE_SET_DOUBLE(pVals, queryFieldPos, (static_cast<double>(realVal) / DBVAL_REAL6_MULTIPLE));
        break;
      }
    }

    isAdd = false;
    retVal = pQuery->AppendSingle(pVals, valCnt, &isAdd);
    if (retVal != PdbE_OK)
      return retVal;

    if PDB_CONSTEXPR(IsSnapshot)
    {
      if (pIsAdd != nullptr)
        *pIsAdd = isAdd;

      return PdbE_OK;
    }

    if PDB_CONSTEXPR(QuerySingle)
    {
      if (isAdd)
      {
        if (pIsAdd != nullptr)
          *pIsAdd = true;

        return PdbE_OK;
      }
    }

    if PDB_CONSTEXPR(IsAsc)
    {
      idx++;
      if (DBVAL_ELE_IS_DATETIME(pVals, PDB_TSTAMP_INDEX) && DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX) > edTs)
        break;
    }
    else
    {
      idx--;
      if (DBVAL_ELE_IS_DATETIME(pVals, PDB_TSTAMP_INDEX) && DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX) < bgTs)
        break;
    }
  }

  return PdbE_OK;
}


PdbErr_t NormalDataPart::DumpToCompPartId(int64_t devId, CompPartBuilder* pPartBuilder)
{
  PdbErr_t retVal = PdbE_OK;
  PageRef pageRef;
  PageHdr* pPage = nullptr;
  MemPageIdx pageIdx;
  NormalDataPage dataPage;
  size_t fieldCnt = fieldInfoVec_.size();
  std::vector<DBVal> valVec;
  std::vector<int> typeVec;
  valVec.reserve(fieldCnt);
  typeVec.reserve(fieldCnt);
  for (size_t idx = 0; idx < fieldCnt; idx++)
  {
    typeVec.push_back(fieldInfoVec_[idx].GetFieldType());
  }

  DBVal* pVals = valVec.data();
  DBVAL_ELE_SET_INT64(pVals, PDB_DEVID_INDEX, devId);

  retVal = normalIdx_.GetIndex(devId, 0, &pageIdx);
  if (retVal != PdbE_OK)
    return retVal;

  const char* pRec = nullptr;
  while (true)
  {
    retVal = GetPage(pageIdx.pageNo_, &pageRef);
    if (retVal != PdbE_OK)
      return retVal;

    pPage = pageRef.GetPageHdr();
    retVal = dataPage.Load(pPage);
    if (retVal != PdbE_OK)
      return retVal;

    int recCnt = dataPage.GetRecCnt();
    for (int idx = 0; idx < recCnt; idx++)
    {
      retVal = dataPage.GetRecData(idx, &pRec);
      if (retVal != PdbE_OK)
        return retVal;

      const char* pBlockBg = nullptr;
      size_t blockPos = 0;
      uint32_t blockLen = 0;
      uint32_t recLen = Coding::FixedDecode16(pRec);
      for (size_t fieldIdx = PDB_TSTAMP_INDEX; fieldIdx < fieldCnt; fieldIdx++)
      {
        size_t storePos = fieldPosVec_[fieldIdx];
        switch (typeVec[fieldIdx])
        {
        case PDB_FIELD_TYPE::TYPE_BOOL:
          DBVAL_ELE_SET_BOOL(pVals, fieldIdx, (pRec[storePos] == PDB_BOOL_TRUE));
          break;
        case PDB_FIELD_TYPE::TYPE_INT8:
          DBVAL_ELE_SET_INT8(pVals, fieldIdx, static_cast<int8_t>(pRec[storePos]));
          break;
        case PDB_FIELD_TYPE::TYPE_INT16:
          DBVAL_ELE_SET_INT16(pVals, fieldIdx, Coding::FixedDecode16(pRec + storePos));
          break;
        case PDB_FIELD_TYPE::TYPE_INT32:
          DBVAL_ELE_SET_INT32(pVals, fieldIdx, Coding::FixedDecode32(pRec + storePos));
          break;
        case PDB_FIELD_TYPE::TYPE_INT64:
        case PDB_FIELD_TYPE::TYPE_REAL2:
        case PDB_FIELD_TYPE::TYPE_REAL3:
        case PDB_FIELD_TYPE::TYPE_REAL4:
        case PDB_FIELD_TYPE::TYPE_REAL6:
          DBVAL_ELE_SET_INT64(pVals, fieldIdx, Coding::FixedDecode64(pRec + storePos));
          break;
        case PDB_FIELD_TYPE::TYPE_DATETIME:
          DBVAL_ELE_SET_DATETIME(pVals, fieldIdx, Coding::FixedDecode64(pRec + storePos));
          break;
        case PDB_FIELD_TYPE::TYPE_FLOAT:
          DBVAL_ELE_SET_FLOAT(pVals, fieldIdx, Coding::DecodeFloat(Coding::FixedDecode32(pRec + storePos)));
          break;
        case PDB_FIELD_TYPE::TYPE_DOUBLE:
          DBVAL_ELE_SET_DOUBLE(pVals, fieldIdx, Coding::DecodeDouble(Coding::FixedDecode64(pRec + storePos)));
          break;
        case PDB_FIELD_TYPE::TYPE_STRING:
          blockLen = 0;
          blockPos = Coding::FixedDecode16(pRec + storePos);
          if (blockPos > 0)
          {
            pBlockBg = Coding::VarintDecode32((pRec + blockPos), (pRec + recLen), &blockLen);
            if (pBlockBg == nullptr || (pBlockBg + blockLen) > (pRec + recLen))
            {
              return PdbE_RECORD_FAIL;
            }
            DBVAL_ELE_SET_STRING(pVals, fieldIdx, pBlockBg, blockLen);
          }
          else
          {
            DBVAL_ELE_SET_STRING(pVals, fieldIdx, nullptr, 0);
          }
          break;
        case PDB_FIELD_TYPE::TYPE_BLOB:
          blockLen = 0;
          blockPos = Coding::FixedDecode16(pRec + storePos);
          if (blockPos > 0)
          {
            pBlockBg = Coding::VarintDecode32((pRec + blockPos), (pRec + recLen), &blockLen);
            if (pBlockBg == nullptr || (pBlockBg + blockLen) > (pRec + recLen))
            {
              return PdbE_RECORD_FAIL;
            }
            DBVAL_ELE_SET_BLOB(pVals, fieldIdx, pBlockBg, blockLen);
          }
          else
          {
            DBVAL_ELE_SET_BLOB(pVals, fieldIdx, nullptr, 0);
          }
          break;
        }
      }

      retVal = pPartBuilder->Append(pVals, fieldCnt);
      if (retVal != PdbE_OK)
        return retVal;
    }

    retVal = normalIdx_.GetNextIndex(devId, pageIdx.idxTs_, &pageIdx);
    if (retVal == PdbE_IDX_NOT_FOUND)
      break;

    if (retVal != PdbE_OK)
      return retVal;

    if (glbCancelCompTask || !glbRunning)
      return PdbE_TASK_CANCEL;
  }

  return PdbE_OK;
}

PdbErr_t NormalDataPart::GetPage(int32_t pageNo, PageRef* pPageRef)
{
  PdbErr_t retVal = PdbE_OK;
  PageHdr* pTmpPage = nullptr;

  if (pageNo <= 0 || pPageRef == nullptr)
    return PdbE_INVALID_PARAM;
  uint64_t pageCode = pageCodeMask_ | pageNo;
  retVal = pGlbPagePool->GetPage(pageCode, pPageRef);
  if (retVal != PdbE_OK)
    return retVal;

  pTmpPage = pPageRef->GetPageHdr();
  if (!PAGEHDR_IS_INIT(pTmpPage))
  {
    retVal = dataFile_.Read(PAGEHDR_GET_PAGEDATA(pTmpPage),
      NORMAL_PAGE_SIZE, NORMAL_PAGE_OFFSET(pageNo));
    if (retVal != PdbE_OK)
    {
      pPageRef->Attach(nullptr);
      return retVal;
    }
    PAGEHDR_SET_INIT_TRUE(pTmpPage);
  }

  return PdbE_OK;
}

PdbErr_t NormalDataPart::AllocPage(PageRef* pPageRef)
{
  PdbErr_t retVal = PdbE_OK;

  if (pPageRef == nullptr)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> allocPageLock(allocPageMutex_);
  size_t dataFileSize = dataFile_.FileSize();
  if (((size_t)nextPageNo_ + 1) * NORMAL_PAGE_SIZE > dataFileSize)
  {
    //扩充文件
    retVal = dataFile_.Grow(NORMAL_DATA_GROW_SIZE);
    if (retVal != PdbE_OK)
      return retVal;
  }

  uint64_t pageCode = pageCodeMask_ | nextPageNo_; 
  retVal = pGlbPagePool->GetPage(pageCode, pPageRef);
  if (retVal != PdbE_OK)
    return retVal;

  nextPageNo_++;
  return PdbE_OK;
}

PdbErr_t NormalDataPart::WritePages(const std::vector<PageHdr*>& hdrVec, OSFile* pDwFile)
{
  PdbErr_t retVal = PdbE_OK;

  NormalDataPage dataPage;
  NormalDataHead* pDataHead = nullptr;
  std::mutex* pPageMutex = nullptr;
  char* pPageBuf = nullptr;
  Arena arena;
  std::string idxBuf;

  pPageBuf = arena.AllocateAligned(NORMAL_PAGE_SIZE * (SYNC_PAGE_CNT + 1));
  if (pPageBuf == nullptr)
    return PdbE_NOMEM;

  size_t bufMod = reinterpret_cast<uintptr_t>(pPageBuf) & (NORMAL_PAGE_SIZE - 1);
  pPageBuf += (bufMod == 0 ? 0 : (NORMAL_PAGE_SIZE - bufMod));

  size_t tmpPageCnt = 0;
  size_t curIdx = 0;
  size_t partIdx = 0;
  uint32_t firstPageCrc = 0;

  while (curIdx < hdrVec.size())
  {
    idxBuf.clear();
    tmpPageCnt = 0;

    memset(pPageBuf, 0, (NORMAL_PAGE_SIZE * SYNC_PAGE_CNT));

    while (curIdx < hdrVec.size())
    {
      pPageMutex = pGlbMutexManager->GetPageMutex(PAGEHDR_GET_PAGENO(hdrVec[curIdx]));
      std::unique_lock<std::mutex> pageLock(*pPageMutex);
      PAGEHDR_ROUND_ROBIN(hdrVec[curIdx]);
      dataPage.Load(hdrVec[curIdx]);
      dataPage.UpdateCrc();
      memcpy((pPageBuf + tmpPageCnt * NORMAL_PAGE_SIZE),
        PAGEHDR_GET_PAGEDATA(hdrVec[curIdx]), NORMAL_PAGE_SIZE);

      if (PAGEHDR_ONLY_MEM(hdrVec[curIdx]))
      {
        normalIdx_.AppendIdx(idxBuf, dataPage.GetDevId(), dataPage.GetIdxTs(), dataPage.GetPageNo());
      }

      curIdx++;
      tmpPageCnt++;
      dataPage.Load(nullptr);

      if (tmpPageCnt == SYNC_PAGE_CNT)
        break;
    }

    pDataHead = (NormalDataHead*)pPageBuf;
    firstPageCrc = NormalDataHead_GetPageCrc(pDataHead);
    uint32_t allPageCrc = StringTool::CRC32(pPageBuf, (NORMAL_PAGE_SIZE * SYNC_PAGE_CNT - sizeof(uint32_t)), sizeof(uint32_t));
    NormalDataHead_SetPageCrc(pDataHead, allPageCrc);

    //1. 写入临时文件
    retVal = pDwFile->Write(pPageBuf, (SYNC_PAGE_CNT * NORMAL_PAGE_SIZE), 0);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to sync page cache to data file, write double write file error {}", retVal);
      return retVal;
    }

    NormalDataHead_SetPageCrc(pDataHead, firstPageCrc);

    for (size_t idx = 0; idx < tmpPageCnt; idx++)
    {
      int32_t pageNo = PAGEHDR_GET_PAGENO(hdrVec[partIdx + idx]);
      retVal = dataFile_.Write((pPageBuf + idx * NORMAL_PAGE_SIZE), NORMAL_PAGE_SIZE, NORMAL_PAGE_OFFSET(pageNo));
      if (retVal != PdbE_OK)
      {
        LOG_ERROR("failed to sync page cache to data file, write data file error {}", retVal);
        return retVal;
      }

      pPageMutex = pGlbMutexManager->GetPageMutex(PAGEHDR_GET_PAGENO(hdrVec[partIdx + idx]));
      std::unique_lock<std::mutex> pageLock(*pPageMutex);
      PAGEHDR_SYNC_DATA(hdrVec[partIdx + idx]);
      if (PAGEHDR_IS_DIRTY(hdrVec[partIdx + idx]))
      {
        std::unique_lock<std::mutex> dirtyLock(dirtyMutex_);
        dirtyList_.push_back(hdrVec[partIdx + idx]);
      }
    }

    //写入索引信息
    normalIdx_.WriteIdx(idxBuf);

    partIdx = curIdx;
    pGlbPagePool->NotifyAll();
  }

  return PdbE_OK;
}
