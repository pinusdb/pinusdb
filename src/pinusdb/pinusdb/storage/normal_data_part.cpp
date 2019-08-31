#include "storage/normal_data_part.h"
#include "storage/normal_data_page.h"
#include "db/page_pool.h"
#include "util/coding.h"
#include "global_variable.h"
#include "util/log_util.h"
#include "util/date_time.h"
#include "storage/comp_block_builder.h"

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
  const TableInfo* pTabInfo, int32_t partCode)
{
  PdbErr_t retVal = PdbE_OK;
  OSFile dataFile;
  Arena arena;
  char* pTmpMeta = arena.Allocate(sizeof(DataFileMeta));
  DataFileMeta* pMeta = (DataFileMeta*)pTmpMeta;
  size_t fieldCnt = pTabInfo->GetFieldCnt();

  ZeroMemory(pTmpMeta, sizeof(DataFileMeta));
  strncpy(pMeta->headStr_, NORMAL_DATA_FILE_HEAD_STR, sizeof(pMeta->headStr_));

  pMeta->partCode_ = partCode;
  pMeta->pageSize_ = NORMAL_PAGE_SIZE;
  pMeta->fieldCnt_ = static_cast<uint32_t>(fieldCnt);
  pMeta->tableType_ = PDB_PART_TYPE_NORMAL_VAL;

  int32_t fieldType = 0;
  const char* pFieldName = nullptr;
  for (size_t i = 0; i < fieldCnt; i++)
  {
    pTabInfo->GetFieldInfo(i, &fieldType);
    pFieldName = pTabInfo->GetFieldName(i);

    strncpy(pMeta->fieldRec_[i].fieldName_, pFieldName, PDB_FILED_NAME_LEN);
    pMeta->fieldRec_[i].fieldType_ = fieldType;
  }

  pMeta->crc_ = StringTool::CRC32(pTmpMeta, (sizeof(DataFileMeta) - 4));

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

PdbErr_t NormalDataPart::Open(uint8_t tabCode, int32_t partCode,
  const TableInfo* pTabInfo, const char* pIdxPath, const char* pDataPath)
{
  PdbErr_t retVal = PdbE_OK;
  Arena arena;
  idxPath_ = pIdxPath;
  dataPath_ = pDataPath;
  readOnly_ = false;

  char* pTmpMeta = arena.Allocate(sizeof(DataFileMeta));
  if (pTmpMeta == nullptr)
    return PdbE_NOMEM;

  retVal = dataFile_.OpenNoBuf(pDataPath, false);
  if (retVal != PdbE_OK)
    return retVal;

  retVal = dataFile_.Read(pTmpMeta, sizeof(DataFileMeta), 0);
  if (retVal != PdbE_OK)
    return retVal;

  DataFileMeta* pDataMeta = (DataFileMeta*)pTmpMeta;
  if (StringTool::CRC32(pTmpMeta, (sizeof(DataFileMeta) - 4)) != pDataMeta->crc_)
  {
    LOG_ERROR("failed to open data file ({}), file meta crc error", pDataPath);
    return PdbE_PAGE_ERROR;
  }

  //判断数据文件表结构是否和指定的表结构一致
  if (pDataMeta->fieldCnt_ != pTabInfo->GetFieldCnt())
  {
    LOG_ERROR("failed to open data file ({}), field list mismatch", pDataPath);
    return PdbE_PAGE_ERROR;
  }

  int fieldType = 0;
  FieldInfo finfo;
  for (size_t i = 0; i < pDataMeta->fieldCnt_; i++)
  {
    retVal = finfo.SetFieldInfo(pDataMeta->fieldRec_[i].fieldName_,
      pDataMeta->fieldRec_[i].fieldType_, false);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to open data file ({}), invalid field info", pDataPath);
      return retVal;
    }

    pTabInfo->GetFieldInfo(i, &fieldType);
    if (finfo.GetFieldType() != fieldType)
    {
      LOG_ERROR("failed to open data file ({}), field list mismatch", pDataPath);
      return PdbE_PAGE_ERROR;
    }

    if (!StringTool::ComparyNoCase(finfo.GetFieldName(), pTabInfo->GetFieldName(i)))
    {
      LOG_ERROR("failed to open data file ({}), field list mismatch", pDataPath);
      return PdbE_PAGE_ERROR;
    }

    fieldVec_.push_back(finfo);
  }

  retVal = normalIdx_.Open(pIdxPath, false);
  if (retVal != PdbE_OK)
    return retVal;

  int32_t idxPartCode = normalIdx_.GetPartCode();
  if (idxPartCode != partCode)
  {
    LOG_ERROR("failed to open data file ({}), date code error", pDataPath);
    return PdbE_PAGE_ERROR;
  }

  if (idxPartCode != pDataMeta->partCode_)
  {
    LOG_ERROR("failed to open data file, mismatch between index file ({}:{}) and data file ({}:{}) ",
      pIdxPath, idxPartCode, pDataPath, pDataMeta->partCode_);
    return PdbE_PAGE_ERROR;
  }

  bgDayTs_ = (int64_t)idxPartCode * MillisPerDay;
  edDayTs_ = bgDayTs_ + MillisPerDay;
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
  NormalPageIdx pageIdx;
  const char* pPageData = nullptr;
  NormalDataHead* pPageHead = nullptr;
  std::vector<NormalPageIdx> idxVec;
  memset(&pageIdx, 0, sizeof(NormalPageIdx));

  for (int i = 0; i < SYNC_PAGE_CNT; i++)
  {
    pPageData = pPageBuf + (NORMAL_PAGE_SIZE * i);
    pPageHead = (NormalDataHead*)pPageData;

    if (pPageHead->pageCrc_ == 0)
      break;

    retVal = dataFile_.Write(pPageData, NORMAL_PAGE_SIZE, NORMAL_PAGE_OFFSET(pPageHead->pageNo_));
    if (retVal != PdbE_OK)
      return retVal;

    retVal = normalIdx_.GetIndex(pPageHead->devId_, pPageHead->idxTs_, &pageIdx);
    if (retVal != PdbE_OK || pageIdx.idxTs_ != pPageHead->idxTs_)
    {
      normalIdx_.AddIdx(pPageHead->devId_, pPageHead->idxTs_,
        pPageHead->pageNo_);

      pageIdx.idxTs_ = pPageHead->idxTs_;
      pageIdx.devId_ = pPageHead->devId_;
      pageIdx.pageNo_ = pPageHead->pageNo_;
      idxVec.push_back(pageIdx);

      //更新下个分配的页号
      if (pageIdx.pageNo_ >= nextPageNo_)
        nextPageNo_ = pageIdx.pageNo_ + 1;
    }
  }

  //写入索引信息
  if (!idxVec.empty())
  {
    normalIdx_.WriteIdx(idxVec);
  }

  return PdbE_OK;
}

PdbErr_t NormalDataPart::InsertRec(int64_t devId, int64_t tstamp,
  bool replace, const uint8_t* pRec, size_t recLen)
{
  PdbErr_t retVal = PdbE_OK;
  if (readOnly_)
    return PdbE_FILE_READONLY;

  std::mutex* pDevMutex = pGlbMutexManager->GetDevMutex(devId);
  NormalPageIdx pageIdx;
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

    dirtyList_.push_back(pPage);
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
      if (oldPageDirty && PAGEHDR_GET_SPLIT_GRP(pPage) == 0)
      {
        PAGEHDR_SET_IS_FULL(pPage);
      }
    }
    else
    {
      //如果原始页已经被分裂过，在同步完之前不允许分裂
      if (PAGEHDR_GET_SPLIT_GRP(pPage) != 0)
        return PdbE_RETRY;

      //分裂原来的数据页
      if (normalIdx_.GetNextIndex(devId, tstamp, &pageIdx) != PdbE_OK)
      {
        //如果是最后一个页，将少部分分裂到新页
        retVal = dataPage.SplitMost(&newDataPage, (24 * 1024));
        if (retVal == PdbE_OK && PAGEHDR_ONLY_MEM(pPage))
        {
          PAGEHDR_SET_IS_FULL(pPage); //设置旧页已满， 加快写盘
        }
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
  PageDataIter dataIter;
  CompPartBuilder partBuilder;

  retVal = dataIter.InitForDump(fieldVec_);
  if (retVal != PdbE_OK)
    return retVal;

  std::vector<int64_t> allDevIdVec;
  normalIdx_.GetAllDevId(allDevIdVec);

  retVal = partBuilder.Create(normalIdx_.GetPartCode(), pDataPath, fieldVec_);
  if (retVal != PdbE_OK)
    return retVal;

  for (auto devIt = allDevIdVec.begin(); devIt != allDevIdVec.end(); devIt++)
  {
    retVal = DumpToCompPartId(*devIt, &dataIter, &partBuilder);
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

PdbErr_t NormalDataPart::QueryDevAsc(int64_t devId, void* pQueryParam,
  IResultFilter* pResult, uint64_t timeOut, bool queryFirst, bool* pIsAdd)
{
  PdbErr_t retVal = PdbE_OK;
  NormalPageIdx pageIdx;
  PageDataIter* pDataIter = (PageDataIter*)pQueryParam;
  int64_t bgTs = pDataIter->GetBgTs();
  int64_t edTs = pDataIter->GetEdTs();
  size_t fieldCnt = pDataIter->GetFieldCnt();
  PageRef pageRef;
  PageHdr* pPage = nullptr;
  std::mutex* pDevMutex = pGlbMutexManager->GetDevMutex(devId);
  std::mutex* pPageMutex = nullptr;
  int64_t curTs = 0;
  bool firstPage = true;
  bool isAdd = false;

  retVal = normalIdx_.GetIndex(devId, bgTs, &pageIdx);
  if (retVal == PdbE_DEV_NOT_FOUND)
    return PdbE_OK;

  if (retVal != PdbE_OK)
    return retVal;

  while (true)
  {
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

    pPage = pageRef.GetPageHdr();
    retVal = pDataIter->Load(pPage);
    if (retVal != PdbE_OK)
      return retVal;

    if (firstPage)
    {
      retVal = pDataIter->SeekTo(bgTs);
      firstPage = false;
    }
    else
    {
      retVal = pDataIter->SeekToFirst();
    }
    if (retVal != PdbE_OK)
      return retVal;

    while (pDataIter->Valid())
    {
      DBVal* pVals = pDataIter->GetRecord();
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

      pDataIter->Next();
    }

    if (GetTickCount64() > timeOut)
      return PdbE_QUERY_TIME_OUT;

    retVal = normalIdx_.GetNextIndex(devId, curTs, &pageIdx);
    if (retVal == PdbE_IDX_NOT_FOUND)
      return PdbE_OK;

    if (retVal != PdbE_OK)
      return retVal;
  }
}

PdbErr_t NormalDataPart::QueryDevDesc(int64_t devId, void* pQueryParam,
  IResultFilter* pResult, uint64_t timeOut, bool queryLast, bool* pIsAdd)
{
  PdbErr_t retVal = PdbE_OK;
  NormalPageIdx pageIdx;
  PageDataIter* pDataIter = (PageDataIter*)pQueryParam;
  int64_t bgTs = pDataIter->GetBgTs();
  int64_t edTs = pDataIter->GetEdTs();
  size_t fieldCnt = pDataIter->GetFieldCnt();
  PageRef pageRef;
  PageHdr* pPage = nullptr;
  std::mutex* pDevMutex = pGlbMutexManager->GetDevMutex(devId);
  std::mutex* pPageMutex = nullptr;
  int64_t curTs = 0;
  bool firstPage = true;
  bool isAdd = false;

  retVal = normalIdx_.GetIndex(devId, edTs, &pageIdx);
  if (retVal == PdbE_DEV_NOT_FOUND)
    return PdbE_OK;

  if (retVal != PdbE_OK)
    return retVal;

  while (true)
  {
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

    pPage = pageRef.GetPageHdr();
    retVal = pDataIter->Load(pPage);
    if (retVal != PdbE_OK)
      return retVal;

    if (firstPage)
    {
      retVal = pDataIter->SeekTo(edTs);
      firstPage = false;
    }
    else
    {
      retVal = pDataIter->SeekToLast();
    }
    if (retVal != PdbE_OK)
      return retVal;

    while (pDataIter->Valid())
    {
      DBVal* pVals = pDataIter->GetRecord();
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

      pDataIter->Prev();
    }

    if (GetTickCount64() > timeOut)
      return PdbE_QUERY_TIME_OUT;

    retVal = normalIdx_.GetPrevIndex(devId, curTs, &pageIdx);
    if (retVal == PdbE_IDX_NOT_FOUND)
      return PdbE_OK;

    if (retVal != PdbE_OK)
      return retVal;
  }

}

PdbErr_t NormalDataPart::DumpToCompPartId(int64_t devId, PageDataIter* pDataIter, CompPartBuilder* pPartBuilder)
{
  PdbErr_t retVal = PdbE_OK;
  PageRef pageRef;
  PageHdr* pPage = nullptr;
  size_t fieldCnt = pDataIter->GetFieldCnt();
  NormalPageIdx pageIdx;

  retVal = normalIdx_.GetIndex(devId, 0, &pageIdx);
  if (retVal != PdbE_OK)
    return retVal;

  while (true)
  {
    retVal = GetPage(pageIdx.pageNo_, &pageRef);
    if (retVal != PdbE_OK)
      return retVal;

    pPage = pageRef.GetPageHdr();
    retVal = pDataIter->Load(pPage);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pDataIter->SeekToFirst();
    if (retVal != PdbE_OK)
      return retVal;

    while (pDataIter->Valid())
    {
      DBVal* pVals = pDataIter->GetRecord();

      retVal = pPartBuilder->Append(pVals, fieldCnt);
      if (retVal != PdbE_OK)
        return retVal;

      pDataIter->Next();
    }

    retVal = normalIdx_.GetNextIndex(devId, pageIdx.idxTs_, &pageIdx);
    if (retVal == PdbE_IDX_NOT_FOUND)
      break;

    if (retVal != PdbE_OK)
      return retVal;

    if (glbCancelCompTask)
      return PdbE_TASK_CANCEL;
  }

  return pPartBuilder->Flush();
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

  NormalPageIdx pageIdx;
  NormalDataPage dataPage;
  NormalDataHead* pDataHead = nullptr;
  std::vector<NormalPageIdx> idxVec;
  std::mutex* pPageMutex = nullptr;
  char* pPageBuf = nullptr;
  Arena arena;

  memset(&pageIdx, 0, sizeof(pageIdx));

  pPageBuf = arena.AllocateAligned(NORMAL_PAGE_SIZE * (SYNC_PAGE_CNT + 1));
  if (pPageBuf == nullptr)
    return PdbE_NOMEM;

  size_t bufMod = reinterpret_cast<uintptr_t>(pPageBuf) & (NORMAL_PAGE_SIZE - 1);
  pPageBuf += (bufMod == 0 ? 0 : (NORMAL_PAGE_SIZE - bufMod));

  int tmpPageCnt = 0;
  size_t curIdx = 0;
  size_t partIdx = 0;
  uint32_t firstPageCrc = 0;

  while (curIdx < hdrVec.size())
  {
    idxVec.clear();
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
        pageIdx.idxTs_ = dataPage.GetIdxTs();
        pageIdx.devId_ = dataPage.GetDevId();
        pageIdx.pageNo_ = dataPage.GetPageNo();
        idxVec.push_back(pageIdx);
      }

      curIdx++;
      tmpPageCnt++;
      dataPage.Load(nullptr);

      if (tmpPageCnt == SYNC_PAGE_CNT)
        break;
    }

    pDataHead = (NormalDataHead*)pPageBuf;
    firstPageCrc = pDataHead->pageCrc_;
    pDataHead->pageCrc_ = StringTool::CRC32(pPageBuf, 
      (NORMAL_PAGE_SIZE * SYNC_PAGE_CNT - sizeof(uint32_t)), sizeof(uint32_t));

    //1. 写入临时文件
    retVal = pDwFile->Write(pPageBuf, (SYNC_PAGE_CNT * NORMAL_PAGE_SIZE), 0);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to sync page cache to data file, write double write file error {}", retVal);
      return retVal;
    }

    pDataHead->pageCrc_ = firstPageCrc;

    for (int idx = 0; idx < tmpPageCnt; idx++)
    {
      retVal = dataFile_.Write((pPageBuf + idx * NORMAL_PAGE_SIZE), NORMAL_PAGE_SIZE,
        NORMAL_PAGE_OFFSET(PAGEHDR_GET_PAGENO(hdrVec[partIdx + idx])));
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
    if (!idxVec.empty())
    {
      normalIdx_.WriteIdx(idxVec);
    }

    partIdx = curIdx;
  }

  return PdbE_OK;
}

void* NormalDataPart::InitQueryParam(int* pTypes, size_t fieldCnt, int64_t bgTs, int64_t edTs)
{
  PageDataIter* pDataIter = new (std::nothrow) PageDataIter();
  if (pDataIter == nullptr)
    return nullptr;

  if (pDataIter->Init(pTypes, fieldCnt, bgTs, edTs) != PdbE_OK)
  {
    delete pDataIter;
    return nullptr;
  }

  return pDataIter;
}

void NormalDataPart::ClearQueryParam(void* pQueryParam)
{
  PageDataIter* pDataIter = (PageDataIter*)pQueryParam;
  delete pDataIter;
}

NormalDataPart::PageDataIter::PageDataIter()
{
  fieldCnt_ = 0;
  pTypes_ = nullptr;
  pVals_ = nullptr;
  bgTs_ = 0;
  edTs_ = 0;
  pHdr_ = nullptr;
  devId_ = 0;
  curIdx_ = 0;
}

NormalDataPart::PageDataIter::~PageDataIter()
{
}

PdbErr_t NormalDataPart::PageDataIter::Init(int* pTypes, size_t fieldCnt, int64_t bgTs, int64_t edTs)
{
  fieldCnt_ = fieldCnt;
  pTypes_ = (int*)arena_.AllocateAligned((sizeof(int) * fieldCnt_));
  pVals_ = (DBVal*)arena_.AllocateAligned((sizeof(DBVal) * fieldCnt_));

  if (pTypes_ == nullptr || pVals_ == nullptr)
    return PdbE_NOMEM;

  memcpy(pTypes_, pTypes, (sizeof(int) * fieldCnt));
  bgTs_ = bgTs;
  edTs_ = edTs;
  pHdr_ = nullptr;
  devId_ = 0;
  curIdx_ = 0;

  return PdbE_OK;
}

PdbErr_t NormalDataPart::PageDataIter::InitForDump(const std::vector<FieldInfo>& fieldVec_)
{
  fieldCnt_ = fieldVec_.size();
  pTypes_ = (int*)arena_.AllocateAligned((sizeof(int) * fieldCnt_));
  pVals_ = (DBVal*)arena_.AllocateAligned((sizeof(DBVal) * fieldCnt_));

  if (pTypes_ == nullptr || pVals_ == nullptr)
    return PdbE_NOMEM;

  for (size_t idx = 0; idx < fieldCnt_; idx++)
  {
    pTypes_[idx] = fieldVec_[idx].GetFieldType();
    if (pTypes_[idx] == PDB_FIELD_TYPE::TYPE_REAL2
      || pTypes_[idx] == PDB_FIELD_TYPE::TYPE_REAL3
      || pTypes_[idx] == PDB_FIELD_TYPE::TYPE_REAL4
      || pTypes_[idx] == PDB_FIELD_TYPE::TYPE_REAL6)
    {
      pTypes_[idx] = PDB_FIELD_TYPE::TYPE_INT64;
    }
  }

  bgTs_ = MinMillis;
  edTs_ = MaxMillis;
  pHdr_ = nullptr;
  devId_ = 0;
  curIdx_ = 0;

  return PdbE_OK;
}

PdbErr_t NormalDataPart::PageDataIter::Load(PageHdr* pHdr)
{
  PdbErr_t retVal = PdbE_OK;

  curIdx_ = 0;
  devId_ = 0;
  pHdr_ = pHdr;

  retVal = normalPage_.Load(pHdr_);
  if (retVal != PdbE_OK)
    return retVal;
  
  devId_ = normalPage_.GetDevId();
  return PdbE_OK;
}

bool NormalDataPart::PageDataIter::Valid() const
{
  return pHdr_ != nullptr && curIdx_ >= 0 && curIdx_ < normalPage_.GetRecCnt();
}

PdbErr_t NormalDataPart::PageDataIter::SeekTo(int64_t tstamp)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t recCnt = normalPage_.GetRecCnt();
  int32_t lwr = 0;
  int32_t upr = recCnt - 1;
  int32_t idx = 0;
  int64_t curTs = 0;

  if (recCnt == 0)
    return PdbE_OK;

  while (lwr <= upr)
  {
    idx = (lwr + upr) / 2;
    retVal = normalPage_.GetRecTstamp(idx, &curTs);
    if (retVal != PdbE_OK)
      return retVal;

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

PdbErr_t NormalDataPart::PageDataIter::SeekToFirst()
{
  curIdx_ = 0;
  return PdbE_OK;
}

PdbErr_t NormalDataPart::PageDataIter::SeekToLast()
{
  curIdx_ = normalPage_.GetRecCnt() - 1;
  return PdbE_OK;
}

PdbErr_t NormalDataPart::PageDataIter::Next()
{
  curIdx_++;
  return PdbE_OK;
}

PdbErr_t NormalDataPart::PageDataIter::Prev()
{
  curIdx_--;
  return PdbE_OK;
}

DBVal* NormalDataPart::PageDataIter::GetRecord()
{
  const PdbByte* pRecBg = nullptr;
  const PdbByte* pRecLimit = nullptr;
  const PdbByte* pRecVal = nullptr;
  int64_t tstamp = 0;
  uint64_t u64val = 0;
  double realVal = 0;
  uint32_t valLen = 0;
  uint32_t u32val = 0;
  uint16_t recLen = 0;
  if (normalPage_.GetRecData(curIdx_, &pRecBg) != PdbE_OK)
    return nullptr;

  memcpy(&recLen, pRecBg, sizeof(recLen));
  pRecLimit = pRecBg + recLen;

  pRecVal = pRecBg + sizeof(recLen);
  pRecVal = Coding::VarintDecode64(pRecVal, pRecLimit, &u64val);
  tstamp = u64val;

  DBVAL_ELE_SET_INT64(pVals_, PDB_DEVID_INDEX, devId_);
  DBVAL_ELE_SET_DATETIME(pVals_, PDB_TSTAMP_INDEX, tstamp);
  for (size_t idx = (PDB_TSTAMP_INDEX + 1); idx < fieldCnt_; idx++)
  {
    switch (pTypes_[idx])
    {
    case PDB_FIELD_TYPE::TYPE_BOOL:
      DBVAL_ELE_SET_BOOL(pVals_, idx, (*pRecVal == PDB_BOOL_TRUE));
      pRecVal++;
      break;
    case PDB_FIELD_TYPE::TYPE_INT64:
      pRecVal = Coding::VarintDecode64(pRecVal, pRecLimit, &u64val);
      DBVAL_ELE_SET_INT64(pVals_, idx, Coding::ZigzagDecode64(u64val));
      break;
    case PDB_FIELD_TYPE::TYPE_DATETIME:
      pRecVal = Coding::VarintDecode64(pRecVal, pRecLimit, &u64val);
      DBVAL_ELE_SET_DATETIME(pVals_, idx, u64val);
      break;
    case PDB_FIELD_TYPE::TYPE_DOUBLE:
      u64val = Coding::FixedDecode64(pRecVal);
      DBVAL_ELE_SET_DOUBLE_FOR_UINT64(pVals_, idx, u64val);
      pRecVal += 8;
      break;
    case PDB_FIELD_TYPE::TYPE_STRING:
      pRecVal = Coding::VarintDecode32(pRecVal, pRecLimit, &valLen);
      DBVAL_ELE_SET_STRING(pVals_, idx, pRecVal, valLen);
      pRecVal += valLen;
      break;
    case PDB_FIELD_TYPE::TYPE_BLOB:
      pRecVal = Coding::VarintDecode32(pRecVal, pRecLimit, &valLen);
      DBVAL_ELE_SET_BLOB(pVals_, idx, pRecVal, valLen);
      pRecVal += valLen;
      break;
    case PDB_FIELD_TYPE::TYPE_REAL2:
      pRecVal = Coding::VarintDecode64(pRecVal, pRecLimit, &u64val);
      realVal = static_cast<double>(Coding::ZigzagDecode64(u64val));
      DBVAL_ELE_SET_DOUBLE(pVals_, idx, (realVal / DBVAL_REAL2_MULTIPLE));
      break;
    case PDB_FIELD_TYPE::TYPE_REAL3:
      pRecVal = Coding::VarintDecode64(pRecVal, pRecLimit, &u64val);
      realVal = static_cast<double>(Coding::ZigzagDecode64(u64val));
      DBVAL_ELE_SET_DOUBLE(pVals_, idx, (realVal / DBVAL_REAL3_MULTIPLE));
      break;
    case PDB_FIELD_TYPE::TYPE_REAL4:
      pRecVal = Coding::VarintDecode64(pRecVal, pRecLimit, &u64val);
      realVal = static_cast<double>(Coding::ZigzagDecode64(u64val));
      DBVAL_ELE_SET_DOUBLE(pVals_, idx, (realVal / DBVAL_REAL4_MULTIPLE));
      break;
    case PDB_FIELD_TYPE::TYPE_REAL6:
      pRecVal = Coding::VarintDecode64(pRecVal, pRecLimit, &u64val);
      realVal = static_cast<double>(Coding::ZigzagDecode64(u64val));
      DBVAL_ELE_SET_DOUBLE(pVals_, idx, (realVal / DBVAL_REAL6_MULTIPLE));
      break;
    }
  }

  return pVals_;
}
