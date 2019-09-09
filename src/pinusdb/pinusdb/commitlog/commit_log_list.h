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

  bool Init(const char* pLogPath, bool enableRep);

  PdbErr_t AppendData(uint64_t tabCrc, uint32_t fieldCrc,
    bool isRep, const CommitLogBlock* pLogBlock);

  void GetCurLogPos(uint64_t* pDataLogPos);
  void SetSyncPos(uint64_t syncPos);
  void SetRepPos(uint64_t repPos);

  PdbErr_t GetRedoData(uint64_t *pTabCrc, uint32_t *pFieldCrc,
    size_t* pRecCnt, size_t* pDataLen, uint8_t* pBuf);

  void Shutdown();

private:
  PdbErr_t InitLogFileList(const char* pLogPath);
  PdbErr_t NewLogFile();
  void AppendSyncInfo();

private:
  std::mutex logMutex_;
  bool enableRep_;             // 是否启用组复制
  uint32_t nextFileCode_;      // 下一个文件编号

  uint64_t redoPos_;            // 恢复-位置-用于宕机恢复数据
  uint64_t repPos_;             // 复制-位置-用于将内存中数据写入磁盘
  uint64_t syncPos_;            // 同步-位置-用于与组内数据库同步

  CommitLogFile* pRedoLog_;      // 重做数据日志文件
  CommitLogFile* pRepLog_;       // 复制数据日志文件
  CommitLogFile* pWriteLog_;     // 写日志文件
  std::string logPath_;        // 日志路径
  std::vector<CommitLogFile*> logFileVec_;  //日志文件列表
};




