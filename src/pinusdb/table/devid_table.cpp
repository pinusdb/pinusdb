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

#include "table/devid_table.h"
#include "pdb.h"
#include "internal.h"
#include "port/os_file.h"
#include "util/log_util.h"
#include "util/string_tool.h"
#include "util/coding.h"

//文件块大小 1M
#define DEVID_FILE_BLOCK_SIZE  (PDB_MB_BYTES(1))

#define DEVID_FILE_TYPE_STR_LEN  16
#define DEVID_FILE_TYPE_STR      "PDB DEV 1"

bool DevIdPosComp(DevIdPos a, DevIdPos b)
{
  return a.devId_ < b.devId_;
}

typedef struct _DevMetaFormat
{
  char headStr_[DEVID_FILE_TYPE_STR_LEN]; //头字符串，当前版本为 PDB DEV 1
  uint32_t fieldCnt_;                     //字段数量
  char padHead_[44];

  FieldInfoFormat fieldRec_[PDB_TABLE_MAX_FIELD_COUNT];

  char padTail_[10428];
  uint32_t crc_;
}DevMetaFormat;

typedef struct _DevItem
{
  int64_t devId_;
  char    devName_[PDB_DEVID_NAME_LEN];
  char    expand_[PDB_DEVID_EXPAND_LEN];
  char    padding_[16];
  uint32_t pos_;
  uint32_t crc_;
}DevItem;

DevIDTable::DevIDTable()
{
  isSortId_ = true;
  isSortInfo_ = true;
}

DevIDTable::~DevIDTable()
{
  Close();
}

PdbErr_t DevIDTable::Create(const char* pDevPath, const TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;
  OSFile devFile;

  Arena arena;
  char* pTmpMeta = arena.Allocate(sizeof(DevMetaFormat));
  DevMetaFormat* pMeta = (DevMetaFormat*)pTmpMeta;

  memset(pTmpMeta, 0, sizeof(DevMetaFormat));
  strncpy(pMeta->headStr_, DEVID_FILE_TYPE_STR, DEVID_FILE_TYPE_STR_LEN);
  pMeta->fieldCnt_ = static_cast<uint32_t>(pTabInfo->GetFieldCnt());

  int32_t fieldType = 0;
  const char* pFieldName = nullptr;

  for (uint32_t i = 0; i < pMeta->fieldCnt_; i++)
  {
    pTabInfo->GetFieldInfo(i, &fieldType);
    pFieldName = pTabInfo->GetFieldName(i);

    strncpy(pMeta->fieldRec_[i].fieldName_, pFieldName, PDB_FILED_NAME_LEN);
    pMeta->fieldRec_[i].fieldType_ = fieldType;
  }

  pMeta->crc_ = StringTool::CRC32(pTmpMeta, (sizeof(DevMetaFormat) - 4));

  retVal = devFile.OpenNew(pDevPath);
  if (retVal != PdbE_OK)
    return retVal;

  do {
    retVal = devFile.GrowTo(DEVID_FILE_BLOCK_SIZE);
    if (retVal != PdbE_OK)
      break;

    retVal = devFile.Write(pTmpMeta, sizeof(DevMetaFormat), 0);
  } while (false);

  devFile.Close();
  if (retVal != PdbE_OK)
  {
    FileTool::RemoveFile(pDevPath);
  }
  return retVal;
}

PdbErr_t DevIDTable::Open(const char* pPath, const char* pTabName, TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  retVal = devIdFile_.Open(pPath, false);
  if (retVal != PdbE_OK)
    return retVal;

  freePosList_.clear();
  uint8_t* pBase = devIdFile_.GetBaseAddr();
  size_t fileSize = devIdFile_.MemMapSize();

  //1. 验证元数据页
  const DevMetaFormat* pMeta = (const DevMetaFormat*)pBase;
  if (strncmp(pMeta->headStr_, DEVID_FILE_TYPE_STR, DEVID_FILE_TYPE_STR_LEN) != 0)
  {
    LOG_ERROR("device file ({}) unknown file type", pPath);
    return PdbE_DEVID_FILE_ERROR;
  }

  //1.1 验证 CRC
  if (pMeta->crc_ != StringTool::CRC32(pBase, (sizeof(DevMetaFormat) - 4)))
  {
    LOG_ERROR("table file ({}) meta block crc error", pPath);
    return PdbE_DEVID_FILE_ERROR;
  }

  if (pMeta->fieldCnt_ <= 2 || pMeta->fieldCnt_ > PDB_TABLE_MAX_FIELD_COUNT)
  {
    LOG_ERROR("table file ({}) field count({}) error", pPath, pMeta->fieldCnt_);
    return PdbE_DEVID_FILE_ERROR;
  }

  //1.2 设置表名
  retVal = pTabInfo->SetTableName(pTabName);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("device file ({}), invalid table name ({}), ret:{}", pPath, pTabName, retVal);
    return retVal;
  }

  for (uint32_t i = 0; i < pMeta->fieldCnt_; i++)
  {
    retVal = pTabInfo->AddField(pMeta->fieldRec_[i].fieldName_, 
      pMeta->fieldRec_[i].fieldType_, (i <= PDB_TSTAMP_INDEX));

    if (retVal != PdbE_OK)
    {
      LOG_ERROR("device file ({}) field ({},{}) error {}",
        pPath, pMeta->fieldRec_[i].fieldName_, pMeta->fieldRec_[i].fieldType_, retVal);
      return retVal;
    }
  }

  //2. 验证数据页
  DevIdPos devIdPos;
  DevItem* pDevItem = (DevItem*)pBase;
  size_t devCnt = fileSize / sizeof(DevItem);
  for (size_t idx = (sizeof(DevMetaFormat) / sizeof(DevItem)); idx < devCnt; idx++)
  {
    if (pDevItem[idx].pos_ == idx && pDevItem[idx].devId_ > 0)
    {
      if (devIdSet_.find(pDevItem[idx].devId_) != devIdSet_.end())
      {
        LOG_INFO("device file ({}) position({}) device ({}) exists",
          pPath, idx, pDevItem[idx].devId_);
      }
      else
      {
        devIdPos.devId_ = pDevItem[idx].devId_;
        devIdPos.pos_ = idx;

        devIdSet_.insert(pDevItem[idx].devId_);
        devIdVec_.push_back(pDevItem[idx].devId_);
        devIdPosVec_.push_back(devIdPos);
      }
    }
    else
    {
      freePosList_.push_back(idx);
    }
  }

  std::sort(devIdVec_.begin(), devIdVec_.end());
  std::sort(devIdPosVec_.begin(), devIdPosVec_.end(), DevIdPosComp);
  isSortId_ = true;
  isSortInfo_ = true;

  return PdbE_OK;
}

PdbErr_t DevIDTable::Close()
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  devIdFile_.Close();
  return PdbE_OK;
}

PdbErr_t DevIDTable::Alter(const TableInfo* pTabInfo)
{
  Arena arena;
  char* pTmpMeta = arena.Allocate(sizeof(DevMetaFormat));
  DevMetaFormat* pMeta = (DevMetaFormat*)pTmpMeta;

  std::unique_lock<std::mutex> fileLock(fileMutex_);

  memset(pTmpMeta, 0, sizeof(DevMetaFormat));
  strncpy(pMeta->headStr_, DEVID_FILE_TYPE_STR, DEVID_FILE_TYPE_STR_LEN);
  pMeta->fieldCnt_ = static_cast<int32_t>(pTabInfo->GetFieldCnt());

  int32_t fieldType = 0;
  const char* pFieldName = nullptr;

  for (uint32_t i = 0; i < pMeta->fieldCnt_; i++)
  {
    pTabInfo->GetFieldInfo(i, &fieldType);
    pFieldName = pTabInfo->GetFieldName(i);

    strncpy(pMeta->fieldRec_[i].fieldName_, pFieldName, PDB_FILED_NAME_LEN);
    pMeta->fieldRec_[i].fieldType_ = fieldType;
  }

  pMeta->crc_ = StringTool::CRC32(pTmpMeta, (sizeof(DevMetaFormat) - 4));
  uint8_t* pBase = devIdFile_.GetBaseAddr();
  memcpy(pBase, pTmpMeta, sizeof(DevMetaFormat));
  devIdFile_.Sync();

  return PdbE_OK;
}

size_t DevIDTable::GetDevCnt() const
{
  return devIdSet_.size();
}

PdbErr_t DevIDTable::DevExist(int64_t devId)
{
  std::unique_lock<std::mutex> insertLock(insertMutex_);
  return devIdSet_.find(devId) != devIdSet_.end() ? PdbE_OK : PdbE_DEV_NOT_FOUND;
}

PdbErr_t DevIDTable::AddDev(int64_t devId, PdbStr devName, PdbStr expand)
{
  PdbErr_t retVal = PdbE_OK;
  if (devId <= 0)
    return PdbE_INVALID_DEVID;

  if (devName.len_ >= PDB_DEVID_NAME_LEN)
    return PdbE_INVALID_DEVNAME;

  if (expand.len_ >= PDB_DEVID_EXPAND_LEN)
    return PdbE_INVALID_DEVEXPAND;

  std::unique_lock<std::mutex> fileLock(fileMutex_);

  if (DevExist(devId) == PdbE_OK)
    return PdbE_DEVID_EXISTS;
  
  if (freePosList_.empty())
  {
    size_t oldSize = devIdFile_.MemMapSize();
    retVal = devIdFile_.GrowFile(DEVID_FILE_BLOCK_SIZE);
    if (retVal != PdbE_OK)
      return retVal;

    size_t newSize = devIdFile_.MemMapSize();
    size_t bgIdx = oldSize / sizeof(DevItem);
    size_t edIdx = newSize / sizeof(DevItem);

    for (size_t idx = bgIdx; idx < edIdx; idx++)
    {
      freePosList_.push_back(idx);
    }

    if (freePosList_.empty())
      return PdbE_INVALID_PARAM;
  }

  uint8_t* pBase = devIdFile_.GetBaseAddr();
  size_t devPos = *freePosList_.begin();
  freePosList_.erase(freePosList_.begin());

  DevItem* pDevItem = (DevItem*)(pBase + (sizeof(DevItem) * devPos));
  memset(pDevItem, 0, sizeof(DevItem));

  pDevItem->pos_ = static_cast<uint32_t>(devPos);
  pDevItem->devId_ = devId;
  memcpy(pDevItem->devName_, devName.pStr_, devName.len_);
  memcpy(pDevItem->expand_, expand.pStr_, expand.len_);
  pDevItem->crc_ = StringTool::CRC32(pDevItem, (sizeof(DevItem) - 4));

  do {
    std::unique_lock<std::mutex> insertLock(insertMutex_);
    devIdSet_.insert(devId);
  } while (false);

  do {
    std::unique_lock<std::mutex> queryLock(queryIdMutex_);
    devIdVec_.push_back(devId);
    isSortId_ = false;
  } while (false);

  DevIdPos devIdPos;
  devIdPos.devId_ = devId;
  devIdPos.pos_ = devPos;
  devIdPosVec_.push_back(devIdPos);
  isSortInfo_ = false;

  return PdbE_OK;
}

PdbErr_t DevIDTable::QueryDevId(const DevFilter* pDevFilter, std::list<int64_t>& devIdVec)
{
  return QueryDevId(pDevFilter, devIdVec, 0, INT64_MAX);
}

PdbErr_t DevIDTable::QueryDevId(const DevFilter* pDevFilter, std::list<int64_t>& devIdList,
  size_t queryOffset, size_t queryRecord)
{
  int64_t minDevId = pDevFilter->GetMinDevId();
  int64_t maxDevId = pDevFilter->GetMaxDevId();

  std::unique_lock<std::mutex> queryLock(queryIdMutex_);
  if (!isSortId_)
  {
    std::sort(devIdVec_.begin(), devIdVec_.end());
    isSortId_ = true;
  }

  if (devIdVec_.empty())
    return PdbE_OK;

  int lwr = 0;
  int upr = static_cast<int>(devIdVec_.size() - 1);
  int idx = 0;

  while (lwr <= upr)
  {
    idx = (lwr + upr) / 2;
    if (devIdVec_[idx] == minDevId)
      break;
    else if (devIdVec_[idx] < minDevId)
      lwr = idx + 1;
    else
      upr = idx - 1;
  }

  for (; idx < static_cast<int>(devIdVec_.size()); idx++)
  {
    if (pDevFilter->Filter(devIdVec_[idx]))
    {
      if (queryOffset > 0)
      {
        queryOffset--;
      }
      else
      {
        devIdList.push_back(devIdVec_[idx]);
      }
    }

    if (devIdVec_[idx] >= maxDevId)
      break;

    if (devIdList.size() >= queryRecord)
      break;
  }

  return PdbE_OK;
}

PdbErr_t DevIDTable::QueryDevInfo(const std::string& tabName, IResultFilter* pFilter)
{
  PdbErr_t retVal = PdbE_OK;
  const int valCnt = 4;
  DBVal vals[valCnt];
  DBVAL_ELE_SET_STRING(vals, 0, tabName.c_str(), tabName.size());

  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (!isSortInfo_)
  {
    std::sort(devIdPosVec_.begin(), devIdPosVec_.end(), DevIdPosComp);
    isSortInfo_ = true;
  }

  uint8_t* pBase = devIdFile_.GetBaseAddr();
  for (auto devIt = devIdPosVec_.begin(); devIt != devIdPosVec_.end(); devIt++)
  {
    DevItem* pDevItem = (DevItem*)(pBase + (sizeof(DevItem) * devIt->pos_));
    DBVAL_ELE_SET_INT64(vals, 1, pDevItem->devId_);
    DBVAL_ELE_SET_STRING(vals, 2, pDevItem->devName_, strlen(pDevItem->devName_));
    DBVAL_ELE_SET_STRING(vals, 3, pDevItem->expand_, strlen(pDevItem->expand_));

    retVal = pFilter->AppendData(vals, valCnt, nullptr);
    if (retVal != PdbE_OK)
      return retVal;

    if (pFilter->GetIsFullFlag())
      return PdbE_OK;
  }

  return PdbE_OK;
}

PdbErr_t DevIDTable::DelDev(const std::string& tabName, const ConditionFilter* pCondition)
{
  PdbErr_t retVal = PdbE_OK;
  const int valCnt = 4;
  bool isDel = false;
  DBVal vals[valCnt];
  DBVAL_ELE_SET_STRING(vals, 0, tabName.c_str(), tabName.size());

  do {
    std::unique_lock<std::mutex> fileLock(fileMutex_);
    if (!isSortInfo_)
    {
      std::sort(devIdPosVec_.begin(), devIdPosVec_.end(), DevIdPosComp);
      isSortInfo_ = true;
    }

    uint8_t* pBase = devIdFile_.GetBaseAddr();
    for (auto devIt = devIdPosVec_.begin(); devIt != devIdPosVec_.end(); )
    {
      DevItem* pDevItem = (DevItem*)(pBase + (sizeof(DevItem) * devIt->pos_));
      DBVAL_ELE_SET_INT64(vals, 1, pDevItem->devId_);
      DBVAL_ELE_SET_STRING(vals, 2, pDevItem->devName_, strlen(pDevItem->devName_));
      DBVAL_ELE_SET_STRING(vals, 3, pDevItem->expand_, strlen(pDevItem->expand_));

      retVal = pCondition->RunCondition(vals, valCnt, isDel);
      if (retVal != PdbE_OK)
        return retVal;

      if (isDel)
      {
        _DelDev(pDevItem->devId_);
        memset(pDevItem, 0, sizeof(DevItem));
        devIt = devIdPosVec_.erase(devIt);
      }
      else
      {
        devIt++;
      }
    }
  } while (false);

  Flush();
  return PdbE_OK;
}

void DevIDTable::Flush()
{
  do {
    std::unique_lock<std::mutex> fileLock(fileMutex_);
    devIdFile_.Sync();

    if (!isSortInfo_)
    {
      std::sort(devIdPosVec_.begin(), devIdPosVec_.end(), DevIdPosComp);
      isSortInfo_ = true;
    }
  } while (false);

  do {
    std::unique_lock<std::mutex> queryLock(queryIdMutex_);
    if (!isSortId_)
    {
      std::sort(devIdVec_.begin(), devIdVec_.end());
      isSortId_ = true;
    }
  } while (false);
}

PdbErr_t DevIDTable::_DelDev(int64_t devId)
{
  do {
    std::unique_lock<std::mutex> insertLock(insertMutex_);
    devIdSet_.erase(devId);
  } while (false);

  do {
    std::unique_lock<std::mutex> queryLock(queryIdMutex_);
    if (!isSortId_)
    {
      std::sort(devIdVec_.begin(), devIdVec_.end());
      isSortId_ = true;
    }

    auto devIt = std::find(devIdVec_.begin(), devIdVec_.end(), devId);
    if (devIt != std::end(devIdVec_))
    {
      devIdVec_.erase(devIt);
      return PdbE_OK;
    }

  } while (false);

  return PdbE_OK;
}
