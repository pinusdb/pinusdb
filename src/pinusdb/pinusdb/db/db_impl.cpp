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

#include "db/db_impl.h"

#include <algorithm>
#include <vector>
#include <condition_variable>
#include "boost/filesystem.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/foreach.hpp"

#include "pdb_error.h"
#include "server/table_config.h"
#include "db/page_pool.h"

#include "expr/expr_item.h"
#include "expr/column_item.h"
#include "expr/parse.h"
#include "query/result_filter.h"
#include "util/string_tool.h"
#include "table/db_obj.h"

#include "util/log_util.h"
#include "util/coding.h"

#include "server/user_config.h"
#include "util/date_time.h"
#include "util/performance_counter.h"
#include "server/event_handle.h"
#include "global_variable.h"
#include "sys_table_schema.h"

#include "util/dbg.h"

DBImpl* DBImpl::dbImpl_ = nullptr;

DBImpl::DBImpl()
{
  pSyncTask_ = nullptr;
  pRepTask_ = nullptr;
  pCompTask_ = nullptr;
  isInit_ = false;
  running_ = true;
}

DBImpl::~DBImpl()
{
}

DBImpl* DBImpl::GetInstance()
{
  if (dbImpl_ == nullptr)
  {
    dbImpl_ = new DBImpl();
  }
  return dbImpl_;
}

PdbErr_t DBImpl::Start()
{
  PdbErr_t retVal = PdbE_OK;
  //打开表
  const std::vector<TableItem*>& tabVec = pGlbTabCfg->GetTables();
  for (auto tabIt = tabVec.begin(); tabIt != tabVec.end(); tabIt++)
  {
    std::string tabName = (*tabIt)->GetTabName();
    retVal = pGlbTableSet->OpenTable(tabName.c_str());
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("faield to open table({}), err: {}", (*tabIt)->GetTabName().c_str(), retVal);
      return retVal;
    }

    const std::vector<PartItem>& partVec = (*tabIt)->GetPartVec();
    for (auto partIt = partVec.begin(); partIt != partVec.end(); partIt++)
    {
      int32_t partType = partIt->GetPartType();
      int32_t partCode = partIt->GetPartDate();
      std::string partDateStr = partIt->GetPartDateStr();

      if (partType == PDB_PART_TYPE_NORMAL_VAL
        || partType == PDB_PART_TYPE_COMPRESS_VAL)
      {
        retVal = pGlbTableSet->OpenDataPart(tabName.c_str(), partCode, (partType == PDB_PART_TYPE_NORMAL_VAL));
        if (retVal != PdbE_OK)
          return retVal;
      }
      else
      {
        LOG_ERROR("failed to open table({}) datapart({}), not supported file type: {}", 
          tabName.c_str(), partDateStr.c_str(), partType);
        return PdbE_TABLE_CFG_ERROR;
      }
    }

    //恢复DoubleWrite内容
    retVal = pGlbTableSet->RecoverDW(tabName.c_str());
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to recover table ({}) double write file, err: {}", tabName.c_str(), retVal);
      return retVal;
    }
  }

  //恢复数据日志内容
  retVal = RecoverDataLog();
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  pSyncTask_ = new std::thread(&DBImpl::SyncTask, this);
  pCompTask_ = new std::thread(&DBImpl::CompressTask, this);

  isInit_ = true;
  return PdbE_OK;
}

void DBImpl::StopTask()
{
  running_ = false;
  glbCancelCompTask = true;
  pGlbTableSet->Stop();
}

void DBImpl::Stop()
{
  running_ = false;
  glbCancelCompTask = true;
  stopVariable_.notify_all();
  pCompTask_->join();
  pSyncTask_->join();
  //CloseAllTable 会同步所有的数据页
  pGlbTableSet->CloseAllTable();
  //同步所有数据页后，如果只是单机运行，删除所有日志文件
  pGlbCommitLog->Shutdown();
}

void DBImpl::SyncTask()
{
  PDB_SEH_BEGIN(false);
  _SyncTask();
  PDB_SEH_END("synctask", return);
}

void DBImpl::_SyncTask()
{
  PdbErr_t retVal = PdbE_OK;
  uint32_t syncFileCode = 1;
  uint64_t curLogPos = 0;
  int64_t dbTick = 0;
  int64_t syncAllTick = 0;

  while (running_)
  {
    do {
      std::unique_lock<std::mutex> stopLock(stopMutex_);
      stopVariable_.wait_for(stopLock, std::chrono::seconds(1));
    } while (false);

    if (!running_)
      break;

    dbTick++;
    if ((dbTick - syncAllTick) >= 1800)
    {
      //距离上次全量刷盘超过半小时，启动全量刷盘
      LOG_INFO("timeout to sync all page cache to datafile");
      pGlbTableSet->SyncDirtyPages(true);
      LOG_INFO("sync all page cache to datafile finished");
      syncAllTick = dbTick;
    }
    else
    {
      //每秒判断是否生成了新的日志文件，若有新的日志文件则需要刷盘
      size_t dirtyPercent = pGlbTableSet->GetDirtyPagePercent();
      pGlbCommitLog->GetCurLogPos(&curLogPos);
      if (static_cast<uint32_t>(curLogPos / DATA_LOG_FILE_SIZE) > syncFileCode)
      {
        LOG_INFO("create new data log file, sync all page cache to datafile");
        pGlbTableSet->SyncDirtyPages(true);
        syncFileCode = static_cast<uint32_t>(curLogPos / DATA_LOG_FILE_SIZE);
        LOG_INFO("sync all page cache to datafile finished");
        syncAllTick = dbTick;
      }
      else if (dirtyPercent >= 80)
      {
        //判断是否脏页太多，需要刷盘
        LOG_INFO("dirty page percent ({}%), sync all page cache to datafile", dirtyPercent);
        pGlbTableSet->SyncDirtyPages(true);
        LOG_INFO("sync all page cache to datafile finished");
        syncAllTick = dbTick;
      }
      else
      {
        pGlbTableSet->SyncDirtyPages(false);
      }
    }
  }
}

void DBImpl::CompressTask()
{
  PDB_SEH_BEGIN(false);
  _CompressTask();
  PDB_SEH_END("compresstask", return);
}

void DBImpl::_CompressTask()
{
  while (running_)
  {
    do {
      std::unique_lock<std::mutex> stopLock(stopMutex_);
      stopVariable_.wait_for(stopLock, std::chrono::seconds(600));
    } while (false);

    if (!running_)
      break;

    if (pGlbSysCfg->GetCompressFlag())
      pGlbTableSet->DumpToCompress();

    if (running_)
      pGlbTableSet->UnMapCompressData();
  }
}

PdbErr_t DBImpl::RecoverDataLog()
{
  PdbErr_t retVal = PdbE_OK;
  uint32_t fieldCrc = 0;
  uint64_t tabCrc = 0;
  int64_t devId = 0;
  uint64_t recTstamp = 0;
  size_t recCnt = 0;
  size_t dataLen = 0;
  size_t recLen = 0;
  TableRef tabRef;
  Arena arena;
  PDBTable* pTab = nullptr;
  uint8_t* pBuf = (uint8_t*)arena.Allocate(PDB_MB_BYTES(8));
  uint8_t* pBufLimit = nullptr;
  uint8_t* pRec = nullptr;

  while (true)
  {
    retVal = pGlbCommitLog->GetRedoData(&tabCrc, &fieldCrc, &recCnt, &dataLen, pBuf);
    if (retVal == PdbE_END_OF_DATALOG)
      return PdbE_OK;

    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to recover datalog, get datalog error, err:{}", retVal);
      return retVal;
    }

    pTab = pGlbTableSet->GetTable(tabCrc, &tabRef);
    if (pTab == nullptr)
    {
      LOG_INFO("failed to recover datalog, table ({}) not found, skip ({}) record data", tabCrc, recCnt);
      continue;
    }

    if (pTab->GetFieldCrc() != fieldCrc)
    {
      LOG_INFO("failed to recover datalog, table ({}) column info mistake, skip ({}) record data", tabCrc, recCnt);
      continue;
    }

    pRec = pBuf;
    pBufLimit = pBuf + dataLen;
    for (size_t i = 0; i < recCnt; i++)
    {
      devId = (int64_t)Coding::FixedDecode64(pRec);
      pRec += sizeof(int64_t);
      recLen = Coding::FixedDecode16(pRec);
      Coding::VarintDecode64((pRec + sizeof(uint16_t)), pBufLimit, &recTstamp);
      
      retVal = pTab->InsertByDataLog(devId, (int64_t)recTstamp, pRec, recLen);
      if (retVal != PdbE_OK)
      {
        LOG_ERROR("failed to recover datalog, insert record error, err:{}", retVal);
        return retVal;
      }

      pRec += recLen;
    }
  }

  return PdbE_OK;
}