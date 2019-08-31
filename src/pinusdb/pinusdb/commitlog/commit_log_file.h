#pragma once

#include "port/os_file.h"
#include "commitlog/commit_log_block.h"

#define BLKTYPE_FILE_HEAD      1  //文件头信息
#define BLKTYPE_SYNC_INFO      2
#define BLKTYPE_INSERT_CLI     3  // 数据插入由客户端发起，需要同步到同组的数据库
#define BLKTYPE_INSERT_REP     4  // 数据插入由同组数据库发起，不需要同步到同组的数据库

#define DATA_LOG_FILE_VER      2
#define DATA_LOG_FILE_SIZE     ((uint64_t)512 * 1024 * 1024)  //每个日志文件最大512M

typedef struct _BlkHdr
{
  int32_t  blkType_;         // 块类型
  uint32_t hdrCrc_;          // 头部校验
}BlkHdr;

typedef struct _LogFileHdr {
  uint32_t logFileVer_;      // 文件版本
  uint32_t logFileCode_;     // 文件编号
  char padding_[16];
  BlkHdr blkHdr_;
}LogFileHdr;

typedef struct _LogBlkHdr {
  uint64_t tabCrc_;          // 表
  uint32_t fieldCrc_;        // 字段
  int32_t  recCnt_;          // 记录条数
  int32_t  dataLen_;         // 数据长度
  uint32_t dataCrc_;         // 数据校验
  BlkHdr   blkHdr_;          // 块头部信息
}LogBlkHdr;

typedef struct _SyncBlkHdr {
  uint64_t repPos_;      // 组文件位置
  uint64_t syncPos_;     // 数据同步位置
  uint64_t padding_;
  BlkHdr   blkHdr_;      // 块头部信息
}SyncBlkHdr;

class CommitLogFile
{
public:
  CommitLogFile();
  ~CommitLogFile();

  PdbErr_t OpenLog(uint32_t fileCode, const char* pPath);
  PdbErr_t NewLog(uint32_t fileCode, const char* pPath, uint64_t grpPos, uint64_t syncPos);
  PdbErr_t Close();
  void Sync();
  uint32_t GetFileCode() const { return fileCode_; }
  uint64_t GetCurPos() const { return curPos_; }
  std::string GetFilePath() const { return filePath_; }

  PdbErr_t AppendData(const LogBlkHdr* pLogHdr, const CommitLogBlock* pLogBlock);
  PdbErr_t AppendSync(uint64_t repPos, uint64_t syncPos);
  PdbErr_t ReadBuf(uint8_t* pBuf, int readLen, int64_t offset);
  PdbErr_t RecoverPoint(uint64_t* pRepPos, uint64_t* pSyncPos);

private:
  std::string filePath_;
  OSFile logFile_;
  uint64_t curPos_;
  uint64_t syncLen_;
  uint64_t syncTime_;
  uint32_t fileCode_;
};
