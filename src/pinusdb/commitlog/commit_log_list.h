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
#include "port/os_file.h"
#include "commitlog/commit_log_file.h"

class CommitLogList
{
public:
  CommitLogList();
  ~CommitLogList();

  PdbErr_t Init(const char* pLogPath);
  PdbErr_t AppendRec(uint64_t tabCrc, uint32_t metaCrc, int recType,
    int64_t devId, const char* pRec, size_t recLen);

  PdbErr_t GetRedoRecList(std::vector<LogRecInfo>& recList, char* pRedoBuf, size_t bufSize);
  bool NeedSyncAllPage();

  void MakeSyncPoint();
  void CommitSyncPoint();
  void Shutdown();

  void SyncLogFile();

  int64_t GetLogPosition() const;

private:
  PdbErr_t NewLogFile();
  PdbErr_t InitLogList();
  PdbErr_t AppendPoint();
  void RemoveExpiredLog();

private:
  std::mutex logMutex_;
  uint32_t nextFileCode_;      // 下一个文件编号

  uint64_t curSyncPos_;         // 同步-位置-用于将内存中数据写入磁盘
  uint64_t nextSyncPos_;

  CommitLogFile* pWriteLog_;     // 写日志文件

  std::string logPath_;        // 日志路径
  std::mutex vecMutex_;
  std::vector<CommitLogFile*> logFileVec_;  //日志文件列表
};




