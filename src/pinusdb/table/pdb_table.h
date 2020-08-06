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
#include "table/table_info.h"
#include "storage/data_part.h"
#include "table/devid_table.h"
#include "util/ref_util.h"
#include "expr/insert_sql.h"
#include <atomic>
#include <vector>

class PDBTable : public RefObj
{
public:
  PDBTable();
  ~PDBTable();

  TableInfo* GetTableInfo(RefUtil* pRefUtil);

  PdbErr_t AlterTable(const std::vector<ColumnItem*>& colItemVec);
  PdbErr_t OpenTable(uint32_t tabCode, const char* pTabName);
  PdbErr_t Close();
  PdbErr_t RecoverDW();
  PdbErr_t OpenDataPart(uint32_t partCode, bool isNormalPart);
  PdbErr_t DetachPart(uint32_t partCode);
  PdbErr_t DropPart(uint32_t partCode);
  void CancelDumpTask();

  PdbErr_t AttachPart(const char* pPartDate, int fileType);
  PdbErr_t DropTable();

  PdbErr_t Insert(InsertSql* pInsertSql, bool errBreak, std::list<PdbErr_t>& resultList);
  PdbErr_t InsertByDataLog(uint32_t metaCode, int64_t devId, int64_t tstamp, const char* pRec, size_t recLen);
  PdbErr_t InsertByReplicate(std::vector<LogRecInfo>& recVec, size_t beginIdx);
  PdbErr_t ExecQuery(const QueryParam* pQueryParam, 
    std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt);
  PdbErr_t ExecQuerySnapshot(const QueryParam* pQueryParam,
    std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt);

  size_t GetDevCnt() const { return devTable_.GetDevCnt(); }
  PdbErr_t AddDev(int64_t devId, PdbStr devName, PdbStr expand);
  PdbErr_t FlushDev();
  PdbErr_t DelDev(const ConditionFilter* pCondition);
  PdbErr_t QueryDev(IQuery* pQuery);

  PdbErr_t DumpPartToComp();
  PdbErr_t UnMapCompressData();

  //同步脏数据页
  PdbErr_t SyncDirtyPages(bool syncAll);
  size_t GetDirtyPageCnt();

  uint32_t GetTabCode() { return tabCode_; }

private:
  PdbErr_t ExecQueryGroupAll(const QueryParam* pQueryParam, 
    std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt);
  PdbErr_t ExecQueryGroupDevId(const QueryParam* pQueryParam, 
    std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt);
  PdbErr_t ExecQueryGroupTstamp(const QueryParam* pQueryParam, 
    std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt);
  PdbErr_t ExecQueryRawData(const QueryParam* pQueryParam, 
    std::string& resultData, uint32_t* pFieldCnt, uint32_t* pRecordCnt);

  PdbErr_t QueryLast(std::list<int64_t>& devIdList, IQuery* pQuery, uint64_t queryTimeOut);
  PdbErr_t QueryFirst(std::list<int64_t>& devIdList, IQuery* pQuery, uint64_t queryTimeOut);
  PdbErr_t QueryAsc(std::list<int64_t>& devIdList, IQuery* pQuery, uint64_t queryTimeOut);
  PdbErr_t QueryDesc(std::list<int64_t>& devIdList, IQuery* pQuery, uint64_t queryTimeOut);
  PdbErr_t QuerySnapshotData(std::list<int64_t>& devIdList, IQuery* pQuery, uint64_t queryTimeOut);

private:
  void GetDataPartEqualOrGreat(uint32_t partCode, bool includeEqual, RefUtil* pPartRef);
  void GetDataPartEqualOrLess(uint32_t partCode, bool includeEqual, RefUtil* pPartRef);

  DataPart* GetDataPart(uint32_t partCode, RefUtil* pPartRef);

  int _GetDataPartPos(uint32_t partCode);

  PdbErr_t GetOrCreateNormalPart(uint32_t partCode, RefUtil* pPartRef);
  PdbErr_t BuildPartPath(uint32_t partCode, bool isNormal, bool createParent,
    std::string& partDateStr, std::string& idxPath, std::string& dataPath);

  DataPart* DelOrReplacePart(uint32_t partCode, DataPart* pNewPart);

private:
  std::atomic_int dumpPartCode_;

  DevIDTable devTable_;
  TableInfo* pTabInfo_;
  uint64_t tabCrc_;
  uint32_t tabCode_;

  std::string tabName_;
  std::string devPath_;
  std::string dwPath_;
  OSFile dwFile_;

  std::mutex syncMutex_;

  std::mutex partMutex_;
  std::vector<DataPart*> partVec_;
};
