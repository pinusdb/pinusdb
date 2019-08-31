#pragma once
#include "internal.h"
#include "util/arena.h"
#include "util/skip_list.h"
#include "table/table_info.h"
#include "query/dev_filter.h"
#include "query/result_filter.h"

typedef struct _DevIdPos
{
  int64_t devId_;
  size_t pos_;
}DevIdPos;

class DevIDTable
{
public:
  DevIDTable();
  ~DevIDTable();

  static PdbErr_t Create(const char* pDevPath, const TableInfo* pTabInfo);

  PdbErr_t Open(const char* pPath, const char* pTabName, TableInfo* pTabInfo);
  PdbErr_t Close();

  size_t GetDevCnt() const;
  PdbErr_t DevExist(int64_t devId);
  PdbErr_t AddDev(int64_t devId, PdbStr devName, PdbStr expand);
  PdbErr_t QueryDevId(const DevFilter* pDevFilter, std::list<int64_t>& devIdList);
  PdbErr_t QueryDevId(const DevFilter* pDevFilter, std::list<int64_t>& devIdList,
    size_t queryOffset, size_t queryRecord);
  PdbErr_t QueryDevInfo(const std::string& tabName, IResultFilter* pFilter);
  PdbErr_t DelDev(const std::string& tabName, const ConditionFilter* pCondition);

  void Flush();

private:
  PdbErr_t _MapFile(const char* pPath);
  PdbErr_t _GrowFile();
  PdbErr_t _UnMapFile();
  PdbErr_t _DelDev(int64_t devId);

private:
  HANDLE fileHandle_;
  HANDLE mapHandle_;
  LPVOID pBase_;
  size_t fileSize_;

  //用于插入时判断设备是否存在
  std::mutex insertMutex_;
  std::unordered_set<int64_t> devIdSet_;

  //用于查询数据时筛选设备
  std::mutex queryIdMutex_;
  bool isSortId_;
  std::vector<int64_t> devIdVec_;

  //用于查询设备信息
  std::mutex fileMutex_;
  bool isSortInfo_;
  std::vector<DevIdPos> devIdPosVec_;

  //空闲的位置
  std::list<size_t> freePosList_;
};
