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

#include "db/table_set.h"
#include "util/log_util.h"
#include "global_variable.h"
#include "sys_table_schema.h"
#include "query/result_filter_raw.h"
#include "query/result_filter_group_all.h"

#define PDB_MAX_TABLE_CODE    255  //最大的表Code
#define PDB_MAX_OPEN_TABLE     32  //最大打开表数量

const char* pFieldTypeStr[] = {
  "",                // 0
  "bool",            // 1
  "bigint",          // 2
  "datetime",        // 3
  "double",          // 4
  "string",          // 5
  "blob",            // 6
  "",                // 7
  "",                // 8
  "",                // 9
  "",                // 10
  "",                // 11
  "",                // 12
  "",                // 13
  "",                // 14
  "",                // 15
  "",                // 16
  "",                // 17
  "",                // 18
  "",                // 19
  "",                // 20
  "",                // 21
  "",                // 22
  "",                // 23
  "",                // 24
  "",                // 25
  "",                // 26
  "",                // 27
  "",                // 28
  "",                // 29
  "",                // 30
  "",                // 31
  "real2",           // 32
  "real3",           // 33
  "real4",           // 34
  "real6",           // 35
};


TableSet::TableSet()
{
  running_ = true;
  maxDev_ = PDB_SYS_DEV_CNT;
  for (uint32_t tabCode = 1; tabCode < PDB_MAX_TABLE_CODE; tabCode++)
  {
    freeCodeList_.push_back(tabCode);
  }

  sysDevInfo_.SetTableName(SYSTAB_SYSDEV_NAME);
  sysDevInfo_.AddField(SYSCOL_SYSDEV_TABNAME, PDB_FIELD_TYPE::TYPE_STRING, true);
  sysDevInfo_.AddField(SYSCOL_SYSDEV_DEVID, PDB_FIELD_TYPE::TYPE_INT64, true);
  sysDevInfo_.AddField(SYSCOL_SYSDEV_DEVNAME, PDB_FIELD_TYPE::TYPE_STRING);
  sysDevInfo_.AddField(SYSCOL_SYSDEV_EXPAND, PDB_FIELD_TYPE::TYPE_STRING);

  sysUserInfo_.SetTableName(SYSTAB_SYSUSER_NAME);
  sysUserInfo_.AddField(SYSCOL_SYSUSER_USERNAME, PDB_FIELD_TYPE::TYPE_STRING, true);
  sysUserInfo_.AddField(SYSCOL_SYSUSER_ROLE, PDB_FIELD_TYPE::TYPE_STRING);

  sysTableInfo_.SetTableName(SYSTAB_SYSTABLE_NAME);
  sysTableInfo_.AddField(SYSCOL_SYSTABLE_TABNAME, PDB_FIELD_TYPE::TYPE_STRING, true);

  sysDataFileInfo_.SetTableName(SYSTAB_SYSDATAFILE_NAME);
  sysDataFileInfo_.AddField(SYSCOL_SYSDATAFILE_TABNAME, PDB_FIELD_TYPE::TYPE_STRING, true);
  sysDataFileInfo_.AddField(SYSCOL_SYSDATAFILE_FILEDATE, PDB_FIELD_TYPE::TYPE_STRING, true);
  sysDataFileInfo_.AddField(SYSCOL_SYSDATAFILE_PARTTYPE, PDB_FIELD_TYPE::TYPE_STRING);

  sysColumnInfo_.SetTableName(SYSTAB_SYSCOLUMN_NAME);
  sysColumnInfo_.AddField(SYSCOL_SYSCOLUMN_TABNAME, PDB_FIELD_TYPE::TYPE_STRING, true);
  sysColumnInfo_.AddField(SYSCOL_SYSCOLUMN_COLUMNNAME, PDB_FIELD_TYPE::TYPE_STRING, true);
  sysColumnInfo_.AddField(SYSCOL_SYSCOLUMN_DATATYPE, PDB_FIELD_TYPE::TYPE_STRING);
  sysColumnInfo_.AddField(SYSCOL_SYSCOLUMN_ISKEY, PDB_FIELD_TYPE::TYPE_BOOL);

  sysConnInfo_.SetTableName(SYSTAB_CONNECTION_NAME);
  sysConnInfo_.AddField(SYSCOL_CONNECTION_HOSTNAME, PDB_FIELD_TYPE::TYPE_STRING, true);
  sysConnInfo_.AddField(SYSCOL_CONNECTION_PORTNAME, PDB_FIELD_TYPE::TYPE_INT64, true);
  sysConnInfo_.AddField(SYSCOL_CONNECTION_USERNAME, PDB_FIELD_TYPE::TYPE_STRING);
  sysConnInfo_.AddField(SYSCOL_CONNECTION_ROLENAME, PDB_FIELD_TYPE::TYPE_STRING);
  sysConnInfo_.AddField(SYSCOL_CONNECTION_CONNTIMENAME, PDB_FIELD_TYPE::TYPE_DATETIME);

  sysConfigInfo_.SetTableName(SYSTAB_SYSCFG_NAME);
  sysConfigInfo_.AddField(SYSCOL_SYSCFG_NAME, PDB_FIELD_TYPE::TYPE_STRING, true);
  sysConfigInfo_.AddField(SYSCOL_SYSCFG_VALUE, PDB_FIELD_TYPE::TYPE_STRING);

  systab_sysuser_crc_ = StringTool::CRC64NoCase(SYSTAB_SYSUSER_NAME);
  systab_systable_crc_ = StringTool::CRC64NoCase(SYSTAB_SYSTABLE_NAME);
  systab_sysdatafile_crc_ = StringTool::CRC64NoCase(SYSTAB_SYSDATAFILE_NAME);
  systab_syscolumn_crc_ = StringTool::CRC64NoCase(SYSTAB_SYSCOLUMN_NAME);
  systab_connection_crc_ = StringTool::CRC64NoCase(SYSTAB_CONNECTION_NAME);
  systab_sysconfig_crc_ = StringTool::CRC64NoCase(SYSTAB_SYSCFG_NAME);
  systab_sysdev_crc_ = StringTool::CRC64NoCase(SYSTAB_SYSDEV_NAME);

}

TableSet::~TableSet()
{
  for (auto tabIt = tabMap_.begin(); tabIt != tabMap_.end(); tabIt++)
  {
    delete tabIt->second;
  }
}

void TableSet::Stop()
{
  running_ = false;
}

PdbErr_t TableSet::CreateTable(const CreateTableParam* pTableParam)
{
  PdbErr_t retVal = PdbE_OK;
  TableInfo tabInfo;

  if (pTableParam == nullptr)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> tabLock(tabMutex_);

  retVal = tabInfo.SetTableName(pTableParam->tabName_.c_str());
  if (retVal != PdbE_OK)
    return retVal;

  const char* pTabName = tabInfo.GetTableName();
  uint64_t tabNameCrc = StringTool::CRC64NoCase(pTabName);

  if (GetTable(tabNameCrc, nullptr) != nullptr)
  {
    return PdbE_TABLE_EXIST;
  }

  //表名不能以sys_开头
  if (StringTool::StartWithNoCase(pTabName, "sys_"))
  {
    LOG_INFO("failed to create table ({}), invalid table name", pTabName);
    return PdbE_INVALID_TABLE_NAME;
  }

  if (tabMap_.size() >= PDB_MAX_OPEN_TABLE)
  {
    LOG_INFO("failed to create table ({}), too many open tables", pTabName);
    return PdbE_TABLE_CAPACITY_FULL;
  }
  
  const std::vector<ColumnItem*>& colItemVec = pTableParam->pColList_->GetColumnList();
  for (auto colIt = colItemVec.begin(); colIt != colItemVec.end(); colIt++)
  {
    retVal = tabInfo.AddField((*colIt)->GetName(), (*colIt)->GetType());
    if (retVal != PdbE_OK)
    {
      LOG_INFO("failed to create table ({}), invalid field name or type", pTabName);
      return retVal;
    }
  }

  retVal = tabInfo.ValidStorageTable();
  if (retVal != PdbE_OK)
  {
    LOG_INFO("failed to create table ({}), invalid field list", pTabName);
    return retVal;
  }
  
  std::string devPath = pGlbSysCfg->GetTablePath() + "/" + pTabName + ".dev";

  retVal = DevIDTable::Create(devPath.c_str(), &tabInfo);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("failed to create table ({}), err: {}", pTabName, retVal);
    return retVal;
  }

  retVal = pGlbTabCfg->AddTable(pTabName);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("failed to create table ({}), add table config error, err: {}", pTabName, retVal);
    return retVal;
  }

  retVal = OpenTable(pTabName);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("failed to create table ({}), OpenTable error {} ", pTabName, retVal);
    return retVal;
  }

  RecoverDW(pTabName);

  return PdbE_OK;
}

PdbErr_t TableSet::AlterTable(const CreateTableParam* pTableParam)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabRef;

  if (pTableParam == nullptr)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> tabLock(tabMutex_);
  PDBTable* pTab = GetTable(pTableParam->tabName_.c_str(), &tabRef);
  if (pTab == nullptr)
  {
    return PdbE_TABLE_NOT_FOUND;
  }

  return pTab->AlterTable(pTableParam->pColList_->GetColumnList());
}

PdbErr_t TableSet::OpenTable(const char* pTabName)
{
  //OpenTable 只有两中情形下会调用
  //1. 服务启动时， 不用加锁
  //2. 创建表时，创建表的方法已经锁
  PdbErr_t retVal = PdbE_OK;
  size_t totalDevCnt = 0;
  uint64_t tabNameCrc = StringTool::CRC64NoCase(pTabName);

  if (StringTool::StartWithNoCase(pTabName, "sys_"))
  {
    LOG_ERROR("failed to open table ({}), invalid table name", pTabName);
    return PdbE_INVALID_TABLE_NAME;
  }

  if (GetTable(tabNameCrc, nullptr) != nullptr)
  {
    LOG_ERROR("failed to open table ({}), table exits", pTabName);
    return PdbE_TABLE_EXIST;
  }

  if (tabMap_.size() >= PDB_MAX_OPEN_TABLE)
  {
    LOG_INFO("failed to open table ({}), too many open tables", pTabName);
    return PdbE_TABLE_CAPACITY_FULL;
  }

  if (freeCodeList_.empty())
  {
    LOG_INFO("failed to open table ({}), too many open tables", pTabName);
    return PdbE_TABLE_CAPACITY_FULL;
  }

  totalDevCnt = GetTotalDevCnt();

  uint32_t tabCode = *freeCodeList_.begin();
  freeCodeList_.erase(freeCodeList_.begin());

  PDBTable* pNewTab = new PDBTable();
  do {
    retVal = pNewTab->OpenTable(tabCode, pTabName);
    if (retVal != PdbE_OK)
      break;

    if (totalDevCnt + pNewTab->GetDevCnt() > maxDev_)
    {
      LOG_INFO("failed to open table ({}), too many device", pTabName);
      retVal = PdbE_DEV_CAPACITY_FULL;
      break;
    }

    AddTableHandle(tabNameCrc, pNewTab);
  } while (false);

  if (retVal != PdbE_OK)
  {
    freeCodeList_.push_back(tabCode);
    delete pNewTab;
  }

  return retVal;
}

PdbErr_t TableSet::OpenDataPart(const char* pTabName, int partCode, bool isNormalPart)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabRef;
  uint64_t tabNameCrc = StringTool::CRC64NoCase(pTabName);

  PDBTable* pTable = GetTable(tabNameCrc, &tabRef);
  if (pTable == nullptr)
  {
    return PdbE_TABLE_NOT_FOUND;
  }

  return pTable->OpenDataPart(partCode, isNormalPart);
}

PdbErr_t TableSet::RecoverDW(const char* pTabName)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabRef;
  uint64_t tabNameCrc = StringTool::CRC64NoCase(pTabName);

  PDBTable* pTable = GetTable(tabNameCrc, &tabRef);
  if (pTable == nullptr)
  {
    return PdbE_TABLE_NOT_FOUND;
  }

  return pTable->RecoverDW();
}

PdbErr_t TableSet::DropTable(const char* pTabName)
{
  PdbErr_t retVal = PdbE_OK;
  PDBTable* pTable = nullptr;
  uint64_t tabCrc = 0;

  if (pTabName == nullptr)
    return PdbE_INVALID_PARAM;

  tabCrc = StringTool::CRC64NoCase(pTabName);

  std::unique_lock<std::mutex> tabLock(tabMutex_);

  pTable = EraseTable(tabCrc);
  if (pTable == nullptr)
    return PdbE_TABLE_NOT_FOUND;

  pTable->CancelDumpTask();
  while (pTable->GetRefCnt() > 0)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
  }

  pGlbTabCfg->DropTable(pTabName);

  uint32_t tabCode = pTable->GetTabCode();
  uint64_t pageCode = MAKE_DATATABLE_MASK(tabCode);

  pTable->DropTable();
  pGlbPagePool->ClearPageForMask(pageCode, PDB_PAGEPOOL_TABLE_MASK);
  delete pTable;
  return PdbE_OK;
}

PdbErr_t TableSet::AttachTable(const char* pTabName)
{
  PdbErr_t retVal = PdbE_OK;
  if (pTabName == nullptr)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> tabLock(tabMutex_);
  retVal = OpenTable(pTabName);
  if (retVal == PdbE_OK)
  {
    pGlbTabCfg->AddTable(pTabName);
  }

  return retVal;
}

PdbErr_t TableSet::DetachTable(const char* pTabName)
{
  PdbErr_t retVal = PdbE_OK;
  PDBTable* pTable = nullptr;
  uint64_t tabCrc = 0;

  if (pTabName == nullptr)
    return PdbE_INVALID_PARAM;

  tabCrc = StringTool::CRC64NoCase(pTabName);

  std::unique_lock<std::mutex> tabLock(tabMutex_);

  pTable = EraseTable(tabCrc);
  if (pTable == nullptr)
    return PdbE_TABLE_NOT_FOUND;

  pTable->CancelDumpTask();
  while (pTable->GetRefCnt() > 0)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
  }

  pGlbTabCfg->DropTable(pTabName);

  uint32_t tabCode = pTable->GetTabCode();
  uint64_t pageCode = MAKE_DATATABLE_MASK(tabCode);

  pTable->Close();
  pGlbPagePool->ClearPageForMask(pageCode, PDB_PAGEPOOL_TABLE_MASK);
  delete pTable;
  return PdbE_OK;
}

PdbErr_t TableSet::AttachFile(const char* pTabName, const char* pPartStr, int fileType)
{
  RefUtil tabRef;
  std::unique_lock<std::mutex> tabLock(tabMutex_);
  PDBTable* pTab = GetTable(pTabName, &tabRef);
  if (pTab == nullptr)
    return PdbE_TABLE_NOT_FOUND;

  return pTab->AttachPart(pPartStr, fileType);
}

PdbErr_t TableSet::DetachFile(const char* pTabName, int partCode)
{
  RefUtil tabRef;
  std::unique_lock<std::mutex> tabLock(tabMutex_);
  PDBTable* pTab = GetTable(pTabName, &tabRef);
  if (pTab == nullptr)
    return PdbE_TABLE_NOT_FOUND;

  return pTab->DetachPart(partCode);
}

PdbErr_t TableSet::DropFile(const char* pTabName, int partCode)
{
  RefUtil tabRef;
  std::unique_lock<std::mutex> tabLock(tabMutex_);
  PDBTable* pTab = GetTable(pTabName, &tabRef);
  if (pTab == nullptr)
    return PdbE_TABLE_NOT_FOUND;

  return pTab->DropPart(partCode);
}

PdbErr_t TableSet::ExecuteQuery(DataTable* pResultTable, SQLParser* pParser, int32_t userRole)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabRef;

  if (pResultTable == nullptr || pParser == nullptr)
    return PdbE_INVALID_PARAM;

  if (pParser->GetCmdType() != SQLParser::CmdType::CT_Select)
    return PdbE_SQL_NOT_QUERY;

  //判断用户是否有权限查询数据
  if (userRole == PDB_ROLE::ROLE_WRITE_ONLY)
    return PdbE_OPERATION_DENIED;

  const QueryParam* pQueryParam = pParser->GetQueryParam();
  PDBTable* pTab = GetTable(pQueryParam->srcTab_.c_str(), &tabRef);
  if (pTab != nullptr)
  {
    retVal = pTab->Query(pResultTable, pQueryParam);
  }
  else if (StringTool::EndWithNoCase(pQueryParam->srcTab_, SNAPSHOT_NAME, SNAPSHOT_NAME_LEN))
  {
    //查询快照
    std::string tmpTabName = pQueryParam->srcTab_.substr(0, (pQueryParam->srcTab_.length() - SNAPSHOT_NAME_LEN));
    pTab = GetTable(tmpTabName.c_str(), &tabRef);
    if (pTab != nullptr)
      retVal = pTab->QuerySnapshot(pResultTable, pQueryParam);
    else
      retVal = PdbE_TABLE_NOT_FOUND;
  }
  else
  {
    //是否是系统表
    retVal = QuerySysTable(pQueryParam, userRole, pResultTable);
  }

  return retVal;
}

PdbErr_t TableSet::DeleteDev(const DeleteParam* pDeleteParam)
{
  if (!StringTool::ComparyNoCase(pDeleteParam->tabName_.c_str(), SYSTAB_SYSDEV_NAME))
  {
    LOG_INFO("failed to delete table ({}), only support delete device",
      pDeleteParam->tabName_.c_str());
    return PdbE_OPERATION_DENIED;
  }

  ConditionFilter delCondition;
  PdbErr_t retVal = delCondition.BuildCondition(pDeleteParam->pWhere_, &sysDevInfo_);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("failed to delete device, condition error {}", retVal);
    return retVal;
  }

  RefUtil tabRef;
  std::list<uint64_t> tabCrcList;
  std::unique_lock<std::mutex> devLock(devMutex_);

  GetAllTable(tabCrcList);
  for (auto crcIter = tabCrcList.begin(); crcIter != tabCrcList.end(); crcIter++)
  {
    PDBTable* pTab = GetTable(*crcIter, &tabRef);
    if (pTab != nullptr)
    {
      retVal = pTab->DelDev(&delCondition);
      if (retVal != PdbE_OK)
        break;
    }
  }

  return retVal;
}

PDBTable* TableSet::GetTable(const char* pTabName, RefUtil* pTabRef)
{
  uint64_t tabCrc = StringTool::CRC64NoCase(pTabName);
  return GetTable(tabCrc, pTabRef);
}

PDBTable* TableSet::GetTable(uint64_t tabNameCrc, RefUtil* pTabRef)
{
  std::unique_lock<std::mutex> tabMapLock(tabMapMutex_);
  auto tabIt = tabMap_.find(tabNameCrc);
  if (tabIt != tabMap_.end())
  {
    if (pTabRef != nullptr)
      pTabRef->Attach(tabIt->second);

    return tabIt->second;
  }

  return nullptr;
}

PdbErr_t TableSet::Insert(InsertSql* pInsertSql, bool errBreak, std::list<PdbErr_t>& resultList)
{
  RefUtil tabRef;
  std::string tabName = pInsertSql->GetTableName();
  PDBTable* pTable = GetTable(tabName.c_str(), &tabRef);
  if (pTable != nullptr)
    return pTable->Insert(pInsertSql, errBreak, resultList);
  else if (StringTool::ComparyNoCase(tabName.c_str(), SYSTAB_SYSDEV_NAME))
    return InsertDev(pInsertSql, errBreak, resultList);

  return PdbE_TABLE_NOT_FOUND;
}

PdbErr_t QueryTableColumn(IResultFilter* pFilter, const TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;

  const char* pFieldName = nullptr;
  const char* pTypeStr = nullptr;
  const size_t fieldTypeCnt = sizeof(pFieldTypeStr) / sizeof(pFieldTypeStr[0]);
  int32_t fieldType = 0;
  const int valCnt = 4;
  DBVal vals[valCnt];

  const char* pTabName = pTabInfo->GetTableName();
  size_t tabNameLen = strlen(pTabName);
  size_t fieldCnt = pTabInfo->GetFieldCnt();
  bool isKey = false;
  for (size_t i = 0; i < fieldCnt; i++)
  {
    pFieldName = pTabInfo->GetFieldName(i);
    pTabInfo->GetFieldInfo(i, &fieldType);
    pTypeStr = pFieldTypeStr[0];
    pTabInfo->GetFieldIsKey(i, &isKey);
    if (fieldType > 0 && fieldType < fieldTypeCnt)
      pTypeStr = pFieldTypeStr[fieldType];

    DBVAL_ELE_SET_STRING(vals, 0, pTabName, tabNameLen);
    DBVAL_ELE_SET_STRING(vals, 1, pFieldName, strlen(pFieldName));
    DBVAL_ELE_SET_STRING(vals, 2, pTypeStr, strlen(pTypeStr));
    DBVAL_ELE_SET_BOOL(vals, 3, isKey);

    retVal = pFilter->AppendData(vals, valCnt, nullptr);
    if (retVal != PdbE_OK)
      return retVal;

    if (pFilter->GetIsFullFlag())
      return PdbE_OK;
  }

  return PdbE_OK;
}

PdbErr_t TableSet::QueryColumn(IResultFilter* pFilter)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabRef;
  RefUtil tabInfoRef;
  std::list<uint64_t> tabCrcList;

  retVal = QueryTableColumn(pFilter, &sysConfigInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pFilter->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pFilter, &sysUserInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pFilter->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pFilter, &sysTableInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pFilter->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pFilter, &sysColumnInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pFilter->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pFilter, &sysDataFileInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pFilter->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pFilter, &sysDevInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pFilter->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pFilter, &sysConnInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pFilter->GetIsFullFlag())
    return PdbE_OK;

  GetAllTable(tabCrcList);
  for (auto crcIter = tabCrcList.begin(); crcIter != tabCrcList.end(); crcIter++)
  {
    PDBTable* pTab = GetTable(*crcIter, &tabRef);
    if (pTab != nullptr)
    {
      const TableInfo* pTabInfo = pTab->GetTableInfo(&tabInfoRef);
      retVal = QueryTableColumn(pFilter, pTabInfo);
      if (retVal != PdbE_OK)
        return retVal;

      if (pFilter->GetIsFullFlag())
        break;
    }
  }

  return PdbE_OK;
}

PdbErr_t TableSet::QueryDev(IResultFilter* pFilter)
{
  PdbErr_t retVal = PdbE_OK;

  RefUtil tabRef;
  std::list<uint64_t> tabCrcList;

  GetAllTable(tabCrcList);

  for (auto crcIter = tabCrcList.begin(); crcIter != tabCrcList.end(); crcIter++)
  {
    PDBTable* pTab = GetTable(*crcIter, &tabRef);
    if (pTab != nullptr)
    {
      retVal = pTab->QueryDev(pFilter);
      if (retVal != PdbE_OK)
        return retVal;

      if (pFilter->GetIsFullFlag())
        return PdbE_OK;
    }
  }

  return PdbE_OK;
}

PdbErr_t TableSet::SyncDirtyPages(bool syncAll)
{
  RefUtil tabRef;
  std::list<uint64_t> tabCrcList;
  uint32_t fileCode = 0;
  int64_t filePos = 0;

  uint64_t curLogPos = 0;

  pGlbCommitLog->GetCurLogPos(&curLogPos);
  GetAllTable(tabCrcList);

  for (auto crcIter = tabCrcList.begin(); crcIter != tabCrcList.end(); crcIter++)
  {
    PDBTable* pTab = GetTable(*crcIter, &tabRef);
    if (pTab != nullptr)
    {
      pTab->SyncDirtyPages(syncAll);
    }
  }

  if (syncAll)
    pGlbCommitLog->SetSyncPos(curLogPos);

  return PdbE_OK;
}

PdbErr_t TableSet::CloseAllTable()
{
  RefUtil tabRef;
  std::list<uint64_t> tabCrcList;

  GetAllTable(tabCrcList);

  for (auto crcIter = tabCrcList.begin(); crcIter != tabCrcList.end(); crcIter++)
  {
    PDBTable* pTab = GetTable(*crcIter, &tabRef);
    if (pTab != nullptr)
    {
      pTab->Close();
    }
  }

  return PdbE_OK;
}

void TableSet::DumpToCompress()
{
  RefUtil tabRef;
  std::list<uint64_t> tabCrcList;

  GetAllTable(tabCrcList);
  glbCancelCompTask = false;
  
  for (auto crcIter = tabCrcList.begin(); crcIter != tabCrcList.end(); crcIter++)
  {
    PDBTable* pTab = GetTable(*crcIter, &tabRef);
    if (pTab != nullptr)
    {
      pTab->DumpPartToComp();
    }

    if (glbCancelCompTask)
      break;
  }
}

void TableSet::UnMapCompressData()
{
  RefUtil tabRef;
  std::list<uint64_t> tabCrcList;

  GetAllTable(tabCrcList);
  glbCancelCompTask = false;

  for (auto crcIter = tabCrcList.begin(); crcIter != tabCrcList.end(); crcIter++)
  {
    PDBTable* pTab = GetTable(*crcIter, &tabRef);
    if (pTab != nullptr)
    {
      pTab->UnMapCompressData();
    }
  }
}

size_t TableSet::GetDirtyPagePercent()
{
  size_t poolPageCnt = pGlbPagePool->GetPoolPageCnt();
  size_t dirtyPageCnt = 0;

  RefUtil tabRef;
  std::list<uint64_t> tabCrcList;
  GetAllTable(tabCrcList);
  for (auto crcIter = tabCrcList.begin(); crcIter != tabCrcList.end(); crcIter++)
  {
    PDBTable* pTab = GetTable(*crcIter, &tabRef);
    if (pTab != nullptr)
    {
      dirtyPageCnt += pTab->GetDirtyPageCnt();
    }
  }

  return ((dirtyPageCnt * 100) / poolPageCnt);
}

#define INSERT_DEV_ERROR_OCCUR \
  includeErr = true;\
  if (errBreak) break;\
  resultList.push_back(retVal); \
  continue

PdbErr_t TableSet::InsertDev(InsertSql* pInsertSql,
  bool errBreak, std::list<PdbErr_t>& resultList)
{
  const int devFieldCnt = 4;
  PdbErr_t retVal = PdbE_OK;
  bool includeErr = false;
  RefUtil tabRef;
  PDBTable* pTable = nullptr;
  std::list<uint64_t> tabCrcList;

  size_t totalDevCnt = 0;
  int64_t devId = 0;
  PdbStr devName;
  PdbStr devExpand;

  DBVal devVals[devFieldCnt];
  DBVAL_ELE_SET_STRING(devVals, 0, nullptr, 0);
  DBVAL_ELE_SET_INT64(devVals, 1, 0);
  DBVAL_ELE_SET_STRING(devVals, 2, nullptr, 0);
  DBVAL_ELE_SET_STRING(devVals, 3, nullptr, 0);
  
  retVal = pInsertSql->InitTableInfo(&sysDevInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  do {
    std::unique_lock<std::mutex> devLock(devMutex_);
    totalDevCnt = GetTotalDevCnt();

    while (!pInsertSql->IsEnd())
    {
      retVal = pInsertSql->GetNextRec(devVals, devFieldCnt);
      if (retVal != PdbE_OK)
      {
        INSERT_DEV_ERROR_OCCUR;
      }

      if (totalDevCnt >= maxDev_)
      {
        retVal = PdbE_DEV_CAPACITY_FULL;
        INSERT_DEV_ERROR_OCCUR;
      }

      std::string devTabName = std::string(DBVAL_ELE_GET_STRING(devVals, 0),
        DBVAL_ELE_GET_LEN(devVals, 0));
      pTable = GetTable(devTabName.c_str(), &tabRef);
      if (pTable == nullptr)
      {
        retVal = PdbE_TABLE_NOT_FOUND;
        INSERT_DEV_ERROR_OCCUR;
      }

      devId = DBVAL_ELE_GET_INT64(devVals, 1);
      devName.pStr_ = DBVAL_ELE_GET_STRING(devVals, 2);
      devName.len_ = DBVAL_ELE_GET_LEN(devVals, 2);
      devExpand.pStr_ = DBVAL_ELE_GET_STRING(devVals, 3);
      devExpand.len_ = DBVAL_ELE_GET_LEN(devVals, 3);

      retVal = pTable->AddDev(devId, devName, devExpand);
      if (retVal != PdbE_OK)
      {
        INSERT_DEV_ERROR_OCCUR;
      }

      resultList.push_back(PdbE_OK);
      totalDevCnt++;
    }
  } while (false);

  do {
    //同步设备信息
    GetAllTable(tabCrcList);

    for (auto tabIter = tabCrcList.begin(); tabIter != tabCrcList.end(); tabIter++)
    {
      PDBTable* pTab = GetTable(*tabIter, &tabRef);
      if (pTab != nullptr)
      {
        pTab->FlushDev();
      }
    }
  } while (false);

  if (errBreak)
    return retVal;

  if (includeErr)
    return PdbE_INSERT_PART_ERROR;

  return PdbE_OK;
}

PdbErr_t TableSet::QuerySysTable(const QueryParam* pQueryParam,
  int32_t userRole, DataTable* pResultTable)
{
  PdbErr_t retVal = PdbE_OK;
  uint64_t tabNameCrc = StringTool::CRC64NoCase(pQueryParam->srcTab_.c_str());

  if (tabNameCrc == systab_sysuser_crc_
    || tabNameCrc == systab_connection_crc_
    || tabNameCrc == systab_sysconfig_crc_)
  {
    if (userRole != PDB_ROLE::ROLE_ADMIN)
      return PdbE_OPERATION_DENIED;
  }

  IResultFilter* pFilter = nullptr;
  if (pQueryParam->IsQueryRaw())
    pFilter = new ResultFilterRaw();
  else if (pQueryParam->pGroup_ == nullptr)
    pFilter = new ResultFilterGroupAll(); //Group All
  else
    return PdbE_SQL_GROUP_ERROR;

  if (pFilter == nullptr)
    return PdbE_NOMEM;

  do {
    if (tabNameCrc == systab_sysuser_crc_)
    {
      retVal = pFilter->BuildFilter(pQueryParam, &sysUserInfo_, pResultTable->GetArena());
      if (retVal != PdbE_OK)
        break;

      retVal = pGlbUser->Query(pFilter);
    }
    else if (tabNameCrc == systab_systable_crc_)
    {
      retVal = pFilter->BuildFilter(pQueryParam, &sysTableInfo_, pResultTable->GetArena());
      if (retVal != PdbE_OK)
        break;

      retVal = pGlbTabCfg->QueryTable(pFilter);
    }
    else if (tabNameCrc == systab_sysdatafile_crc_)
    {
      retVal = pFilter->BuildFilter(pQueryParam, &sysDataFileInfo_, pResultTable->GetArena());
      if (retVal != PdbE_OK)
        break;

      retVal = pGlbTabCfg->QueryPart(pFilter);
    }
    else if (tabNameCrc == systab_syscolumn_crc_)
    {
      retVal = pFilter->BuildFilter(pQueryParam, &sysColumnInfo_, pResultTable->GetArena());
      if (retVal != PdbE_OK)
        break;

      retVal = QueryColumn(pFilter);
    }
    else if (tabNameCrc == systab_connection_crc_)
    {
      retVal = pFilter->BuildFilter(pQueryParam, &sysConnInfo_, pResultTable->GetArena());
      if (retVal != PdbE_OK)
        break;

      retVal = pGlbServerConnction->QueryConn(pFilter);
    }
    else if (tabNameCrc == systab_sysconfig_crc_)
    {
      retVal = pFilter->BuildFilter(pQueryParam, &sysConfigInfo_, pResultTable->GetArena());
      if (retVal != PdbE_OK)
        break;

      retVal = pGlbSysCfg->Query(pFilter);
    }
    else if (tabNameCrc == systab_sysdev_crc_)
    {
      retVal = pFilter->BuildFilter(pQueryParam, &sysDevInfo_, pResultTable->GetArena());
      if (retVal != PdbE_OK)
        break;

      retVal = QueryDev(pFilter);
    }
    else
      retVal = PdbE_TABLE_NOT_FOUND;

  } while (false);

  if (retVal == PdbE_OK)
  {
    retVal = pFilter->GetData(pResultTable);
  }

  delete pFilter;
  return retVal;
}

size_t TableSet::GetTotalDevCnt()
{
  size_t devCnt = 0;
  do {
    std::unique_lock<std::mutex> tabMapLock(tabMapMutex_);
    for (auto tabIt = tabMap_.begin(); tabIt != tabMap_.end(); tabIt++)
    {
      devCnt += tabIt->second->GetDevCnt();
    }
  } while (false);

  return devCnt;
}

void TableSet::GetAllTable(std::list<uint64_t>& tableCrcList)
{
  tableCrcList.clear();
  std::unique_lock<std::mutex> tabMapLock(tabMapMutex_);
  for (auto tabIt = tabMap_.begin(); tabIt != tabMap_.end(); tabIt++)
  {
    tableCrcList.push_back(tabIt->first);
  }
}

void TableSet::AddTableHandle(uint64_t tabCrc, PDBTable* pTable)
{
  std::unique_lock<std::mutex> tabMapLock(tabMapMutex_);
  std::pair<uint64_t, PDBTable*> tabPair(tabCrc, pTable);
  tabMap_.insert(tabPair);
}

PDBTable* TableSet::EraseTable(uint64_t tabCrc)
{
  PDBTable* pTab = nullptr;
  std::unique_lock<std::mutex> mapLock(tabMapMutex_);
  auto tabIt = tabMap_.find(tabCrc);
  if (tabIt != tabMap_.end())
  {
    pTab = tabIt->second;
    tabMap_.erase(tabCrc);
  }
  return pTab;
}
