#pragma once
#include "internal.h"
#include "expr/expr_item.h"
#include "table/table_info.h"
#include "storage/data_part.h"
#include "table/devid_table.h"
#include "util/ref_util.h"
#include "expr/insert_sql.h"
#include <atomic>
#include <vector>

class PDBTable
{
public:
  PDBTable();
  ~PDBTable();

  const TableInfo* GetTableInfo() const { return &tabInfo_; }

  PdbErr_t OpenTable(uint32_t tabCode, const char* pTabName);
  PdbErr_t Close();
  PdbErr_t RecoverDW();
  PdbErr_t OpenDataPart(int32_t partCode, bool isNormalPart);
  PdbErr_t DetachPart(int32_t partCode);
  PdbErr_t DropPart(int32_t partCode);
  void CancelDumpTask();

  PdbErr_t AttachPart(const char* pPartDate, int fileType);
  PdbErr_t DropTable();

  PdbErr_t Insert(IInsertObj* pInsertObj, bool errBreak, std::list<PdbErr_t>& resultList);
  PdbErr_t InsertByDataLog(int64_t devId, int64_t tstamp, const uint8_t* pRecBg, size_t recLen);
  PdbErr_t Query(DataTable* pResultTable, const QueryParam* pQueryParam);

  size_t GetDevCnt() const { return devTable_.GetDevCnt(); }
  PdbErr_t AddDev(int64_t devId, PdbStr devName, PdbStr expand);
  PdbErr_t FlushDev();
  PdbErr_t DelDev(const ConditionFilter* pCondition);
  PdbErr_t QueryDev(IResultFilter* pFilter);

  PdbErr_t DumpPartToComp();
  PdbErr_t UnMapCompressData();

  //同步脏数据页
  PdbErr_t SyncDirtyPages(bool syncAll);
  size_t GetDirtyPageCnt();

  uint32_t GetTabCode() { return tabCode_; }
  uint32_t GetFieldCrc() { return fieldCrc_; }
  void AddRef() { refCnt_.fetch_add(1); }
  void MinusRef() { refCnt_.fetch_sub(1); }
  int GetRefCnt() { return refCnt_; }

private:
  PdbErr_t QueryLast(std::list<int64_t>& devIdList, int64_t minTstamp, 
    int64_t maxTstamp, IResultFilter* pFilter, uint64_t queryTimeOut);
  PdbErr_t QueryFirst(std::list<int64_t>& devIdList, int64_t minTstamp, 
    int64_t maxTstamp, IResultFilter* pFilter, uint64_t queryTimeOut);
  PdbErr_t QueryAsc(std::list<int64_t>& devIdList, int64_t minTstamp, 
    int64_t maxTstamp, IResultFilter* pFilter, uint64_t queryTimeOut);
  PdbErr_t QueryDesc(std::list<int64_t>& devIdList, int64_t minTstamp, 
    int64_t maxTstamp, IResultFilter* pFilter, uint64_t queryTimeOut);

private:
  void GetDataPartEqualOrGreat(int32_t partCode, bool includeEqual, DataPartRef* pPartRef);
  void GetDataPartEqualOrLess(int32_t partCode, bool includeEqual, DataPartRef* pPartRef);

  DataPart* GetDataPart(int32_t partCode, DataPartRef* pPartRef);

  int _GetDataPartPos(int32_t partCode);

  PdbErr_t GetOrCreateNormalPart(int32_t partCode, DataPartRef* pPartRef);
  PdbErr_t BuildPartPath(int32_t partCode, bool isNormal, bool createParent,
    std::string& partDateStr, std::string& idxPath, std::string& dataPath);

  DataPart* DelOrReplacePart(int32_t partCode, DataPart* pNewPart);

private:
  std::atomic_int refCnt_;
  std::atomic_int dumpPartCode_;

  DevIDTable devTable_;
  TableInfo tabInfo_;
  uint64_t tabCrc_;
  uint32_t fieldCrc_;
  uint32_t tabCode_;

  std::string tabName_;
  std::string devPath_;
  std::string dwPath_;
  OSFile dwFile_;

  std::mutex syncMutex_;

  std::mutex partMutex_;
  std::vector<DataPart*> partVec_;
};
