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

#include "commitlog/commit_log_file.h"
#include "util/string_tool.h"
#include "util/log_util.h"
#include "util/date_time.h"
#include "util/coding.h"

#define PDB_MIN_VAL(a,b)            (((a) < (b)) ? (a) : (b))
constexpr const size_t kWritBufferSize = 65536;
constexpr const size_t kCacheCnt = 16;

CommitLogFile::CommitLogFile()
{
  readOnly_ = true;
  fileCode_ = 1;
  pFile_ = nullptr;
  fileSize_ = 0;
  pos_ = 0;
  pWriteBuf_ = nullptr;
}

CommitLogFile::~CommitLogFile()
{
  if (pFile_ != nullptr)
    delete pFile_;

  if (pWriteBuf_ != nullptr)
    delete pWriteBuf_;

  for (auto cacheIt = cacheVec_.begin(); cacheIt != cacheVec_.end(); cacheIt++)
  {
    delete cacheIt->pCache_;
  }
}

PdbErr_t CommitLogFile::OpenLogFile(uint32_t fileCode, const char* pPath)
{
  PdbErr_t retVal = PdbE_OK;
  char buf[kRecFileInfoLen];
  uint64_t tmpSize;
  if (pPath == nullptr)
    return PdbE_INVALID_PARAM;

  if (pFile_ != nullptr)
    return PdbE_INVALID_PARAM;

  readOnly_ = true;
  filePath_ = pPath;
  Env* pEnv = Env::Default();

  retVal = pEnv->GetFileSize(pPath, &tmpSize);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("CommitLogFile::OpenLogFile get file ({}) size failed, {}",
      pPath, retVal);
    return retVal;
  }

  if (tmpSize <= kRecFileInfoLen || tmpSize > kCmtLogFileSize)
    return PdbE_DATA_LOG_ERROR;

  fileSize_ = tmpSize;
  retVal = pEnv->OpenNormalFile(pPath, &pFile_);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("CommitLogFile::OpenLogFile open file ({}) failed, {}",
      pPath, retVal);
    return retVal;
  }

  retVal = pFile_->Read(0, buf, kRecFileInfoLen);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("CommitLogFile::OpenLogFile failed to read ({}) file head, {}",
      filePath_.c_str(), retVal);
    return PdbE_DATA_LOG_ERROR;
  }

  uint32_t recCrc = Coding::FixedDecode32(buf);
  if (recCrc != StringTool::CRC32((buf + 4), (kRecFileInfoLen - 4)))
  {
    LOG_ERROR("CommitLogFile::OpenLogFile ({}) file head crc error", filePath_.c_str());
    return PdbE_DATA_LOG_ERROR;
  }

  //上面是可继续的错误，下面是不可继续的错误
  //主要为了处理，新建日志文件过程中发生宕机的情况
  if (buf[7] != CMTLOG_FILE_VER)
  {
    LOG_ERROR("CommitLogFile::OpenLogFile ({}) file version error", filePath_.c_str());
    return PdbE_DATA_LOG_VER_ERROR;
  }

  fileCode_ = Coding::FixedDecode32(buf + 8);
  if (fileCode_ != fileCode)
  {
    LOG_ERROR("CommitLogFile::OpenLogFile ({}) file code error({}, {})",
      filePath_.c_str(), fileCode, fileCode_);
    return PdbE_DATA_FILECODE_ERROR;
  }

  return PdbE_OK;
}

PdbErr_t CommitLogFile::CreateLogFile(uint32_t fileCode, const char* pPath,
  uint64_t syncPos)
{
  PdbErr_t retVal = PdbE_OK;
  char buf[kRecFileInfoLen];
  Env* pEnv = Env::Default();

  if (pPath == nullptr)
    return PdbE_INVALID_PARAM;

  if (pFile_ != nullptr)
    return PdbE_INVALID_PARAM;

  if (pWriteBuf_ == nullptr)
  {
    pWriteBuf_ = new char[kWritBufferSize];
    if (pWriteBuf_ == nullptr)
    {
      LOG_ERROR("CommitLogFile::CreateLogFile new buffer failed");
      return PdbE_NOMEM;
    }
  }

  fileCode_ = fileCode;
  filePath_ = pPath;
  fileSize_ = 0;
  pos_ = 0;
  readOnly_ = false;

  retVal = pEnv->CreateNormalFile(pPath, &pFile_);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("CommitLogFile::CreateLogFile create file failed ({}), {}",
      pPath, retVal);
    return retVal;
  }

  buf[4] = CMTLOG_TYPE_FILE_HEAD;
  Coding::FixedEncode16((buf + 5), static_cast<uint16_t>(kRecFileInfoLen));
  buf[7] = CMTLOG_FILE_VER;
  Coding::FixedEncode32((buf + 8), fileCode_);
  Coding::FixedEncode64((buf + 12), syncPos);
  Coding::FixedEncode64((buf + 20), 0);
  Coding::FixedEncode64((buf + 28), 0);
  uint32_t crc = StringTool::CRC32((buf + 4), (kRecFileInfoLen - 4));
  Coding::FixedEncode32(buf, crc);

  retVal = Append(buf, kRecFileInfoLen);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("CommitLogFile::CreateLogFile ({}) append file head failed, {}",
      pPath, retVal);
    return retVal;
  }

  return retVal;
}

PdbErr_t CommitLogFile::CloseWrite()
{
  PdbErr_t retVal = PdbE_OK;
  if (!readOnly_)
  {
    readOnly_ = true;
    retVal = Sync();
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("CommitLogFile::CloseWrite ({}) flush buffer failed, {}",
        filePath_.c_str(), retVal);
      return retVal;
    }
  }

  return retVal;
}

void CommitLogFile::FreeCache()
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (readOnly_)
  {
    if (pWriteBuf_ != nullptr)
      delete pWriteBuf_;

    for (auto cacheIt = cacheVec_.begin(); cacheIt != cacheVec_.end(); cacheIt++)
    {
      delete cacheIt->pCache_;
    }

    pos_ = 0;
    pWriteBuf_ = nullptr;
    cacheVec_.clear();
  }
}

uint64_t CommitLogFile::GetFileBeginPos() const
{
  return (fileCode_ - 1) * kCmtLogFileSize;
}

uint64_t CommitLogFile::GetFileEndPos() const
{
  return ((fileCode_ - 1) * kCmtLogFileSize) + fileSize_ + pos_;
}

PdbErr_t CommitLogFile::ReadRecList(uint64_t* pOffset, char* pBuf, size_t bufSize,
  std::vector<LogRecInfo>& recList)
{
  PdbErr_t retVal = PdbE_OK;
  size_t tmpSize , offset;
  LogRecInfo recInfo;
  recList.clear();

  uint64_t fileBgOffset = (fileCode_ - 1) * kCmtLogFileSize;
  if (*pOffset < fileBgOffset)
    *pOffset = fileBgOffset;

  while (true)
  {
    if (*pOffset >= (fileBgOffset + kCmtLogFileSize))
      return PdbE_OK;

    tmpSize = bufSize;
    offset = *pOffset - fileBgOffset;
    retVal = Read(offset, pBuf, &tmpSize);
    if (retVal != PdbE_OK)
      return retVal;

    if (tmpSize == 0)
    {
      if (readOnly_)
        *pOffset = fileCode_ * kCmtLogFileSize;
      
      return PdbE_OK;
    }
    
    const char* pRec = pBuf;
    const char* pLimit = pBuf + tmpSize;
    while (pRec + kRecHead < pLimit)
    {
      uint16_t recLen = Coding::FixedDecode16(pRec + 5);
      if ((pRec + recLen) > pLimit)
        break;

      uint32_t recCrc = Coding::FixedDecode32(pRec);
      if (recCrc != StringTool::CRC32((pRec + 4), (recLen - 4)))
      {
        *pOffset = fileCode_ * kCmtLogFileSize;
        return PdbE_OK;
      }

      if (pRec[4] != CMTLOG_TYPE_FILE_HEAD && pRec[4] != CMTLOG_TYPE_SYNC_INFO)
      {
        recInfo.recType = pRec[4];
        recInfo.tabCrc = Coding::FixedDecode64(pRec + kRecHead);
        recInfo.metaCrc = Coding::FixedDecode32(pRec + kRecHead + 8);
        recInfo.devId = Coding::FixedDecode64(pRec + kRecHead + 12);
        recInfo.pRec = pRec + kRecDataLen;
        recInfo.recLen = recLen - kRecDataLen;
        recList.push_back(recInfo);
      }

      pRec += recLen;
    }

    *pOffset += (pRec - pBuf);

    if (recList.size() > 0)
      return PdbE_OK;

    if (pRec == pBuf)
    {
      if (readOnly_)
        *pOffset = fileCode_ * kCmtLogFileSize;

      return PdbE_OK;
    }
  }
}

PdbErr_t CommitLogFile::AppendRecord(const char* pRecHead,
  const char* pRec, size_t recLen)
{
  PdbErr_t retVal = PdbE_OK;
  if (pRecHead == nullptr || pRec == nullptr || recLen == 0 || recLen > kWritBufferSize)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if ((fileSize_ + pos_ + kRecDataLen + recLen) >= kCmtLogFileSize)
    return PdbE_LOGFILE_FULL;

  retVal = Append(pRecHead, kRecDataLen);
  if (retVal != PdbE_OK)
    return retVal;
  
  return Append(pRec, recLen);
}

PdbErr_t CommitLogFile::AppendSync(uint64_t syncPos)
{
  char data[kRecSyncLen];
  data[4] = CMTLOG_TYPE_SYNC_INFO;
  Coding::FixedEncode16(data + 5, static_cast<uint16_t>(kRecSyncLen));
  Coding::FixedEncode64(data + 7, syncPos);
  Coding::FixedEncode64(data + 15, 0);
  Coding::FixedEncode64(data + 23, 0);
  Coding::FixedEncode32(data, StringTool::CRC32(data + 4, kRecSyncLen - 4));

  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if ((fileSize_ + pos_ + kRecSyncLen) >= kCmtLogFileSize)
    return PdbE_LOGFILE_FULL;

  return Append(data, kRecSyncLen);
}

PdbErr_t CommitLogFile::RecoverPoint(uint64_t* pSyncPos)
{
  PdbErr_t retVal = PdbE_OK;
  const size_t kBufLen = 128 * 1024;
  std::string buf;
  buf.resize(kBufLen);
  char* pBuf = &(buf[0]);
  size_t curPos = 0;
  size_t bufLen;

  while (true)
  {
    bufLen = kBufLen;
    retVal = Read(curPos, pBuf, &bufLen);
    if (retVal != PdbE_OK)
      return retVal;

    if (bufLen == 0)
      return PdbE_OK;

    const char* pRec = pBuf;
    const char* pLimit = pRec + bufLen;
    while (pRec + kRecHead < pLimit)
    {
      uint16_t recLen = Coding::FixedDecode16(pRec + 5);
      if (pRec + recLen > pLimit)
        break;

      if (pRec[4] == CMTLOG_TYPE_FILE_HEAD || pRec[4] == CMTLOG_TYPE_SYNC_INFO)
      {
        uint32_t recCrc = Coding::FixedDecode32(pRec);
        if (recCrc != StringTool::CRC32(pRec + 4, recLen - 4))
          return PdbE_OK;

        *pSyncPos = Coding::FixedDecode64(pRec + recLen - 24);
      }

      pRec += recLen;
    }

    if (pRec == pBuf)
    {
      return PdbE_OK;
    }

    curPos += (pRec - pBuf);
  }

  return PdbE_OK;
}

PdbErr_t CommitLogFile::Sync()
{
  PdbErr_t retVal = PdbE_OK;
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  retVal = FlushBuffer();
  if (retVal != PdbE_OK)
    return retVal;
  
  retVal = pFile_->Sync();
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("flush commit log ({}) failed, fileSize({}), err({})",
      filePath_.c_str(), fileSize_, retVal);
  }
  else
  {
    LOG_DEBUG("flush commit log ({}) success, fileSize({})",
      filePath_.c_str(), fileSize_);
  }

  return retVal;
}

PdbErr_t CommitLogFile::Read(size_t offset, char* pBuf, size_t* pBytes)
{
  PdbErr_t retVal = PdbE_OK;
  size_t tmpBytes;
  if (pBuf == nullptr || pBytes == nullptr)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (offset >= (fileSize_ + pos_))
  {
    *pBytes = 0;
    return PdbE_OK;
  }

  char* pTmpBuf = pBuf;
  char* pLimit = pBuf + *pBytes;
  size_t bgPos = 0;

  do {
    if (offset < fileSize_)
    {
      tmpBytes = PDB_MIN_VAL(static_cast<size_t>(pLimit - pTmpBuf), (fileSize_ - offset));
      if (cacheVec_.size() > 0)
      {
        tmpBytes = 0;
        if (offset < cacheVec_[0].cacheOffset_)
        {
          tmpBytes = PDB_MIN_VAL(static_cast<size_t>(pLimit - pTmpBuf), (cacheVec_[0].cacheOffset_ - offset));
        }
      }

      if (tmpBytes > 0)
      {
        retVal = pFile_->Read(offset, pTmpBuf, tmpBytes);
        if (retVal != PdbE_OK)
        {
          return retVal;
        }

        pTmpBuf += tmpBytes;
        if (pTmpBuf >= pLimit)
          break;
      }

      for (auto cacheIt = cacheVec_.begin(); cacheIt != cacheVec_.end(); cacheIt++)
      {
        if (offset < (cacheIt->cacheOffset_ + cacheIt->cacheSize_))
        {
          bgPos = 0;
          if (offset > cacheIt->cacheOffset_)
            bgPos = offset - cacheIt->cacheOffset_;

          tmpBytes = PDB_MIN_VAL(static_cast<size_t>(pLimit - pTmpBuf), (cacheIt->cacheSize_ - bgPos));
          std::memcpy(pTmpBuf, (cacheIt->pCache_ + bgPos), tmpBytes);
          pTmpBuf += tmpBytes;
          if (pTmpBuf >= pLimit)
            break;
        }
      }
    
    }

    if (pTmpBuf >= pLimit)
      break;

    if (pos_ > 0)
    {
      bgPos = 0;
      if (offset > fileSize_)
        bgPos = offset - fileSize_;

      tmpBytes = PDB_MIN_VAL(static_cast<size_t>(pLimit - pTmpBuf), (pos_ - bgPos));
      std::memcpy(pTmpBuf, (pWriteBuf_ + bgPos), tmpBytes);
      pTmpBuf += tmpBytes;
    }
  } while (false);

  *pBytes = pTmpBuf - pBuf;
  return PdbE_OK;
}

PdbErr_t CommitLogFile::Append(const char* pBuf, size_t bytes)
{
  if (pBuf == nullptr || bytes > kWritBufferSize)
    return PdbE_INVALID_PARAM;

  if (readOnly_)
    return PdbE_FILE_READONLY;

  size_t copySize = PDB_MIN_VAL(bytes, (kWritBufferSize - pos_));
  std::memcpy(pWriteBuf_ + pos_, pBuf, copySize);
  pBuf += copySize;
  bytes -= copySize;
  pos_ += copySize;
  if (bytes == 0) {
    return PdbE_OK;
  }

  PdbErr_t retVal = FlushBuffer();
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  std::memcpy(pWriteBuf_, pBuf, bytes);
  pos_ = bytes;
  return PdbE_OK;
}

PdbErr_t CommitLogFile::FlushBuffer()
{
  PdbErr_t retVal = PdbE_OK;

  if (pos_ > 0)
  {
    retVal = pFile_->Write(fileSize_, pWriteBuf_, pos_);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("write commit log ({}) failed, position({}), bytes({}), err({})",
        filePath_.c_str(), fileSize_, pos_, retVal);
      return retVal;
    }

    char* pNewBuf = nullptr;
    do {
      if (cacheVec_.size() < kCacheCnt)
      {
        pNewBuf = new char[kWritBufferSize];
        if (pNewBuf != nullptr)
          break;
      }

      if (cacheVec_.size() > 0)
      {
        pNewBuf = cacheVec_[0].pCache_;
        cacheVec_.erase(cacheVec_.begin());
      }
    } while (false);

    if (pNewBuf != nullptr)
    {
      CacheBlock newCache;
      newCache.pCache_ = pWriteBuf_;
      newCache.cacheSize_ = pos_;
      newCache.cacheOffset_ = fileSize_;
      cacheVec_.push_back(newCache);
      pWriteBuf_ = pNewBuf;
    }

    fileSize_ += pos_;
    pos_ = 0;
  }

  return retVal;
}
