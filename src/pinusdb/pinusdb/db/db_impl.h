#pragma once

#include <unordered_map>
#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>

#include "pdb.h"
#include "table/db_obj.h"
#include "table/table_info.h"
#include "expr/sql_parser.h"
#include "query/data_table.h"
#include "query/result_filter.h"
#include "table/table_info.h"
#include "query/result_filter_raw.h"

class DBImpl
{
private:
  DBImpl();

public:
  virtual ~DBImpl();

  static DBImpl* GetInstance();
  
  PdbErr_t Start();
  void StopTask();
  void Stop();

  void SyncTask();
  void CompressTask();

private:
  void _SyncTask();
  void _CompressTask();

private:
  PdbErr_t RecoverDataLog();

private:
  std::thread* pSyncTask_;  //数据同步到数据文件
  std::thread* pRepTask_;   //数据同步到对端数据库
  std::thread* pCompTask_;  //历史数据压缩
  bool isInit_;

  bool running_;
  std::mutex stopMutex_;
  std::condition_variable stopVariable_;

  static DBImpl* dbImpl_;
};

