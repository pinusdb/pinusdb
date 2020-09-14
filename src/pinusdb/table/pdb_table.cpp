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

#include "table/pdb_table.h"
#include "table/devid_table.h"
#include "util/log_util.h"
#include "util/date_time.h"
#include "global_variable.h"
#include "query/iquery.h"
#include "query/query_raw.h"
#include "query/query_group.h"
#include "query/query_snapshot.h"
#include "util/coding.h"
#include "storage/normal_data_part.h"
#include "storage/comp_data_part.h"

const uint64_t TicksPerSecond = 1000;

bool PartComp(const DataPart* pA, const DataPart* pB)
{
  return pA->GetPartCode() < pB->GetPartCode();
}

PDBTable::PDBTable()
{
  dumpPartCode_.store(-1);
  pTabInfo_ = nullptr;
  tabCode_ = 0;
  tabCrc_ = 0;
}

PDBTable::~PDBTable()
{
  Close();
}

TableInfo* PDBTable::GetTableInfo(RefUtil* pRefUtil)
{
  if (pRefUtil == nullptr)
    return nullptr;

  pRefUtil->Attach(pTabInfo_);
  return pRefUtil->GetObj<TableInfo>();
}

PdbErr_t PDBTable::AlterTable(const std::vector<ColumnItem*>& colItemVec)
{
  PdbErr_t retVal = PdbE_OK;
  TableInfo* pTabInfo = new TableInfo();

  do {
    retVal = pTabInfo->SetTableName(tabName_.c_str());
    if (retVal != PdbE_OK)
      break;

    for (auto colIt = colItemVec.begin(); colIt != colItemVec.end(); colIt++)
    {
      retVal = pTabInfo->AddField((*colIt)->GetName(), (*colIt)->GetType());
      if (retVal != PdbE_OK)
      {
        break;
      }
    }

    if (retVal != PdbE_OK)
      break;

    retVal = pTabInfo->ValidStorageTable();
    if (retVal != PdbE_OK)
      break;

    retVal = devTable_.Alter(pTabInfo);
    if (retVal != PdbE_OK)
      break;

    TableInfo* pOldInfo = pTabInfo_;
    pTabInfo_ = pTabInfo;

    while (pOldInfo->GetRefCnt() > 0)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

    delete pOldInfo;

  } while (false);

  if (retVal != PdbE_OK)
  {
    delete pTabInfo;
  }

  return retVal;
}

PdbErr_t PDBTable::OpenTable(uint32_t tabCode, const char* pTabName)
{
  PdbErr_t retVal = PdbE_OK;
  tabName_ = pTabName;
  tabCode_ = tabCode;
  tabCrc_ = StringTool::CRC64NoCase(pTabName);

  devPath_ = pGlbSysCfg->GetTablePath() + "/" + tabName_ + ".dev";
  dwPath_ = pGlbSysCfg->GetNormalDataPath() + "/sys_dw/" + tabName_ + ".dw";
  pTabInfo_ = new TableInfo();
  retVal = devTable_.Open(devPath_.c_str(), pTabName, pTabInfo_);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("failed to open table({}), ret:{}", pTabName, retVal);
    return retVal;
  }

  return PdbE_OK;
}

PdbErr_t PDBTable::Close()
{
  PdbErr_t retVal = PdbE_OK; 

  if (tabCrc_ == 0 && tabCode_ == 0)
    return PdbE_OK;

  retVal = SyncDirtyPages(true);
  if (retVal == PdbE_OK)
  {
    //删除二次写文件
    dwFile_.Close();
    FileTool::RemoveFile(dwPath_.c_str());
  }

  for (auto partIt = partVec_.begin(); partIt != partVec_.end(); partIt++)
  {
    delete *partIt;
  }

  if (pTabInfo_ == nullptr)
    delete pTabInfo_;

  partVec_.clear();
  tabCrc_ = 0;
  tabCode_ = 0;
  return PdbE_OK;
}

PdbErr_t PDBTable::RecoverDW()
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil partRef;
  char* pPageBuf = nullptr;
  Arena arena;

  if (FileTool::FileExists(dwPath_.c_str()))
  {
    //判断文件是否完整
    retVal = dwFile_.OpenNoBuf(dwPath_.c_str(), false);
    if (retVal != PdbE_OK)
      return retVal;

    do {
      if (dwFile_.FileSize() != (SYNC_PAGE_CNT * NORMAL_PAGE_SIZE))
      {
        break;
      }

      //判断文件内容是否正确
      pPageBuf = arena.AllocateAligned(NORMAL_PAGE_SIZE * SYNC_PAGE_CNT, PDB_KB_BYTES(8));
      if (pPageBuf == nullptr)
        return PdbE_NOMEM;

      retVal = dwFile_.Read(pPageBuf, (NORMAL_PAGE_SIZE * SYNC_PAGE_CNT), 0);
      if (retVal != PdbE_OK)
        return retVal;

      NormalDataHead* pPageHead = (NormalDataHead*)pPageBuf;
      uint32_t fileCrc = StringTool::CRC32(pPageBuf, (NORMAL_PAGE_SIZE * SYNC_PAGE_CNT - sizeof(uint32_t)), sizeof(uint32_t));
      uint32_t firstPageCrc = NormalDataHead_GetPageCrc(pPageHead);
      if (firstPageCrc != 0 && fileCrc == firstPageCrc)
      {
        int32_t partCode = static_cast<int32_t>(NormalDataHead_GetIdxTs(pPageHead) / DateTime::MicrosecondPerDay);
        UPDATE_NORMAL_DATA_PAGE_CRC(pPageHead);
        DataPart* pDataPart = GetDataPart(partCode, &partRef);
        if (pDataPart != nullptr)
        {
          pDataPart->RecoverDW(pPageBuf);
        }
      }

    } while (false);

    dwFile_.Close();
    FileTool::RemoveFile(dwPath_.c_str());
  }

  FileTool::MakeParentPath(dwPath_.c_str());

  retVal = dwFile_.Open(dwPath_.c_str(), false, true, true);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("failed to create double write file({})", dwPath_.c_str());
    return retVal;
  }

  dwFile_.GrowTo((SYNC_PAGE_CNT * NORMAL_PAGE_SIZE));
  return PdbE_OK;
}

PdbErr_t PDBTable::OpenDataPart(uint32_t partCode, bool isNormalPart)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil partRef;
  std::string partDateStr;
  std::string partIdxPath;
  std::string partDataPath;
  DataPart* pDataPart = nullptr;

  if (GetDataPart(partCode, &partRef) != nullptr)
  {
    return PdbE_TABLE_PART_EXIST;
  }

  retVal = BuildPartPath(partCode, isNormalPart, false,
    partDateStr, partIdxPath, partDataPath);
  if (retVal != PdbE_OK)
    return retVal;

  do {
    if (isNormalPart)
    {
      NormalDataPart* pNormalDataPart = new NormalDataPart();
      if (pNormalDataPart == nullptr)
      {
        retVal = PdbE_NOMEM;
        break;
      }

      pDataPart = pNormalDataPart;
      retVal = pNormalDataPart->Open(tabCode_, partCode, partIdxPath.c_str(), partDataPath.c_str());
    }
    else
    {
      CompDataPart* pCompDataPart = new CompDataPart();
      if (pCompDataPart == nullptr)
      {
        retVal = PdbE_NOMEM;
        break;
      }

      pDataPart = pCompDataPart;
      retVal = pCompDataPart->Open(partCode, partDataPath.c_str());
    }

    if (retVal != PdbE_OK)
      break;

    std::unique_lock<std::mutex> partLock(partMutex_);
    int partIdx = _GetDataPartPos(partCode);
    if (partIdx >= 0 && partVec_[partIdx]->GetPartCode() == partCode)
    {
      retVal = PdbE_TABLE_PART_EXIST;
      break;
    }

    partVec_.push_back(pDataPart);
    std::sort(partVec_.begin(), partVec_.end(), PartComp);
  } while (false);

  if (retVal != PdbE_OK)
  {
    if (pDataPart != nullptr)
      delete pDataPart;
  }

  return retVal;
}

PdbErr_t PDBTable::AttachPart(const char* pPartDate, int fileType)
{
  PdbErr_t retVal = PdbE_OK;

  int32_t partDateCode = 0;
  if (!DateTime::ParseDate(pPartDate, strlen(pPartDate), &partDateCode))
  {
    return PdbE_INVALID_PARAM;
  }

  if (fileType == PDB_PART_TYPE_NORMAL_VAL)
    retVal = OpenDataPart(partDateCode, true);
  else if (fileType == PDB_PART_TYPE_COMPRESS_VAL)
    retVal = OpenDataPart(partDateCode, false);
  else
    return PdbE_INVALID_PARAM;

  if (retVal == PdbE_OK)
  {
    pGlbTabCfg->AddFilePart(tabName_.c_str(), pPartDate, partDateCode, fileType);
  }

  return retVal;
}

PdbErr_t PDBTable::DetachPart(uint32_t partCode)
{
  DataPart* pDataPart = nullptr;
  int32_t invalidDays = pGlbSysCfg->GetInsertValidDay();
  int32_t invalidBgDay = DateTime::NowDayCode() - invalidDays;

  if (partCode >= static_cast<uint32_t>(invalidBgDay))
    return PdbE_DATA_FILE_IN_ACTIVE;

  pDataPart = DelOrReplacePart(partCode, nullptr);
  if (pDataPart == nullptr)
    return PdbE_DATA_FILE_NOT_FOUND;

  //Cancel dump task
  if (dumpPartCode_.load() == static_cast<int>(partCode))
  {
    CancelDumpTask();
  }

  pGlbTabCfg->DelFilePart(tabName_.c_str(), partCode);
  while (pDataPart->GetRefCnt() > 0)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
  }

  //同步修改过的页
  do {
    std::unique_lock<std::mutex> syncLock(syncMutex_);
    pDataPart->SyncDirtyPages(true, &dwFile_);
  } while (false);

  uint64_t pageCode = MAKE_DATAPART_MASK(tabCode_, partCode);
  pGlbPagePool->ClearPageForMask(pageCode, PDB_PAGEPOOL_DATAFILE_MASK);
  delete pDataPart;

  return PdbE_OK;
}

PdbErr_t PDBTable::DropPart(uint32_t partCode)
{
  DataPart* pDataPart = nullptr;
  int32_t invalidDays = pGlbSysCfg->GetInsertValidDay();
  int32_t invalidBgDay = DateTime::NowDayCode() - invalidDays; 

  if (partCode >= static_cast<uint32_t>(invalidBgDay))
    return PdbE_DATA_FILE_IN_ACTIVE;

  pDataPart = DelOrReplacePart(partCode, nullptr);
  if (pDataPart == nullptr)
    return PdbE_DATA_FILE_NOT_FOUND;

  //Cancel dump task
  if (dumpPartCode_.load() == static_cast<int>(partCode))
  {
    CancelDumpTask();
  }

  std::string idxPath = pDataPart->GetIdxPath();
  std::string dataPath = pDataPart->GetDataPath();
  pGlbTabCfg->DelFilePart(tabName_.c_str(), partCode);
  while (pDataPart->GetRefCnt() > 0)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
  }

  pDataPart->AbandonDirtyPages();
  uint64_t pageCode = MAKE_DATAPART_MASK(tabCode_, partCode);
  pGlbPagePool->ClearPageForMask(pageCode, PDB_PAGEPOOL_DATAFILE_MASK);
  delete pDataPart;
  FileTool::RemoveFile(idxPath.c_str());
  FileTool::RemoveFile(dataPath.c_str());
  return PdbE_OK;
}

void PDBTable::CancelDumpTask()
{
  if (dumpPartCode_.load() >= 0)
  {
    glbCancelCompTask = true;
    while (dumpPartCode_.load() >= 0)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
  }
}

PdbErr_t PDBTable::DropTable()
{
  //删除所有数据文件
  do {
    std::unique_lock<std::mutex> partLock(partMutex_);
    for (auto partIt = partVec_.begin(); partIt != partVec_.end(); partIt++)
    {
      //抛弃所有未同步的数据页
      (*partIt)->AbandonDirtyPages();

      std::string idxPath = (*partIt)->GetIdxPath();
      std::string dataPath = (*partIt)->GetDataPath();
      delete (*partIt);
      FileTool::RemoveFile(idxPath.c_str());
      FileTool::RemoveFile(dataPath.c_str());
    }

    partVec_.clear();
  } while (false);

  //删除二次写文件
  dwFile_.Close();
  FileTool::RemoveFile(dwPath_.c_str());
  //删除设备文件
  devTable_.Close();
  FileTool::RemoveFile(devPath_.c_str());
  return PdbE_OK;
}

#define INSERT_REC_ERROR_OCCUR \
  includeErr = true;\
  if (errBreak) break;\
  resultList.push_back(retVal); \
  continue\

PdbErr_t PDBTable::Insert(InsertSql* pInsertSql,
  bool errBreak, std::list<PdbErr_t>& resultList)
{
  PdbErr_t retVal = PdbE_OK;
  bool includeErr = false;
  RefUtil partRef;
  RefUtil tabInfoRef;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);
  retVal = pInsertSql->InitTableInfo(pTabInfo);
  if (retVal != PdbE_OK)
    return retVal;

  uint32_t tabMetaCode = pTabInfo->GetMetaCode();

  std::string recBuf;
  int64_t invalidDays = pGlbSysCfg->GetInsertValidDay();
  int64_t invalidTstampBg = (DateTime::NowDayCode() - invalidDays) * DateTime::MicrosecondPerDay;
  int64_t invalidTstampEd = invalidTstampBg + invalidDays * DateTime::MicrosecondPerDay * 2 + DateTime::MicrosecondPerDay;

  int64_t devId = 0;
  int64_t tstamp = 0;
  DataPart* pDataPart = nullptr;
  int curPartDay = -1;
  int curRecDay = 0;
  while (!pInsertSql->IsEnd() && glbRunning)
  {
    retVal = pInsertSql->GetNextRecBinary(recBuf, devId, tstamp);
    if (retVal != PdbE_OK)
    {
      LOG_DEBUG("failed to insert record, get next record error {}", retVal);
      INSERT_REC_ERROR_OCCUR;
    }

    //判断时间戳是否有效
    if (tstamp < invalidTstampBg || tstamp > invalidTstampEd)
    {
      retVal = PdbE_INVALID_TSTAMP_VAL;
      INSERT_REC_ERROR_OCCUR;
    }

    //判断设备是否存在
    retVal = devTable_.DevExist(devId);
    if (retVal != PdbE_OK)
    {
      INSERT_REC_ERROR_OCCUR;
    }

    curRecDay = static_cast<int32_t>(tstamp / DateTime::MicrosecondPerDay);
    if (curPartDay != curRecDay)
    {
      retVal = GetOrCreateNormalPart(curRecDay, &partRef);
      if (retVal != PdbE_OK)
      {
        INSERT_REC_ERROR_OCCUR;
      }

      curPartDay = curRecDay;
      pDataPart = partRef.GetObj<DataPart>();
    }

    if (pDataPart->GetMetaCode() != tabMetaCode)
    {
      retVal = PdbE_TABLE_FIELD_MISMATCH;
      INSERT_REC_ERROR_OCCUR;
    }

    pGlbCommitLog->AppendRec(tabCrc_, tabMetaCode, CMTLOG_TYPE_INSERT_CLIENT,
      devId, recBuf.data(), recBuf.size());

    retVal = pDataPart->InsertRec(tabMetaCode, devId, tstamp, true, recBuf.data(), recBuf.size());
    if (retVal != PdbE_OK)
    {
      LOG_DEBUG("failed to insert record {}", retVal);
      INSERT_REC_ERROR_OCCUR;
    }

    resultList.push_back(PdbE_OK);
  }

  if (errBreak)
    return retVal;

  if (includeErr)
    return PdbE_INSERT_PART_ERROR;

  return PdbE_OK;
}


PdbErr_t PDBTable::InsertByDataLog(uint32_t metaCode, int64_t devId, 
  int64_t tstamp, const char* pRec, size_t recLen)
{
  RefUtil partRef;
  int32_t recDay = static_cast<int32_t>(tstamp / DateTime::MicrosecondPerDay);
  DataPart* pDataPart = GetDataPart(recDay, &partRef);
  if (pDataPart != nullptr)
    return pDataPart->InsertRec(metaCode, devId, tstamp, true, pRec, recLen);
  
  return PdbE_OK;
}

PdbErr_t PDBTable::InsertByReplicate(std::vector<LogRecInfo>& recVec, size_t beginIdx)
{
  PdbErr_t retVal = PdbE_OK;
  uint64_t tstamp;
  size_t idx = beginIdx;
  size_t cnt = recVec.size();
  RefUtil partRef;
  int32_t tmpCode;
  int32_t partCode = -1;
  DataPart* pDataPart = nullptr;
  int32_t successCnt = 0;
  int32_t errCnt = 0;

  for (; idx < cnt; idx++)
  {
    if (recVec[idx].tabCrc == tabCrc_)
    {
      if (nullptr != Coding::VarintDecode64((recVec[idx].pRec + 2),
        (recVec[idx].pRec + recVec[idx].recLen), &tstamp))
      {
        tmpCode = static_cast<int32_t>(tstamp / DateTime::MicrosecondPerDay);
        if (pDataPart == nullptr || partCode != tmpCode)
        {
          retVal = GetOrCreateNormalPart(tmpCode, &partRef);
          if (retVal != PdbE_OK)
          {
            errCnt++;
            LOG_DEBUG("insert replicate record failed, GetOrCreateNormalPart({}:{}) ", 
              tabName_.c_str(), partCode);
            continue;
          }

          partCode = tmpCode;
          pDataPart = partRef.GetObj<DataPart>();
        }

        retVal = pDataPart->InsertRec(recVec[idx].metaCrc, recVec[idx].devId,
          tstamp, true, recVec[idx].pRec, recVec[idx].recLen);
        if (retVal != PdbE_OK)
        {
          errCnt++;
          LOG_DEBUG("insert replicate record failed, InsertRec({}:{})",
            tabName_.c_str(), partCode);
        }
        else
        {
          successCnt++;
          pGlbCommitLog->AppendRec(tabCrc_, recVec[idx].metaCrc, CMTLOG_TYPE_INSERT_REPLICATE,
            recVec[idx].devId, recVec[idx].pRec, recVec[idx].recLen);
        }
      }
    }
  }

  LOG_DEBUG("insert replicate table ({}) success({}) error({})",
    tabName_.c_str(), successCnt, errCnt);

  return PdbE_OK;
}

PdbErr_t PDBTable::ExecQuery(const QueryParam* pQueryParam,
  std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  bool groupQuery = false;
  if (pQueryParam == nullptr)
    return PdbE_INVALID_PARAM;

  if (pQueryParam->pTagList_ == nullptr)
    return PdbE_SQL_ERROR;

  if (pQueryParam->pGroup_ == nullptr)
  {
    const std::vector<TargetItem>* pTagList = pQueryParam->pTagList_->GetTargetList();
    for (auto tagIt = pTagList->begin(); tagIt != pTagList->end(); tagIt++)
    {
      if (IncludeAggFunction(tagIt->first))
      {
        groupQuery = true;
        break;
      }
    }
    
    if (groupQuery)
    {
      //GROUP ALL
      if (pQueryParam->pOrderBy_ != nullptr)
        return PdbE_SQL_GROUP_ERROR;

      return ExecQueryGroupAll(pQueryParam, resultData, pFieldCnt, pRecordCnt);
    }
    else
    {
      //NONE GROUP
      return ExecQueryRawData(pQueryParam, resultData, pFieldCnt, pRecordCnt);
    }
  }
  else
  {
    if (!pQueryParam->pGroup_->Valid())
      return PdbE_SQL_GROUP_ERROR;

    if (pQueryParam->pOrderBy_ != nullptr)
      return PdbE_SQL_GROUP_ERROR;

    if (pQueryParam->pGroup_->IsDevIdGroup())
    {
      //GROUP BY DEVID
      return ExecQueryGroupDevId(pQueryParam, resultData, pFieldCnt, pRecordCnt);
    }
    else if (pQueryParam->pGroup_->IsTStampGroup())
    {
      //GROUP BY TSTAMP 1M...
      return ExecQueryGroupTstamp(pQueryParam, resultData, pFieldCnt, pRecordCnt);
    }
  }

  return PdbE_SQL_GROUP_ERROR;
}

PdbErr_t PDBTable::ExecQuerySnapshot(const QueryParam* pQueryParam,
  std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabInfoRef;
  uint64_t timeOutTick = DateTime::NowTickCount() + pGlbSysCfg->GetQueryTimeOut() * TicksPerSecond;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);

  if (pQueryParam->pGroup_ != nullptr)
    return PdbE_SQL_GROUP_ERROR;
  if (pQueryParam->pOrderBy_ != nullptr)
    return PdbE_SQL_ERROR;

  QuerySnapshot* pSnapshot = new QuerySnapshot();
  std::list<int64_t> devIdList;

  do {
    retVal = pSnapshot->BuildQuery(pQueryParam, pTabInfo);
    if (retVal != PdbE_OK)
      break;

    retVal = devTable_.QueryDevId(pSnapshot, devIdList);
    if (retVal != PdbE_OK)
      break;

    if (devIdList.empty() || pSnapshot->IsEmptySet())
      break;

    retVal = QuerySnapshotData(devIdList, pSnapshot, timeOutTick);

  } while (false);

  if (retVal == PdbE_OK)
  {
    pSnapshot->GetResult(resultData, pFieldCnt, pRecordCnt);
  }

  if (pSnapshot != nullptr)
    delete pSnapshot;

  return retVal;
}


PdbErr_t PDBTable::AddDev(int64_t devId, PdbStr devName, PdbStr expand)
{
  return devTable_.AddDev(devId, devName, expand);
}

PdbErr_t PDBTable::FlushDev()
{
  devTable_.Flush();
  return PdbE_OK;
}

PdbErr_t PDBTable::DelDev(const ConditionFilter* pCondition)
{
  return devTable_.DelDev(tabName_, pCondition);
}

PdbErr_t PDBTable::QueryDev(IQuery* pQuery)
{
  return devTable_.QueryDevInfo(tabName_, pQuery);
}

PdbErr_t PDBTable::DumpPartToComp()
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil partRef;
  DataPart* pDataPart = nullptr;
  int32_t invalidDays = pGlbSysCfg->GetInsertValidDay();
  int32_t partCode = DateTime::NowDayCode() - invalidDays;

  std::string compDateStr;
  std::string compIdxPath;
  std::string compDataPath;

  while (!glbCancelCompTask && glbRunning)
  {
    GetDataPartEqualOrLess(partCode, false, &partRef);
    pDataPart = partRef.GetObj<DataPart>();
    if (pDataPart == nullptr)
      break;

    partCode = pDataPart->GetPartCode();
    if (pDataPart->IsNormalPart() && pDataPart->IsPartReadOnly())
    {
      do {
        LOG_INFO("start to compress for data file ({})", pDataPart->GetDataPath().c_str());
        dumpPartCode_.store(partCode);

        retVal = BuildPartPath(partCode, false, true, compDateStr, compIdxPath, compDataPath);
        if (retVal != PdbE_OK)
        {
          LOG_ERROR("failed to build compress datapart path for table ({}), {}", tabName_.c_str(), retVal);
          break;
        }

        if (FileTool::FileExists(compDataPath.c_str()))
        {
          LOG_INFO("failed to dump table ({}) datapart({}) to compress, file exists",
            tabName_.c_str(), compDateStr.c_str());
          break;
        }

        retVal = pDataPart->DumpToCompPart(compDataPath.c_str());
        if (retVal != PdbE_OK)
        {
          LOG_INFO("failed to dump table ({}) datapart({}) to compress, ret {}",
            tabName_.c_str(), compDateStr.c_str(), retVal);
          break;
        }
        
        retVal = pGlbTabCfg->SetFilePart(tabName_.c_str(), compDateStr.c_str(), partCode, PDB_PART_TYPE_COMPRESS_VAL);
        if (retVal != PdbE_OK)
        {
          LOG_INFO("failed to dump table ({}) datapart ({}) to compress, update table config error {}",
            tabName_.c_str(), compDateStr.c_str(), retVal);
          break;
        }

        CompDataPart* pCompDataPart = new CompDataPart();
        retVal = pCompDataPart->Open(partCode, compDataPath.c_str());
        if (retVal != PdbE_OK)
        {
          delete pCompDataPart;
          break;
        }

        DelOrReplacePart(partCode, pCompDataPart);
        partRef.Attach(nullptr);

        while (pDataPart->GetRefCnt() > 0)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }

        std::string normalIdxPath = pDataPart->GetIdxPath();
        std::string normalDataPath = pDataPart->GetDataPath();
        delete pDataPart;
        FileTool::RemoveFile(normalIdxPath.c_str());
        FileTool::RemoveFile(normalDataPath.c_str());
        LOG_INFO("successful to compress data file ({}) to ({})", 
          normalDataPath.c_str(), compDataPath.c_str());

      } while (false);

      dumpPartCode_.store(-1);
    }

    if (retVal != PdbE_OK)
      return retVal;
  }

  return PdbE_OK;
}

PdbErr_t PDBTable::UnMapCompressData()
{
  std::unique_lock<std::mutex> partLock(partMutex_);
  for (auto partIt = partVec_.begin(); partIt != partVec_.end(); partIt++)
  {
    (*partIt)->UnMap();
  }

  return PdbE_OK;
}

//同步脏数据页
PdbErr_t PDBTable::SyncDirtyPages(bool syncAll)
{
  RefUtil partRef;
  DataPart* pDataPart = nullptr;
  int curPartCode = 0;
  PdbErr_t retVal = PdbE_OK;

  int32_t invalidDays = pGlbSysCfg->GetInsertValidDay();
  int32_t readOnlyPartCode = DateTime::NowDayCode() - invalidDays;

  std::unique_lock<std::mutex> syncLock(syncMutex_);

  GetDataPartEqualOrGreat(0, true, &partRef);
  pDataPart = partRef.GetObj<DataPart>();
  while (pDataPart != nullptr)
  {
    curPartCode = pDataPart->GetPartCode();

    if (pDataPart->IsNormalPart() && (!pDataPart->IsPartReadOnly()))
    {
      retVal = pDataPart->SyncDirtyPages(syncAll, &dwFile_);

      if (retVal == PdbE_OK && syncAll && curPartCode < readOnlyPartCode)
        pDataPart->SwitchToReadOnly();
    }

    GetDataPartEqualOrGreat(curPartCode, false, &partRef);
    pDataPart = partRef.GetObj<DataPart>();
  }

  return PdbE_OK;
}

size_t PDBTable::GetDirtyPageCnt()
{
  RefUtil partRef;
  DataPart* pDataPart = nullptr;
  size_t dirtyPageCnt = 0;

  int32_t invalidDays = pGlbSysCfg->GetInsertValidDay();
  int32_t curPartCode = DateTime::NowDayCode() - invalidDays;

  GetDataPartEqualOrGreat(curPartCode, true, &partRef);
  pDataPart = partRef.GetObj<DataPart>();
  while (pDataPart != nullptr)
  {
    dirtyPageCnt += pDataPart->GetDirtyPageCnt();

    curPartCode = pDataPart->GetPartCode();
    GetDataPartEqualOrGreat(curPartCode, false, &partRef);
    pDataPart = partRef.GetObj<DataPart>();
  }

  return dirtyPageCnt;
}

PdbErr_t PDBTable::ExecQueryGroupAll(const QueryParam* pQueryParam,
  std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabInfoRef;
  uint64_t timeOutTick = DateTime::NowTickCount() + pGlbSysCfg->GetQueryTimeOut() * TicksPerSecond;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);
  QueryGroupAll* pGroupQuery = new QueryGroupAll();
  std::list<int64_t> devIdList;

  do {
    retVal = pGroupQuery->BuildQuery(pQueryParam, pTabInfo);
    if (retVal != PdbE_OK)
      break;

    retVal = devTable_.QueryDevId(pGroupQuery, devIdList);
    if (retVal != PdbE_OK)
      break;

    if (devIdList.empty() || pGroupQuery->IsEmptySet())
      break;

    if (pGroupQuery->IsQueryFirst())
    {
      retVal = QueryFirst(devIdList, pGroupQuery, timeOutTick);
    }
    else if (pGroupQuery->IsQueryLast())
    {
      retVal = QueryLast(devIdList, pGroupQuery, timeOutTick);
    }
    else
    {
      retVal = QueryAsc(devIdList, pGroupQuery, timeOutTick);
    }
  } while (false);

  if (retVal == PdbE_OK)
  {
    retVal = pGroupQuery->GetResult(resultData, pFieldCnt, pRecordCnt);
  }

  if (pGroupQuery != nullptr)
    delete pGroupQuery;

  return retVal;
}

PdbErr_t PDBTable::ExecQueryGroupDevId(const QueryParam* pQueryParam,
  std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabInfoRef;
  uint64_t timeOutTick = DateTime::NowTickCount() + pGlbSysCfg->GetQueryTimeOut() * TicksPerSecond;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);
  QueryGroupDevID* pGroupQuery = new QueryGroupDevID();
  std::list<int64_t> devIdList;

  do {
    retVal = pGroupQuery->BuildQuery(pQueryParam, pTabInfo);
    if (retVal != PdbE_OK)
      break;

    retVal = devTable_.QueryDevId(pGroupQuery, devIdList, 
      pGroupQuery->GetQueryOffset(), pGroupQuery->GetQueryRecord());
    if (retVal != PdbE_OK)
      break;

    retVal = pGroupQuery->InitGroupDevID(devIdList);
    if (retVal != PdbE_OK)
      break;

    if (devIdList.empty() || pGroupQuery->IsEmptySet())
      break;

    if (pGroupQuery->IsQueryFirst())
    {
      retVal = QueryFirst(devIdList, pGroupQuery, timeOutTick);
    }
    else if (pGroupQuery->IsQueryLast())
    {
      retVal = QueryLast(devIdList, pGroupQuery, timeOutTick);
    }
    else
    {
      retVal = QueryAsc(devIdList, pGroupQuery, timeOutTick);
    }
  } while (false);

  if (retVal == PdbE_OK)
  {
    retVal = pGroupQuery->GetResult(resultData, pFieldCnt, pRecordCnt);
  }

  if (pGroupQuery != nullptr)
    delete pGroupQuery;

  return retVal;
}

PdbErr_t PDBTable::ExecQueryGroupTstamp(const QueryParam* pQueryParam,
  std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabInfoRef;
  uint64_t timeOutTick = DateTime::NowTickCount() + pGlbSysCfg->GetQueryTimeOut() * TicksPerSecond;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);
  QueryGroupTstamp* pGroupQuery = new QueryGroupTstamp();
  QueryRaw* pRawQuery = new QueryRaw();
  std::list<int64_t> devIdList;

  do {
    retVal = pGroupQuery->BuildQuery(pQueryParam, pTabInfo);
    if (retVal != PdbE_OK)
      break;

    if (pQueryParam->pGroup_ == nullptr)
    {
      retVal = PdbE_SQL_GROUP_ERROR;
      break;
    }

    if ((!pQueryParam->pGroup_->Valid()) || (!pQueryParam->pGroup_->IsTStampGroup()))
    {
      retVal = PdbE_SQL_GROUP_ERROR;
      break;
    }

    retVal = devTable_.QueryDevId(pGroupQuery, devIdList);
    if (retVal != PdbE_OK)
      break;

    if (devIdList.empty() || pGroupQuery->IsEmptySet())
      break;

    if (pGroupQuery->IsQueryFirst())
    {
      retVal = QueryFirst(devIdList, pGroupQuery, timeOutTick);
    }
    else if (pGroupQuery->IsQueryLast())
    {
      retVal = QueryLast(devIdList, pGroupQuery, timeOutTick);
    }
    else
    {
      retVal = QueryAsc(devIdList, pGroupQuery, timeOutTick);
    }
  } while (false);

  if (retVal == PdbE_OK)
  {
    retVal = pGroupQuery->GetResult(resultData, pFieldCnt, pRecordCnt);
  }

  if (pGroupQuery != nullptr)
    delete pGroupQuery;

  if (pRawQuery != nullptr)
    delete pRawQuery;

  return retVal;
}

PdbErr_t PDBTable::ExecQueryRawData(const QueryParam* pQueryParam,
  std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  PdbErr_t retVal = PdbE_OK;
  bool isAsc = true;
  RefUtil tabInfoRef;
  uint64_t timeOutTick = DateTime::NowTickCount() + pGlbSysCfg->GetQueryTimeOut() * TicksPerSecond;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);
  QueryRaw* pRawQuery = new QueryRaw();
  std::list<int64_t> devIdList;

  do {
    if (pQueryParam->pOrderBy_ != nullptr)
    {
      if (!pQueryParam->pOrderBy_->Valid())
      {
        retVal = PdbE_SQL_ERROR;
      }

      isAsc = pQueryParam->pOrderBy_->IsASC();
    }
    
    retVal = pRawQuery->BuildQuery(pQueryParam, pTabInfo);
    if (retVal != PdbE_OK)
      break;

    retVal = devTable_.QueryDevId(pRawQuery, devIdList);
    if (retVal != PdbE_OK)
      break;

    if (devIdList.empty() || pRawQuery->IsEmptySet())
      break;

    if (isAsc)
    {
      retVal = QueryAsc(devIdList, pRawQuery, timeOutTick);
    }
    else
    {
      retVal = QueryDesc(devIdList, pRawQuery, timeOutTick);
    }

  } while (false);

  if (retVal == PdbE_OK)
  {
    pRawQuery->GetResult(resultData, pFieldCnt, pRecordCnt);
  }

  if (pRawQuery != nullptr)
    delete pRawQuery;

  return retVal;
}

PdbErr_t PDBTable::QueryLast(std::list<int64_t>& devIdList, IQuery* pQuery, uint64_t queryTimeOut)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil partRef;
  RefUtil tabInfoRef;
  int minQueryDay, maxQueryDay;
  int64_t minTs, maxTs;

  pQuery->GetTstampRange(&minTs, &maxTs);
  minQueryDay = static_cast<int>(minTs / DateTime::MicrosecondPerDay);
  maxQueryDay = static_cast<int>(maxTs / DateTime::MicrosecondPerDay);
  int curDay = maxQueryDay;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);

  GetDataPartEqualOrLess(curDay, true, &partRef);
  DataPart* pDataPart = partRef.GetObj<DataPart>();
  while (pDataPart != nullptr)
  {
    curDay = pDataPart->GetPartCode();
    if (minQueryDay > curDay)
      break;

    retVal = pDataPart->QueryLast(devIdList, pTabInfo, pQuery, queryTimeOut);
    if (retVal == PdbE_RESULT_FULL)
      return PdbE_OK;

    if (retVal != PdbE_OK)
      return retVal;

    if (devIdList.empty())
      break;

    GetDataPartEqualOrLess(curDay, false, &partRef);
    pDataPart = partRef.GetObj<DataPart>();
  }

  return PdbE_OK;
}

PdbErr_t PDBTable::QueryFirst(std::list<int64_t>& devIdList, IQuery* pQuery, uint64_t queryTimeOut)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil partRef;
  RefUtil tabInfoRef;
  int minQueryDay, maxQueryDay;
  int64_t minTs, maxTs;

  pQuery->GetTstampRange(&minTs, &maxTs);
  minQueryDay = static_cast<int>(minTs / DateTime::MicrosecondPerDay);
  maxQueryDay = static_cast<int>(maxTs / DateTime::MicrosecondPerDay);
  int curDay = minQueryDay;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);

  GetDataPartEqualOrGreat(curDay, true, &partRef);
  DataPart* pDataPart = partRef.GetObj<DataPart>();
  while (pDataPart != nullptr)
  {
    curDay = pDataPart->GetPartCode();
    if (maxQueryDay < curDay)
      break;

    retVal = pDataPart->QueryFirst(devIdList, pTabInfo, pQuery, queryTimeOut);
    if (retVal == PdbE_RESULT_FULL)
      return PdbE_OK;

    if (retVal != PdbE_OK)
      return retVal;

    if (devIdList.empty())
      break;

    GetDataPartEqualOrGreat(curDay, false, &partRef);
    pDataPart = partRef.GetObj<DataPart>();
  }

  return PdbE_OK;
}

PdbErr_t PDBTable::QueryAsc(std::list<int64_t>& devIdList, IQuery* pQuery, uint64_t queryTimeOut)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil partRef;
  RefUtil tabInfoRef;
  int minQueryDay, maxQueryDay;
  int64_t minTs, maxTs;

  pQuery->GetTstampRange(&minTs, &maxTs);
  minQueryDay = static_cast<int>(minTs / DateTime::MicrosecondPerDay);
  maxQueryDay = static_cast<int>(maxTs / DateTime::MicrosecondPerDay);
  int curDay = minQueryDay;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);

  GetDataPartEqualOrGreat(curDay, true, &partRef);
  DataPart* pDataPart = partRef.GetObj<DataPart>();
  while (pDataPart != nullptr)
  {
    curDay = pDataPart->GetPartCode();
    if (maxQueryDay < curDay)
      break;

    retVal = pDataPart->QueryAsc(devIdList, pTabInfo, pQuery, queryTimeOut);
    if (retVal == PdbE_RESULT_FULL)
      return PdbE_OK;

    if (retVal != PdbE_OK)
      return retVal;

    if (pQuery->GetIsFullFlag())
      return PdbE_OK;

    GetDataPartEqualOrGreat(curDay, false, &partRef);
    pDataPart = partRef.GetObj<DataPart>();
  }

  return PdbE_OK;
}

PdbErr_t PDBTable::QueryDesc(std::list<int64_t>& devIdList, IQuery* pQuery, uint64_t queryTimeOut)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil partRef;
  RefUtil tabInfoRef;
  int minQueryDay, maxQueryDay;
  int64_t minTs, maxTs;

  pQuery->GetTstampRange(&minTs, &maxTs);
  minQueryDay = static_cast<int>(minTs / DateTime::MicrosecondPerDay);
  maxQueryDay = static_cast<int>(maxTs / DateTime::MicrosecondPerDay);
  int curDay = maxQueryDay;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);

  GetDataPartEqualOrLess(curDay, true, &partRef);
  DataPart* pDataPart = partRef.GetObj<DataPart>();
  while (pDataPart != nullptr)
  {
    curDay = pDataPart->GetPartCode();
    if (minQueryDay > curDay)
      break;

    retVal = pDataPart->QueryDesc(devIdList, pTabInfo, pQuery, queryTimeOut);
    if (retVal == PdbE_RESULT_FULL)
      return PdbE_OK;

    if (retVal != PdbE_OK)
      return retVal;

    if (pQuery->GetIsFullFlag())
      return PdbE_OK;

    GetDataPartEqualOrLess(curDay, false, &partRef);
    pDataPart = partRef.GetObj<DataPart>();
  }

  return PdbE_OK;
}

PdbErr_t PDBTable::QuerySnapshotData(std::list<int64_t>& devIdList, IQuery* pQuery, uint64_t queryTimeOut)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil partRef;
  RefUtil tabInfoRef;
  int minQueryDay, maxQueryDay;
  int64_t minTs, maxTs;

  pQuery->GetTstampRange(&minTs, &maxTs);
  minQueryDay = static_cast<int>(minTs / DateTime::MicrosecondPerDay);
  maxQueryDay = static_cast<int>(maxTs / DateTime::MicrosecondPerDay);
  int curDay = maxQueryDay;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);

  GetDataPartEqualOrLess(curDay, true, &partRef);
  DataPart* pDataPart = partRef.GetObj<DataPart>();
  while (pDataPart != nullptr)
  {
    curDay = pDataPart->GetPartCode();
    if (minQueryDay > curDay)
      break;

    if (devIdList.empty())
      break;

    if (*devIdList.begin() > pQuery->GetMaxDevId())
      break;

    retVal = pDataPart->QuerySnapshot(devIdList, pTabInfo, pQuery, queryTimeOut);
    if (retVal == PdbE_RESULT_FULL)
      return PdbE_OK;

    if (retVal != PdbE_OK)
      return retVal;

    GetDataPartEqualOrLess(curDay, false, &partRef);
    pDataPart = partRef.GetObj<DataPart>();
  }

  return PdbE_OK;
}


void PDBTable::GetDataPartEqualOrGreat(uint32_t partCode, bool includeEqual, RefUtil* pPartRef)
{
  std::unique_lock<std::mutex> partLock(partMutex_);
  pPartRef->Attach(nullptr);
  int idx = _GetDataPartPos(partCode);
  if (idx < 0)
    return;

  uint32_t tmpPartCode = partVec_[idx]->GetPartCode();
  if (tmpPartCode == partCode && includeEqual)
  {
    pPartRef->Attach(partVec_[idx]);
    return;
  }

  if (tmpPartCode > partCode)
  {
    pPartRef->Attach(partVec_[idx]);
    return;
  }

  idx++;
  if (idx < static_cast<int>(partVec_.size()))
  {
    pPartRef->Attach(partVec_[idx]);
    return;
  }
}

void PDBTable::GetDataPartEqualOrLess(uint32_t partCode, bool includeEqual, RefUtil* pPartRef)
{
  std::unique_lock<std::mutex> partLock(partMutex_);
  pPartRef->Attach(nullptr);
  int idx = _GetDataPartPos(partCode);
  if (idx < 0)
    return;

  uint32_t tmpPartCode = partVec_[idx]->GetPartCode();
  if (tmpPartCode == partCode && includeEqual)
  {
    pPartRef->Attach(partVec_[idx]);
    return;
  }

  if (tmpPartCode < partCode)
  {
    pPartRef->Attach(partVec_[idx]);
    return;
  }

  idx--;
  if (idx >= 0)
  {
    pPartRef->Attach(partVec_[idx]);
    return;
  }
}

DataPart* PDBTable::GetDataPart(uint32_t partCode, RefUtil* pPartRef)
{
  std::unique_lock<std::mutex> partLock(partMutex_);
  pPartRef->Attach(nullptr);
  int idx = _GetDataPartPos(partCode);
  if (idx < 0)
    return nullptr;

  if (partVec_[idx]->GetPartCode() == partCode)
  {
    pPartRef->Attach(partVec_[idx]);
    return partVec_[idx];
  }

  return nullptr;
}

int PDBTable::_GetDataPartPos(uint32_t partCode)
{
  int partCnt = static_cast<int>(partVec_.size());
  int lwr = 0;
  int upr = partCnt - 1;
  int idx = 0;
  uint32_t tmpPartCode = 0;

  if (partVec_.empty())
    return -1;

  while (lwr <= upr)
  {
    idx = (lwr + upr) / 2;
    tmpPartCode = partVec_[idx]->GetPartCode();
    if (tmpPartCode == partCode)
      break;
    else if (tmpPartCode < partCode)
      lwr = idx + 1;
    else
      upr = idx - 1;
  }

  return idx;
}

PdbErr_t PDBTable::GetOrCreateNormalPart(uint32_t partCode, RefUtil* pPartRef)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabInfoRef;
  NormalDataPart* pDataPart = nullptr;
  TableInfo* pTabInfo = GetTableInfo(&tabInfoRef);
  std::string partDateStr;
  std::string partIdxPath;
  std::string partDataPath;

  std::unique_lock<std::mutex> partLock(partMutex_);
  pPartRef->Attach(nullptr);
  int idx = _GetDataPartPos(partCode);
  if (idx >= 0 && partVec_[idx]->GetPartCode() == partCode)
  {
    if (!partVec_[idx]->IsNormalPart())
      return PdbE_FILE_READONLY;

    pPartRef->Attach(partVec_[idx]);
    return PdbE_OK;
  }

  // Create New Normal Part
  retVal = BuildPartPath(partCode, true, true, partDateStr, partIdxPath, partDataPath);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("failed to build datapart path for table ({}), {}", tabName_.c_str(), retVal);
    return retVal;
  }

  if (FileTool::FileExists(partIdxPath.c_str()) || FileTool::FileExists(partDataPath.c_str()))
  {
    return PdbE_FILE_EXIST;
  }

  do {
    retVal = NormalDataPart::Create(partIdxPath.c_str(), partDataPath.c_str(), pTabInfo, partCode);
    if (retVal != PdbE_OK)
      break;

    pDataPart = new NormalDataPart();
    if (pDataPart == nullptr)
    {
      retVal = PdbE_NOMEM;
      break;
    }

    retVal = pDataPart->Open(tabCode_, partCode, partIdxPath.c_str(), partDataPath.c_str());
    if (retVal != PdbE_OK)
      break;

    pGlbTabCfg->AddFilePart(tabName_.c_str(), partDateStr.c_str(), partCode, PDB_PART_TYPE_NORMAL_VAL);
    partVec_.push_back(pDataPart);
    std::sort(partVec_.begin(), partVec_.end(), PartComp);
    pPartRef->Attach(pDataPart);

  } while (false);

  if (retVal != PdbE_OK)
  {
    if (pDataPart != nullptr)
      delete pDataPart;

    FileTool::RemoveFile(partIdxPath.c_str());
    FileTool::RemoveFile(partDataPath.c_str());
  }

  return retVal;
}

PdbErr_t PDBTable::BuildPartPath(uint32_t partCode, bool isNormal, bool createParent,
  std::string& partDateStr, std::string& idxPath, std::string& dataPath)
{
  char pathBuf[MAX_PATH];
  int partYear = 0;
  int partMonth = 0;
  int partDay = 0;

  DateTime dtPart((partCode * DateTime::MicrosecondPerDay));
  dtPart.GetDatePart(&partYear, &partMonth, &partDay);

  sprintf(pathBuf, "%04d-%02d-%02d", partYear, partMonth, partDay);
  partDateStr = pathBuf;

  std::string sysDBPath;
  if (isNormal)
    sysDBPath = pGlbSysCfg->GetNormalDataPath();
  else
    sysDBPath = pGlbSysCfg->GetCompressDataPath();

  sprintf(pathBuf, "%s/%s/%04d/%02d/%s_%04d%02d%02d", sysDBPath.c_str(), tabName_.c_str(),
    partYear, partMonth, tabName_.c_str(), partYear, partMonth, partDay);

  idxPath = pathBuf;
  dataPath = pathBuf;

  if (isNormal)
  {
    idxPath += NORMAL_IDX_FILE_EXTEND;
    dataPath += NORMAL_DATA_FILE_EXTEND;
  }
  else
  {
    dataPath += COMPRESS_DATA_FILE_EXTEND;
  }

  if (createParent)
    return FileTool::MakeParentPath(pathBuf);

  return PdbE_OK;
}

DataPart* PDBTable::DelOrReplacePart(uint32_t partCode, DataPart* pNewPart)
{
  DataPart* pDataPart = nullptr;
  std::unique_lock<std::mutex> partLock(partMutex_);
  for (auto partIt = partVec_.begin(); partIt != partVec_.end(); partIt++)
  {
    if ((*partIt)->GetPartCode() == partCode)
    {
      pDataPart = (*partIt);
      partVec_.erase(partIt);
      break;
    }
  }

  if (pNewPart != nullptr)
  {
    partVec_.push_back(pNewPart);
    std::sort(partVec_.begin(), partVec_.end(), PartComp);
  }

  return pDataPart;
}
