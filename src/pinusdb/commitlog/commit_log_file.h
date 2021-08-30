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

#include "port/env.h"
#include "pdb.h"
#include "internal.h"
#include <vector>
#include <mutex>

#define CMTLOG_TYPE_FILE_HEAD            1  //文件头信息
#define CMTLOG_TYPE_SYNC_INFO            2  //同步信息
#define CMTLOG_TYPE_INSERT_CLIENT        3  // 数据由客户端写入，需要同步到同组的数据库，需要再次镜像
#define CMTLOG_TYPE_INSERT_REPLICATE     4  // 数据由同组数据库写入，不需要同步到同组的数据库，不需要再次镜像
#define CMTLOG_TYPE_INSERT_MIRROR        5  // 数据由镜像写入， 需要同步到同组数据库，需要再次镜像

#define CMTLOG_FILE_VER      3

//crc, type, length
const size_t kRecHead = (4 + 1 + 2);
//fileVer, fileCode, syncPos, repPos, mirrorPos
const size_t kRecFileInfoLen = (kRecHead + 1 + 4 + 8 + 8 + 8);
//tabCrc, metaCode, devid
const size_t kRecDataLen = (kRecHead + 8 + 4 + 8);
//syncPos, repPos, mirrorPos
const size_t kRecSyncLen = (kRecHead + 8 + 8 + 8);
const uint64_t kCmtLogFileSize = (512 * 1024 * 1024);

typedef struct _CacheBlock
{
  size_t cacheSize_;
  size_t cacheOffset_;
  char* pCache_;
}CacheBlock;

class CommitLogFile
{
public:
  CommitLogFile();
  ~CommitLogFile();

  PdbErr_t OpenLogFile(uint32_t fileCode, const char* pPath);
  PdbErr_t CreateLogFile(uint32_t fileCode, const char* pPath, uint64_t syncPos);
  PdbErr_t CloseWrite();
  void FreeCache();

  uint32_t GetFileCode() const { return fileCode_; }
  const char* GetFilePath() const { return filePath_.c_str(); }
  size_t GetFileSize() const { return (fileSize_ + pos_); }
  bool GetFileReadOnly() const { return readOnly_; }
  uint64_t GetFileBeginPos() const;
  uint64_t GetFileEndPos() const;
  
  PdbErr_t ReadRecList(uint64_t* pOffset, char* pBuf, size_t bufSize,
    std::vector<LogRecInfo>& recList);

  PdbErr_t AppendRecord(const char* pRecHead, const char* pRec, size_t recLen);
  PdbErr_t AppendSync(uint64_t syncPos);
  PdbErr_t RecoverPoint(uint64_t* pSyncPos);

  PdbErr_t Sync();

private:
  PdbErr_t Read(size_t offset, char* pBuf, size_t* pBytes);
  PdbErr_t Append(const char* pBuf, size_t bytes);
  PdbErr_t FlushBuffer();

private:
  std::mutex fileMutex_;
  bool readOnly_;
  uint32_t fileCode_;
  std::string filePath_;
  NormalFile* pFile_;

  volatile size_t fileSize_;
  std::vector<CacheBlock> cacheVec_;

  volatile size_t pos_;
  char* pWriteBuf_;
};
