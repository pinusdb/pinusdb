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

#include "commitlog/commit_log_list.h"
#include <algorithm>
#include "util/log_util.h"
#include "util/string_tool.h"

CommitLogList::CommitLogList()
{
  enableRep_ = false;
  nextFileCode_ = 1;

  redoPos_ = 0;
  repPos_ = 0;
  syncPos_ = 0;

  pRedoLog_ = nullptr;
  pRepLog_ = nullptr;
  pWriteLog_ = nullptr;
}

CommitLogList::~CommitLogList()
{
}

bool CommitLogList::Init(const char* pLogPath, bool enableRep)
{
  PdbErr_t retVal = PdbE_OK;
  enableRep_ = enableRep_;
  nextFileCode_ = 1;

  redoPos_ = 0;
  repPos_ = 0;
  syncPos_ = 0;

  pRedoLog_ = nullptr;
  pRepLog_ = nullptr;
  pWriteLog_ = nullptr;
  logPath_ = pLogPath;

  retVal = InitLogFileList(pLogPath);
  if (retVal != PdbE_OK)
  {
    LOG_INFO("failed to init datalog list, path: ({}), err: {}", pLogPath, retVal);
    return false;
  }
  
  //初始化当前状态
  if (!logFileVec_.empty())
  {
    CommitLogFile* pLastLog = logFileVec_[logFileVec_.size() - 1];
    pLastLog->RecoverPoint(&repPos_, &syncPos_);
  }

  redoPos_ = syncPos_;
  return true;
}

PdbErr_t CommitLogList::AppendData(uint64_t tabCrc, uint32_t fieldCrc,
  bool isRep, const CommitLogBlock* pLogBlock)
{
  PdbErr_t retVal = PdbE_OK;
  LogBlkHdr logHdr;
  int32_t dataLen = 0;
  uint64_t dataCrc = 0;
  memset(&logHdr, 0, sizeof(LogBlkHdr));

  if (pLogBlock == nullptr)
    return PdbE_INVALID_PARAM;

  const std::list<PdbBlob>& recList = pLogBlock->GetRecList();
  if (recList.size() == 0)
    return PdbE_INVALID_PARAM;

  for (auto recIt = recList.begin(); recIt != recList.end(); recIt++)
  {
    dataLen += static_cast<int32_t>(recIt->len_);
    dataCrc = StringTool::CRC64(recIt->pBlob_, recIt->len_, 0, dataCrc);
  }

  logHdr.tabCrc_ = tabCrc;
  logHdr.fieldCrc_ = fieldCrc;
  logHdr.recCnt_ = static_cast<int32_t>(recList.size());
  logHdr.dataLen_ = dataLen;
  logHdr.dataCrc_ = CRC64_TO_CRC32(dataCrc);
  logHdr.blkHdr_.blkType_ = isRep ? BLKTYPE_INSERT_REP : BLKTYPE_INSERT_CLI;
  logHdr.blkHdr_.hdrCrc_ = StringTool::CRC32(&logHdr, (sizeof(LogBlkHdr) - 4));

  std::unique_lock<std::mutex> logLock(logMutex_);
  if (pWriteLog_ == nullptr)
  {
    retVal = NewLogFile();
    if (retVal != PdbE_OK)
    {
      LOG_INFO("failed to create datalog file, err:{}", retVal);
      return retVal;
    }
  }

  if ((pWriteLog_->GetCurPos() + dataLen + sizeof(LogBlkHdr)) > DATA_LOG_FILE_SIZE)
  {
    pWriteLog_->Sync();
    retVal = NewLogFile();
    if (retVal != PdbE_OK)
    {
      LOG_INFO("failed to crate datalog file, err: {}", retVal);
      return retVal;
    }
  }

  return pWriteLog_->AppendData(&logHdr, pLogBlock);
}

void CommitLogList::GetCurLogPos(uint64_t* pDataLogPos)
{
  PdbErr_t retVal = PdbE_OK;
  if (pDataLogPos != nullptr)
    *pDataLogPos = 0;

  std::unique_lock<std::mutex> logLock(logMutex_);
  if (logFileVec_.size() > 0)
  {
    CommitLogFile* pTmpLog = logFileVec_[logFileVec_.size() - 1];
    if (pDataLogPos != nullptr)
      *pDataLogPos = (pTmpLog->GetFileCode() * DATA_LOG_FILE_SIZE) + pTmpLog->GetCurPos();
  }
}

void CommitLogList::SetSyncPos(uint64_t syncPos)
{
  syncPos_ = syncPos;
  AppendSyncInfo();
}

void CommitLogList::SetRepPos(uint64_t repPos)
{
  repPos_ = repPos;
  AppendSyncInfo();
}

PdbErr_t CommitLogList::GetRedoData(uint64_t *pTabCrc, uint32_t *pFieldCrc,
  size_t* pRecCnt, size_t* pDataLen, uint8_t* pBuf)
{
  PdbErr_t retVal = PdbE_OK;
  LogBlkHdr logHdr;

  if (pTabCrc == nullptr || pFieldCrc == nullptr 
    || pRecCnt == nullptr || pDataLen == nullptr || pBuf == nullptr)
    return PdbE_INVALID_PARAM;

  while (true)
  {
    uint32_t redoFileCode = static_cast<uint32_t>(redoPos_ / DATA_LOG_FILE_SIZE);
    do {
      if (pRedoLog_ != nullptr && pRedoLog_->GetFileCode() == redoFileCode)
        break;

      pRedoLog_ = nullptr;
      for (auto logIt = logFileVec_.begin(); logIt != logFileVec_.end(); logIt++)
      {
        uint32_t tmpCode = (*logIt)->GetFileCode();
        if (tmpCode >= redoFileCode)
        {
          if (tmpCode > redoFileCode)
            redoPos_ = (uint64_t)tmpCode * DATA_LOG_FILE_SIZE + sizeof(LogFileHdr);

          pRedoLog_ = *logIt;
          break;
        }
      }
    } while (false);

    if (pRedoLog_ == nullptr)
      return PdbE_END_OF_DATALOG;

    while (true)
    {
      uint64_t tmpPos = redoPos_ % DATA_LOG_FILE_SIZE;
      if (tmpPos + sizeof(LogBlkHdr) >= DATA_LOG_FILE_SIZE) {
        //下一个日志文件
        redoPos_ = ((redoPos_ / DATA_LOG_FILE_SIZE) + 1) * DATA_LOG_FILE_SIZE;
        break;
      }

      retVal = pRedoLog_->ReadBuf((uint8_t*)&logHdr, sizeof(LogBlkHdr), tmpPos);
      if (retVal != PdbE_OK)
      {
        //下一个日志文件
        redoPos_ = ((redoPos_ / DATA_LOG_FILE_SIZE) + 1) * DATA_LOG_FILE_SIZE;
        break;
      }

      if (StringTool::CRC32(&logHdr, (sizeof(LogBlkHdr) - 4))
        != logHdr.blkHdr_.hdrCrc_)
      {
        //下一个日志文件
        redoPos_ = ((redoPos_ / DATA_LOG_FILE_SIZE) + 1) * DATA_LOG_FILE_SIZE;
        break;
      }

      redoPos_ += sizeof(SyncBlkHdr);
      tmpPos = redoPos_ % DATA_LOG_FILE_SIZE;
      if (logHdr.blkHdr_.blkType_ == BLKTYPE_SYNC_INFO
        || logHdr.blkHdr_.blkType_ == BLKTYPE_FILE_HEAD)
      {
        continue;
      }

      if (logHdr.blkHdr_.blkType_ != BLKTYPE_INSERT_CLI
        && logHdr.blkHdr_.blkType_ != BLKTYPE_INSERT_REP)
      {
        //下一个日志文件
        redoPos_ = ((redoPos_ / DATA_LOG_FILE_SIZE) + 1) * DATA_LOG_FILE_SIZE;
        break;
      }

      //一次最多写入1000条数据，每条数据最多8K, 所以一次读取最多8M
      if (logHdr.dataLen_ >= PDB_MB_BYTES(8))
      {
        //数据错误, 下一个日志文件
        redoPos_ = ((redoPos_ / DATA_LOG_FILE_SIZE) + 1) * DATA_LOG_FILE_SIZE;
        break;
      }

      if (tmpPos + logHdr.dataLen_ > DATA_LOG_FILE_SIZE)
      {
        //数据错误, 下一个日志文件
        redoPos_ = ((redoPos_ / DATA_LOG_FILE_SIZE) + 1) * DATA_LOG_FILE_SIZE;
        break;
      }

      retVal = pRedoLog_->ReadBuf(pBuf, logHdr.dataLen_, tmpPos);
      if (retVal != PdbE_OK)
      {
        //数据错误, 下一个日志文件
        redoPos_ = ((redoPos_ / DATA_LOG_FILE_SIZE) + 1) * DATA_LOG_FILE_SIZE;
        break;
      }

      if (StringTool::CRC32(pBuf, logHdr.dataLen_) != logHdr.dataCrc_)
      {
        //当前数据校验失败,下一个日志文件
        redoPos_ = ((redoPos_ / DATA_LOG_FILE_SIZE) + 1) * DATA_LOG_FILE_SIZE;
        break;
      }

      redoPos_ += logHdr.dataLen_;

      *pTabCrc = logHdr.tabCrc_;
      *pFieldCrc = logHdr.fieldCrc_;
      *pRecCnt = logHdr.recCnt_;
      *pDataLen = logHdr.dataLen_;
      return PdbE_OK;
    }
  }

  return PdbE_END_OF_DATALOG;
}

void CommitLogList::Shutdown()
{
  if (!enableRep_)
  {
    std::unique_lock<std::mutex> logLock(logMutex_);
    for (auto fileIt = logFileVec_.begin(); fileIt != logFileVec_.end(); fileIt++)
    {
      std::string logPath = (*fileIt)->GetFilePath();
      (*fileIt)->Close();
      FileTool::RemoveFile(logPath.c_str());
    }
  }
}

#define ValidateLogFile(pFile, fileCode)  do { \
  fileCode = -1; \
  if (strlen(pFile) == 12) \
  {  \
    fileCode = 0; \
    for (int i = 0; i < 8; i++)  \
    {  \
      if (pFile[i] >= '0' || pFile[i] <= '9') \
      { \
        fileCode = fileCode * 10 + pFile[i] - '0'; \
      }  \
      else { \
        fileCode = -1; \
        break; \
      }  \
    }  \
  }  \
}while(false)

PdbErr_t CommitLogList::InitLogFileList(const char* pLogPath)
{
  PdbErr_t retVal = PdbE_OK;
  WIN32_FIND_DATA findFileData;
  HANDLE findHandle = INVALID_HANDLE_VALUE;
  int32_t fileCode = 0;
  char pathBuf[MAX_PATH];

  std::vector<int32_t> logFileVec;
  std::vector<std::string> errFileVec;

  sprintf_s(pathBuf, "%s/*.cmt", pLogPath);
  findHandle = FindFirstFile(pathBuf, &findFileData);
  if (findHandle == INVALID_HANDLE_VALUE)
    return PdbE_OK;

  do {
    ValidateLogFile(findFileData.cFileName, fileCode);
    if (fileCode > 0)
    {
      if (fileCode >= 20000000)
      {
        LOG_ERROR("failed to init datalog list, file code overflow");
        return PdbE_DATA_LOG_ERROR;
      }

      logFileVec.push_back(fileCode);
    }
  } while (FindNextFile(findHandle, &findFileData));
  FindClose(findHandle);


  if (!logFileVec.empty())
  {
    std::sort(logFileVec.begin(), logFileVec.end());

    CommitLogFile* pLogFile = nullptr;
    for (auto fileIt = logFileVec.begin(); fileIt != logFileVec.end(); fileIt++)
    {
      sprintf_s(pathBuf, "%s/%08d.cmt", pLogPath, *fileIt);
      pLogFile = new CommitLogFile();
      retVal = pLogFile->OpenLog(*fileIt, pathBuf);
      if (retVal != PdbE_OK)
      {
        delete pLogFile;
        errFileVec.push_back(pathBuf);

        if (retVal != PdbE_DATA_LOG_ERROR)
        {
          //PdbE_DATA_LOG_ERROR 错误可以继续，其他错误不可以继续
          return retVal;
        }
      }
      else {
        logFileVec_.push_back(pLogFile);
        nextFileCode_ = *fileIt + 1;
      }
    }
  }

  //删除错误的日志文件
  for (auto fileIt = errFileVec.begin(); fileIt != errFileVec.end(); fileIt++)
  {
    FileTool::RemoveFile(fileIt->c_str());
  }

  return PdbE_OK;
}

PdbErr_t CommitLogList::NewLogFile()
{
  PdbErr_t retVal = PdbE_OK;
  char pathBuf[MAX_PATH];
  uint32_t maxFileCode = nextFileCode_ + 100;

  do {
    sprintf_s(pathBuf, "%s/%08d.cmt", logPath_.c_str(), nextFileCode_);
    if (!FileTool::FileExists(pathBuf))
      break;

    nextFileCode_++;
  } while (nextFileCode_ < maxFileCode);

  CommitLogFile* pTmpLog = new CommitLogFile();
  retVal = pTmpLog->NewLog(nextFileCode_, pathBuf, repPos_, syncPos_);

  if (retVal != PdbE_OK)
  {
    LOG_ERROR("failed to create datalog({}), err:{}", pathBuf, retVal);
    delete pTmpLog;
    return retVal;
  }

  nextFileCode_++;
  pWriteLog_ = pTmpLog;
  logFileVec_.push_back(pTmpLog);
  return PdbE_OK;
}

void CommitLogList::AppendSyncInfo()
{
  std::unique_lock<std::mutex> logLock(logMutex_);
  do {
    if (pWriteLog_ == nullptr)
    {
      //NewLogFile 中已写入了Sync信息
      NewLogFile();
      break;
    }

    if ((pWriteLog_->GetCurPos() + sizeof(SyncBlkHdr)) > DATA_LOG_FILE_SIZE)
    {
      //NewLogFile 中已写入了Sync信息
      NewLogFile();
      break;
    }

    pWriteLog_->AppendSync(repPos_, syncPos_);
  } while (false);

  while (logFileVec_.size() > 1)
  {
    CommitLogFile* pLogFile = *(logFileVec_.begin());
    if (syncPos_ < ((pLogFile->GetFileCode() + 1) * DATA_LOG_FILE_SIZE))
      break;
    if (enableRep_ && repPos_ < ((pLogFile->GetFileCode() + 1) * DATA_LOG_FILE_SIZE))
      break;

    logFileVec_.erase(logFileVec_.begin());
    std::string logPath = pLogFile->GetFilePath();
    pLogFile->Close();
    FileTool::RemoveFile(logPath.c_str());
  }
}
