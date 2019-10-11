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

#include <mutex>
#include <unordered_map>
#include "pdb.h"
#include "expr/sql_parser.h"
#include "query/data_table.h"
#include "query/result_filter.h"
#include "table/pdb_table.h"
#include "util/ref_util.h"
#include "expr/insert_sql.h"

class TableSet
{
public:
  TableSet();
  ~TableSet();

  PdbErr_t CreateTable(const CreateTableParam* pTableParam);
  PdbErr_t AlterTable(const CreateTableParam* pTableParam);
  PdbErr_t OpenTable(const char* pTabName);
  PdbErr_t OpenDataPart(const char* pTabName, int partCode, bool isNormalPart);
  PdbErr_t RecoverDW(const char* pTabName);
  PdbErr_t DropTable(const char* pTabName);
  PdbErr_t AttachTable(const char* pTabName);
  PdbErr_t DetachTable(const char* pTabName);
  PdbErr_t AttachFile(const char* pTabName, const char* pPartStr, int fileType);
  PdbErr_t DetachFile(const char* pTabName, int partCode);
  PdbErr_t DropFile(const char* pTabName, int partCode);
  PdbErr_t ExecuteQuery(DataTable* pResultTable, SQLParser* pParser, int32_t userRole);

  PdbErr_t DeleteDev(const DeleteParam* pDeleteParam);
  PDBTable* GetTable(const char* pTabName, RefUtil* pTabRef);
  PDBTable* GetTable(uint64_t tabNameCrc, RefUtil* pTabRef);

  PdbErr_t Insert(InsertSql* pInsertSql, bool errBreak, std::list<PdbErr_t>& resultList);

  PdbErr_t QueryColumn(IResultFilter* pFilter);
  PdbErr_t QueryDev(IResultFilter* pFilter);

  PdbErr_t SyncDirtyPages(bool syncAll);
  PdbErr_t CloseAllTable();

  void DumpToCompress();
  void UnMapCompressData();

  size_t GetDirtyPagePercent();

private:
  PdbErr_t InsertDev(InsertSql* pInsertSql,
    bool errBreak, std::list<PdbErr_t>& resultList);

  PdbErr_t QuerySysTable(const QueryParam* pQueryParam, int32_t userRole, DataTable* pResultTable);
  PdbErr_t QueryVariable(const QueryParam* pQueryParam, DataTable* pResultTable);

  size_t GetTotalDevCnt();
  void GetAllTable(std::list<uint64_t>& tableCrcList);
  void AddTableHandle(uint64_t tabCrc, PDBTable* pTable);
  PDBTable* EraseTable(uint64_t tabCrc);

private:
  size_t maxDev_;
  std::mutex tabMutex_; //针对添加表、删除表、打开表、附加表、分离表
  std::mutex devMutex_; //设备
  std::list<uint32_t> freeCodeList_;

  std::mutex tabMapMutex_;
  std::unordered_map<uint64_t, PDBTable*> tabMap_;

  TableInfo sysUserInfo_;
  TableInfo sysTableInfo_;
  TableInfo sysDataFileInfo_;
  TableInfo sysColumnInfo_;
  TableInfo sysConnInfo_;
  TableInfo sysConfigInfo_;
  TableInfo sysDevInfo_;

  uint64_t systab_sysuser_crc_;
  uint64_t systab_systable_crc_;
  uint64_t systab_sysdatafile_crc_;
  uint64_t systab_syscolumn_crc_;
  uint64_t systab_connection_crc_;
  uint64_t systab_sysconfig_crc_;
  uint64_t systab_sysdev_crc_;
};

