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
#include "query/iquery.h"
#include "query/query_raw.h"
#include "query/query_group.h"
#include "internal.h"

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
  for (uint32_t tabCode = 1; tabCode < PDB_MAX_TABLE_CODE; tabCode++)
  {
    freeCodeList_.push_back(tabCode);
  }

  maxDev_ = PDB_SYS_DEV_CNT;

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

PdbErr_t TableSet::CreateTable(const char* pTableName, const ColumnList* pColList)
{
  PdbErr_t retVal = PdbE_OK;
  TableInfo tabInfo;

  if (pTableName == nullptr || pColList == nullptr)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> tabLock(tabMutex_);

  retVal = tabInfo.SetTableName(pTableName);
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
  
  const std::vector<ColumnItem*>& colItemVec = pColList->GetColumnList();
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

PdbErr_t TableSet::AlterTable(const char* pTableName, const ColumnList* pColList)
{
  RefUtil tabRef;

  if (pTableName == nullptr || pColList == nullptr)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> tabLock(tabMutex_);
  PDBTable* pTab = GetTable(pTableName, &tabRef);
  if (pTab == nullptr)
  {
    return PdbE_TABLE_NOT_FOUND;
  }

  const std::vector<ColumnItem*>& colItemVec = pColList->GetColumnList();
  return pTab->AlterTable(colItemVec);
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
    {
      break;
    }

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

PdbErr_t TableSet::ExecuteQuery(SQLParser* pParser, int32_t userRole,
  std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabRef;

  if (pParser == nullptr)
    return PdbE_INVALID_PARAM;

  if (pParser->GetCmdType() != SQLParser::CmdType::CT_Select)
    return PdbE_SQL_NOT_QUERY;

  //判断用户是否有权限查询数据
  if (userRole == PDB_ROLE::ROLE_WRITE_ONLY)
    return PdbE_OPERATION_DENIED;

  const QueryParam* pQueryParam = pParser->GetQueryParam();
  std::string tableName = pParser->GetTableName();
  if (tableName.size() == 0)
  {
    retVal = QueryVariable(pQueryParam, resultData, pFieldCnt, pRecordCnt);
  }
  else
  {
    PDBTable* pTab = GetTable(tableName.c_str(), &tabRef);
    if (pTab != nullptr)
    {
      retVal = pTab->ExecQuery(pQueryParam, resultData, pFieldCnt, pRecordCnt);
    }
    else if (StringTool::EndWithNoCase(tableName.c_str(), SNAPSHOT_NAME, SNAPSHOT_NAME_LEN))
    {
      //查询快照
      std::string tmpTabName = tableName.substr(0, (tableName.size() - SNAPSHOT_NAME_LEN));
      pTab = GetTable(tmpTabName.c_str(), &tabRef);
      if (pTab != nullptr)
        retVal = pTab->ExecQuerySnapshot(pQueryParam, resultData, pFieldCnt, pRecordCnt);
      else
        retVal = PdbE_TABLE_NOT_FOUND;
    }
    else
    {
      //是否是系统表
      retVal = QuerySysTable(tableName.c_str(), pQueryParam, userRole, resultData, pFieldCnt, pRecordCnt);
    }
  }

  return retVal;
}

PdbErr_t TableSet::DeleteDev(const char* pTabName, const DeleteParam* pDeleteParam)
{
  if (!StringTool::ComparyNoCase(pTabName, SYSTAB_SYSDEV_NAME))
  {
    LOG_INFO("failed to delete table ({}), only support delete device",
      pTabName);
    return PdbE_OPERATION_DENIED;
  }

  int64_t nowMillis = DateTime::NowMilliseconds();
  ConditionFilter delCondition;
  PdbErr_t retVal = delCondition.BuildCondition(&sysDevInfo_, pDeleteParam->pWhere_, nowMillis);
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

PdbErr_t TableSet::InsertReplicate(std::vector<LogRecInfo>& recVec)
{
  RefUtil tabRef;
  std::unordered_set<uint64_t> tabSet;
  size_t idx;
  size_t cnt = recVec.size();

  for (idx = 0; idx < cnt; idx++)
  {
    if (tabSet.find(recVec[idx].tabCrc) == tabSet.end())
    {
      tabSet.insert(recVec[idx].tabCrc);
      PDBTable* pTable = GetTable(recVec[idx].tabCrc, &tabRef);
      if (pTable != nullptr)
      {
        pTable->InsertByReplicate(recVec, idx);
      }
      else
      {
        LOG_DEBUG("insert replicate failed, table({}) not found", recVec[idx].tabCrc);
      }
    }
  }

  return PdbE_OK;
}

PdbErr_t QueryTableColumn(IQuery* pQuery, const TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;

  const char* pFieldName = nullptr;
  const char* pTypeStr = nullptr;
  const int32_t fieldTypeCnt = static_cast<int32_t>(sizeof(pFieldTypeStr) / sizeof(pFieldTypeStr[0]));
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
    pTabInfo->GetFieldRealInfo(i, &fieldType);
    pTypeStr = pFieldTypeStr[0];
    pTabInfo->GetFieldIsKey(i, &isKey);
    if (fieldType > 0 && fieldType < fieldTypeCnt)
      pTypeStr = pFieldTypeStr[fieldType];

    DBVAL_ELE_SET_STRING(vals, 0, pTabName, tabNameLen);
    DBVAL_ELE_SET_STRING(vals, 1, pFieldName, strlen(pFieldName));
    DBVAL_ELE_SET_STRING(vals, 2, pTypeStr, strlen(pTypeStr));
    DBVAL_ELE_SET_BOOL(vals, 3, isKey);

    retVal = pQuery->AppendData(vals, valCnt, nullptr);
    if (retVal != PdbE_OK)
      return retVal;

    if (pQuery->GetIsFullFlag())
      return PdbE_OK;
  }

  return PdbE_OK;
}

PdbErr_t TableSet::QueryColumn(IQuery* pQuery)
{
  PdbErr_t retVal = PdbE_OK;
  RefUtil tabRef;
  RefUtil tabInfoRef;
  std::list<uint64_t> tabCrcList;

  retVal = QueryTableColumn(pQuery, &sysConfigInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pQuery->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pQuery, &sysUserInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pQuery->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pQuery, &sysTableInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pQuery->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pQuery, &sysColumnInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pQuery->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pQuery, &sysDataFileInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pQuery->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pQuery, &sysDevInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pQuery->GetIsFullFlag())
    return PdbE_OK;

  retVal = QueryTableColumn(pQuery, &sysConnInfo_);
  if (retVal != PdbE_OK)
    return retVal;

  if (pQuery->GetIsFullFlag())
    return PdbE_OK;

  if (pQuery->GetIsFullFlag())
    return PdbE_OK;

  GetAllTable(tabCrcList);
  for (auto crcIter = tabCrcList.begin(); crcIter != tabCrcList.end(); crcIter++)
  {
    PDBTable* pTab = GetTable(*crcIter, &tabRef);
    if (pTab != nullptr)
    {
      const TableInfo* pTabInfo = pTab->GetTableInfo(&tabInfoRef);
      retVal = QueryTableColumn(pQuery, pTabInfo);
      if (retVal != PdbE_OK)
        return retVal;

      if (pQuery->GetIsFullFlag())
        break;
    }
  }

  return PdbE_OK;
}

PdbErr_t TableSet::QueryDev(IQuery* pQuery)
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
      retVal = pTab->QueryDev(pQuery);
      if (retVal != PdbE_OK)
        return retVal;

      if (pQuery->GetIsFullFlag())
        return PdbE_OK;
    }
  }

  return PdbE_OK;
}


PdbErr_t TableSet::SyncDirtyPages(bool syncAll)
{
  RefUtil tabRef;
  std::list<uint64_t> tabCrcList;

  GetAllTable(tabCrcList);
  if (syncAll)
    pGlbCommitLog->MakeSyncPoint();

  for (auto crcIter = tabCrcList.begin(); crcIter != tabCrcList.end(); crcIter++)
  {
    PDBTable* pTab = GetTable(*crcIter, &tabRef);
    if (pTab != nullptr)
    {
      pTab->SyncDirtyPages(syncAll);
    }
  }

  if (syncAll)
    pGlbCommitLog->CommitSyncPoint();

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

    if (glbCancelCompTask || !glbRunning)
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

int32_t TableSet::GetDirtySizeMB()
{
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

  return static_cast<int32_t>((dirtyPageCnt * NORMAL_PAGE_SIZE) / PDB_MB_BYTES(1));
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

PdbErr_t TableSet::QuerySysTable(const char* pTabName, const QueryParam* pQueryParam, 
  int32_t userRole, std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  PdbErr_t retVal = PdbE_OK;
  uint64_t tabNameCrc = StringTool::CRC64NoCase(pTabName);

  if (tabNameCrc == systab_sysuser_crc_
    || tabNameCrc == systab_connection_crc_
    || tabNameCrc == systab_sysconfig_crc_)
  {
    if (userRole != PDB_ROLE::ROLE_ADMIN)
      return PdbE_OPERATION_DENIED;
  }

  if (pQueryParam->pTagList_ == nullptr)
    return PdbE_SQL_ERROR;

  bool groupQuery = false;
  const std::vector<TargetItem>* pTagList = pQueryParam->pTagList_->GetTargetList();
  for (auto tagIt = pTagList->begin(); tagIt != pTagList->end(); tagIt++)
  {
    if (IncludeAggFunction(tagIt->first))
    {
      groupQuery = true;
      break;
    }
  }

  IQuery* pQuery = nullptr;
  if (groupQuery)
    pQuery = new QueryGroupAll();
  else
    pQuery = new QueryRaw();

  do {
    if (tabNameCrc == systab_sysuser_crc_)
    {
      retVal = pQuery->BuildQuery(pQueryParam, &sysUserInfo_);
      if (retVal != PdbE_OK)
        break;

      retVal = pGlbUser->Query(pQuery);
    }
    else if (tabNameCrc == systab_systable_crc_)
    {
      retVal = pQuery->BuildQuery(pQueryParam, &sysTableInfo_);
      if (retVal != PdbE_OK)
        break;

      retVal = pGlbTabCfg->QueryTable(pQuery);
    }
    else if (tabNameCrc == systab_sysdatafile_crc_)
    {
      retVal = pQuery->BuildQuery(pQueryParam, &sysDataFileInfo_);
      if (retVal != PdbE_OK)
        break;

      retVal = pGlbTabCfg->QueryPart(pQuery);
    }
    else if (tabNameCrc == systab_syscolumn_crc_)
    {
      retVal = pQuery->BuildQuery(pQueryParam, &sysColumnInfo_);
      if (retVal != PdbE_OK)
        break;

      retVal = QueryColumn(pQuery);
    }
    else if (tabNameCrc == systab_connection_crc_)
    {
      retVal = pQuery->BuildQuery(pQueryParam, &sysConnInfo_);
      if (retVal != PdbE_OK)
        break;

      retVal = pGlbServerConnction->QueryConn(pQuery);
    }
    else if (tabNameCrc == systab_sysconfig_crc_)
    {
      retVal = pQuery->BuildQuery(pQueryParam, &sysConfigInfo_);
      if (retVal != PdbE_OK)
        break;

      retVal = pGlbSysCfg->Query(pQuery);
    }
    else if (tabNameCrc == systab_sysdev_crc_)
    {
      retVal = pQuery->BuildQuery(pQueryParam, &sysDevInfo_);
      if (retVal != PdbE_OK)
        break;

      retVal = QueryDev(pQuery);
    }
    else
    {
      retVal = PdbE_TABLE_NOT_FOUND;
    }
  } while (false);

  if (retVal == PdbE_OK)
  {
    pQuery->GetResult(resultData, pFieldCnt, pRecordCnt);
  }

  delete pQuery;
  return retVal;
}

PdbErr_t TableSet::QueryVariable(const QueryParam* pQueryParam, 
  std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt)
{
  PdbErr_t retVal = PdbE_OK;
  TableInfo tabInfo;
  Arena arena;
  if (pQueryParam == nullptr)
    return PdbE_SQL_ERROR;

  if (pQueryParam->pTagList_ == nullptr)
    return PdbE_SQL_ERROR;

  QueryRaw* pQuery = new QueryRaw();

  do {
    retVal = pQuery->BuildQuery(pQueryParam, &tabInfo);
    if (retVal != PdbE_OK)
      break;

    retVal = pQuery->AppendData(nullptr, 0, nullptr);
  } while (false);

  if (retVal == PdbE_OK)
    pQuery->GetResult(resultData, pFieldCnt, pRecordCnt);

  delete pQuery;
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
