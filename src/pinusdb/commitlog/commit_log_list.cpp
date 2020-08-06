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
#include "port/env.h"
#include "util/coding.h"

CommitLogList::CommitLogList()
{
  nextFileCode_ = 1;
  curSyncPos_ = 0;
  nextSyncPos_ = 0;

  pWriteLog_ = nullptr;
}

CommitLogList::~CommitLogList()
{
}

PdbErr_t CommitLogList::Init(const char* pLogPath)
{
  PdbErr_t retVal = PdbE_OK;
  if (pLogPath == nullptr)
    return PdbE_INVALID_PARAM;

  logPath_ = pLogPath;
  retVal = InitLogList();
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("CommitLogList::Init failed to init log list, {}", retVal);
    return retVal;
  }

  if (logFileVec_.size() > 0)
  {
    logFileVec_.back()->RecoverPoint(&curSyncPos_);
  }

  nextSyncPos_ = curSyncPos_;
  return PdbE_OK;
}

PdbErr_t CommitLogList::AppendRec(uint64_t tabCrc, uint32_t metaCrc,
  int recType, int64_t devId, const char* pRec, size_t recLen)
{
  PdbErr_t retVal = PdbE_OK;
  char recHead[kRecDataLen];
  recHead[4] = static_cast<char>(recType);
  Coding::FixedEncode16((recHead + 5), static_cast<uint16_t>(recLen + kRecDataLen));
  Coding::FixedEncode64((recHead + kRecHead), tabCrc);
  Coding::FixedEncode32((recHead + kRecHead + 8), metaCrc);
  Coding::FixedEncode64((recHead + kRecHead + 12), devId);
  uint64_t crc64 = StringTool::CRC64((recHead + 4), (kRecDataLen - 4));
  crc64 = StringTool::CRC64(pRec, recLen, 0, crc64);
  Coding::FixedEncode32(recHead, CRC64_TO_CRC32(crc64));

  std::unique_lock<std::mutex> logLock(logMutex_);
  if (pWriteLog_ == nullptr)
  {
    retVal = NewLogFile();
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("CommitLogList::AppendRec create commit log file failed, {}", retVal);
      return retVal;
    }
  }

  retVal = pWriteLog_->AppendRecord(recHead, pRec, recLen);
  if (retVal == PdbE_LOGFILE_FULL)
  {
    pWriteLog_->CloseWrite();
    pWriteLog_ = nullptr;
    retVal = NewLogFile();
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("CommitLogList::AppendRec create commit log file failed, {}", retVal);
      return retVal;
    }
    retVal = pWriteLog_->AppendRecord(recHead, pRec, recLen);
  }

  if (retVal != PdbE_OK)
  {
    LOG_ERROR("CommitLogList::AppendRec append commit log failed, {}", retVal);
  }

  return retVal;
}

PdbErr_t CommitLogList::GetRedoRecList(std::vector<LogRecInfo>& recList, char* pRedoBuf, size_t bufSize)
{
  PdbErr_t retVal = PdbE_OK;
  CommitLogFile* pLogFile;
  recList.clear();

  while (true)
  {
    pLogFile = nullptr;

    do {
      std::unique_lock<std::mutex> vecLock(vecMutex_);
      for (auto logIt = logFileVec_.begin(); logIt != logFileVec_.end(); logIt++)
      {
        if (nextSyncPos_ < (*logIt)->GetFileEndPos())
        {
          pLogFile = *logIt;
          break;
        }
      }
    } while (false);

    if (pLogFile == nullptr)
      return PdbE_OK;

    retVal = pLogFile->ReadRecList(&nextSyncPos_, pRedoBuf, bufSize, recList);
    if (retVal != PdbE_OK)
    {
      nextSyncPos_ = pLogFile->GetFileEndPos();
    }

    if (recList.size() > 0)
      return PdbE_OK;
  }
}

bool CommitLogList::NeedSyncAllPage()
{
  std::unique_lock<std::mutex> vecLock(vecMutex_);
  if (logFileVec_.size() > 0)
  {
    return logFileVec_.back()->GetFileBeginPos() > curSyncPos_;
  }

  return false;
}

void CommitLogList::MakeSyncPoint()
{
  std::unique_lock<std::mutex> vecLock(vecMutex_);
  if (logFileVec_.size() > 0)
  {
    nextSyncPos_ = logFileVec_.back()->GetFileEndPos();
  }
}

void CommitLogList::CommitSyncPoint()
{
  curSyncPos_ = nextSyncPos_;
  AppendPoint();
  RemoveExpiredLog();
}

void CommitLogList::Shutdown()
{
  MakeSyncPoint();
  CommitSyncPoint();
  if (pWriteLog_ != nullptr)
  {
    pWriteLog_->CloseWrite();
    pWriteLog_ = nullptr;
  }
  if (logFileVec_.size() > 0)
  {
    curSyncPos_ = logFileVec_.back()->GetFileEndPos();
  }
  RemoveExpiredLog();
}

void CommitLogList::SyncLogFile()
{
  std::unique_lock<std::mutex> logLock(logMutex_);
  if (pWriteLog_ != nullptr)
  {
    pWriteLog_->Sync();
  }
}

int64_t CommitLogList::GetLogPosition() const
{
  int64_t logPos = 0;
  CommitLogFile* pTmpLog = pWriteLog_;
  if (nextFileCode_ >= 2)
    logPos = nextFileCode_ - 2;

  logPos *= kCmtLogFileSize;
  if (pTmpLog != nullptr)
  {
    logPos += pTmpLog->GetFileSize();
  }

  return logPos;
}

PdbErr_t CommitLogList::NewLogFile()
{
  PdbErr_t retVal = PdbE_OK;
  char buf[16];
  std::string path;
  Env* pEnv = Env::Default();
  uint32_t maxFileCode = nextFileCode_ + 100;

  do {
    sprintf(buf, "/%08d.cmt", nextFileCode_);
    path = logPath_;
    path.append(buf);

    if (!pEnv->FileExists(path.c_str()))
      break;

    nextFileCode_++;
  } while (nextFileCode_ < maxFileCode);

  CommitLogFile* pTmpLog = new CommitLogFile();
  retVal = pTmpLog->CreateLogFile(nextFileCode_, path.c_str(), curSyncPos_);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("failed to create commit log ({}), {}", path.c_str(), retVal);
    delete pTmpLog;
    return retVal;
  }

  nextFileCode_++;
  pWriteLog_ = pTmpLog;
  std::unique_lock<std::mutex> vecLock(vecMutex_);
  logFileVec_.push_back(pTmpLog);
  if (logFileVec_.size() >= 3)
    logFileVec_[(logFileVec_.size() - 3)]->FreeCache();
  return PdbE_OK;
}

bool LogFileComp(const CommitLogFile* pLogA, const CommitLogFile* pLogB)
{
  return pLogA->GetFileEndPos() < pLogB->GetFileEndPos();
}

PdbErr_t CommitLogList::InitLogList()
{
  Env* pEnv = Env::Default();
  PdbErr_t retVal = PdbE_OK;
  int64_t fileCode;
  std::string filePath;
  std::vector<std::string> fileVec;

  retVal = pEnv->GetChildrenFiles(logPath_.c_str(), &fileVec);
  if (retVal != PdbE_OK)
    return retVal;

  for (auto fileIt = fileVec.begin(); fileIt != fileVec.end(); fileIt++)
  {
    if (!StringTool::EndWithNoCase(*fileIt, ".cmt", 4))
      continue;

    if (StringTool::StrToInt64(fileIt->c_str(), fileIt->size() - 4, &fileCode))
    {
      filePath = logPath_;
      filePath.append("/");
      filePath.append(*fileIt);

      CommitLogFile* pLogFile = new CommitLogFile();
      retVal = pLogFile->OpenLogFile(static_cast<uint32_t>(fileCode), filePath.c_str());
      if (retVal != PdbE_OK)
      {
        delete pLogFile;
        if (retVal != PdbE_DATA_LOG_ERROR)
        {
          //PdbE_DATA_LOG_ERROR 错误可以继续，其他错误不可以继续
          return retVal;
        }
        pEnv->DelFile(filePath.c_str());
      }
      else
      {
        logFileVec_.push_back(pLogFile);
      }
    }
  }

  if (logFileVec_.size() > 0)
  {
    std::sort(logFileVec_.begin(), logFileVec_.end(), LogFileComp);
    nextFileCode_ = logFileVec_.back()->GetFileCode() + 1;
  }

  return PdbE_OK;
}

PdbErr_t CommitLogList::AppendPoint()
{
  PdbErr_t retVal = PdbE_OK;

  std::unique_lock<std::mutex> logLock(logMutex_);
  if (pWriteLog_ == nullptr)
  {
    retVal = NewLogFile();
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("CommitLogList::AppendPoint create commit log file failed, {}", retVal);
      return retVal;
    }
  }

  retVal = pWriteLog_->AppendSync(curSyncPos_);
  if (retVal == PdbE_LOGFILE_FULL)
  {
    pWriteLog_->CloseWrite();
    pWriteLog_ = nullptr;
    retVal = NewLogFile();
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("CommitLogList::AppendPoint create commit log file failed, {}", retVal);
      return retVal;
    }
    retVal = pWriteLog_->AppendSync(curSyncPos_);
  }

  LOG_DEBUG("append commit point syncPos({})", curSyncPos_);
  return retVal;
}

void CommitLogList::RemoveExpiredLog()
{
  Env* pEnv = Env::Default();
  CommitLogFile* pLogFile;
  uint64_t minPos = curSyncPos_;

  while (true)
  {
    pLogFile = nullptr;

    do {
      std::unique_lock<std::mutex> vecLock(vecMutex_);
      if (logFileVec_.empty())
        break;

      if (!logFileVec_.front()->GetFileReadOnly())
        break;

      if (logFileVec_.front()->GetFileEndPos() <= minPos)
      {
        pLogFile = logFileVec_.front();
        logFileVec_.erase(logFileVec_.begin());
      }
    } while (false);

    if (pLogFile == nullptr)
      break;

    std::string filePath = pLogFile->GetFilePath();
    delete pLogFile;
    pEnv->DelFile(filePath.c_str());
    LOG_DEBUG("delete commitlog ({})", filePath.c_str());
  }
}

