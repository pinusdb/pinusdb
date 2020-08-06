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

#include <unordered_map>
#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>

#include "pdb.h"
#include "table/table_info.h"
#include "expr/sql_parser.h"
#include "table/table_info.h"
#include "boost/asio.hpp"

class DBImpl
{
private:
  DBImpl();

public:
  virtual ~DBImpl();

  static DBImpl* GetInstance();
  
  PdbErr_t Start();
  void Stop();

  void SyncCmtLog();
  void SyncTask();
  void CompressTask();

private:
  void _SyncCmtLog();
  void _SyncTask();
  void _CompressTask();

private:
  PdbErr_t RecoverDataLog();

private:
  std::thread* pCmtLogTask_; //定时提交Commit日志
  std::thread* pSyncTask_;  //数据同步到数据文件
  std::thread* pCompTask_;  //历史数据压缩
  bool isInit_;

  std::mutex stopMutex_;
  std::condition_variable stopVariable_;

  static DBImpl* dbImpl_;
};

