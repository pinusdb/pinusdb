#include "commitlog/commit_log_file.h"
#include "util/string_tool.h"
#include "util/log_util.h"

CommitLogFile::CommitLogFile()
{
  curPos_ = 0;
  syncLen_ = 0;
  syncTime_ = 0;
  fileCode_ = 0;
}

CommitLogFile::~CommitLogFile()
{
}

PdbErr_t CommitLogFile::OpenLog(uint32_t fileCode, const char* pPath)
{
  PdbErr_t retVal = PdbE_OK;
  LogFileHdr fileHdr;
  SyncBlkHdr syncBlk;
  if (logFile_.IsOpen())
  {
    logFile_.Close();
  }

  fileCode_ = fileCode;
  retVal = logFile_.OpenNormal(pPath, true);
  if (retVal != PdbE_OK)
    return PdbE_DATA_LOG_ERROR;

  filePath_ = pPath;

  retVal = logFile_.Read(&fileHdr, sizeof(LogFileHdr), 0);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("failed to read datalog({}) file head, err: {}", pPath, retVal);
    return PdbE_DATA_LOG_ERROR;
  }

  retVal = logFile_.Read(&syncBlk, sizeof(SyncBlkHdr), sizeof(LogFileHdr));
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("failed to read datalog({}) first sync block, err: {} ", pPath, retVal);
    return PdbE_DATA_LOG_ERROR;
  }

  if (fileHdr.blkHdr_.hdrCrc_ != StringTool::CRC32(&fileHdr, (sizeof(LogFileHdr) - 4)))
  {
    LOG_ERROR("datalog({}) file head crc error", pPath);
    return PdbE_DATA_LOG_ERROR;
  }

  if (syncBlk.blkHdr_.hdrCrc_ != StringTool::CRC32(&syncBlk, (sizeof(SyncBlkHdr) - 4)))
  {
    LOG_ERROR("datalog({}) first sync block crc error", pPath);
    return PdbE_DATA_LOG_ERROR;
  }

  //上面是可继续的错误，下面是不可继续的错误
  //主要为了处理，新建日志文件过程中发生宕机的情况

  if (fileHdr.logFileVer_ != DATA_LOG_FILE_VER)
  {
    LOG_ERROR("datalog({}) file version error", pPath);
    return PdbE_DATA_LOG_VER_ERROR;
  }

  if (fileHdr.logFileCode_ != fileCode_)
  {
    LOG_ERROR("datalog({}) file code error({}, {})", pPath, fileCode_, fileHdr.logFileCode_);
    return PdbE_DATA_FILECODE_ERROR;
  }
  return PdbE_OK;
}

PdbErr_t CommitLogFile::NewLog(uint32_t fileCode, const char* pPath, uint64_t repPos, uint64_t syncPos)
{
  PdbErr_t retVal = PdbE_OK;
  LogFileHdr fileHdr;
  SyncBlkHdr syncBlk;
  if (logFile_.IsOpen())
    logFile_.Close();

  filePath_ = pPath;

  do {
    curPos_ = 0;
    fileCode_ = fileCode;
    retVal = logFile_.OpenNew(pPath);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to create datalog({}) file, err: {}", pPath, retVal);
      return retVal;
    }

    retVal = logFile_.GrowTo(DATA_LOG_FILE_SIZE);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to grow datalog({}) file, err: {}", pPath, retVal);
      break;
    }

    memset(&fileHdr, 0, sizeof(LogFileHdr));
    memset(&syncBlk, 0, sizeof(SyncBlkHdr));

    fileHdr.logFileVer_ = DATA_LOG_FILE_VER;
    fileHdr.logFileCode_ = fileCode;
    fileHdr.blkHdr_.blkType_ = BLKTYPE_FILE_HEAD;
    fileHdr.blkHdr_.hdrCrc_ = StringTool::CRC32(&fileHdr, (sizeof(LogFileHdr) - 4));

    syncBlk.repPos_ = repPos;
    syncBlk.syncPos_ = syncPos;
    syncBlk.blkHdr_.blkType_ = BLKTYPE_SYNC_INFO;
    syncBlk.blkHdr_.hdrCrc_ = StringTool::CRC32(&syncBlk, (sizeof(SyncBlkHdr) - 4));

    retVal = logFile_.Write(&fileHdr, sizeof(LogFileHdr), curPos_);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to write datalog({}) file head info, err: {}", pPath, retVal);
      break;
    }

    curPos_ += sizeof(LogFileHdr);
    retVal = logFile_.Write(&syncBlk, sizeof(SyncBlkHdr), curPos_);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to write datalog({}) file first sync info, err: {}", pPath, retVal);
      break;
    }

    syncLen_ = 0;
    syncTime_ = GetTickCount64();
    curPos_ += sizeof(SyncBlkHdr);
  } while (false);

  if (retVal != PdbE_OK)
  {
    logFile_.Close();
    FileTool::RemoveFile(pPath);
  }

  return retVal;
}

PdbErr_t CommitLogFile::Close()
{
  if (!logFile_.ReadOnly())
  {
    logFile_.Sync();
  }
  logFile_.Close();
  return PdbE_OK;
}

void CommitLogFile::Sync()
{
  if (syncLen_ < curPos_)
  {
    logFile_.Sync();
    syncLen_ = curPos_;
    syncTime_ = GetTickCount64();
  }
}

PdbErr_t CommitLogFile::AppendData(const LogBlkHdr* pLogHdr, const CommitLogBlock* pLogBlock)
{
  PdbErr_t retVal = PdbE_OK;
  if (pLogHdr == nullptr || pLogBlock == nullptr)
    return PdbE_INVALID_PARAM;

  const std::list<PdbBlob>& recList = pLogBlock->GetRecList();
  retVal = logFile_.Write(pLogHdr, sizeof(LogBlkHdr), curPos_);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("failed to write datalog block head, file: ({}), pos: {}, err: {}", filePath_.c_str(), curPos_, retVal);
    return retVal;
  }

  curPos_ += sizeof(LogBlkHdr);
  for (auto recItem = recList.begin(); recItem != recList.end(); recItem++)
  {
    retVal = logFile_.Write(recItem->pBlob_, static_cast<int32_t>(recItem->len_), curPos_);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to write datalog, file: ({}), pos: {}, datalen: {}, err: {}", 
        filePath_.c_str(), curPos_, recItem->len_, retVal);
      return retVal;
    }
    curPos_ += recItem->len_;
  }

  uint64_t curTime = GetTickCount64();
  if ((curPos_ - syncLen_) > PDB_MB_BYTES(2) || curTime > (syncTime_ + 3000))
  {
    logFile_.Sync();
    syncLen_ = curPos_;
    syncTime_ = curTime;
  }

  return PdbE_OK;
}

PdbErr_t CommitLogFile::AppendSync(uint64_t repPos, uint64_t syncPos)
{
  PdbErr_t retVal = PdbE_OK;
  SyncBlkHdr syncHdr;

  if (!logFile_.IsOpen())
  {
    LOG_ERROR("failed to write sync info, file: ({}) closed", filePath_.c_str());
    return PdbE_INVALID_PARAM;
  }

  memset(&syncHdr, 0, sizeof(SyncBlkHdr));
  syncHdr.repPos_ = repPos;
  syncHdr.syncPos_ = syncPos;
  syncHdr.blkHdr_.blkType_ = BLKTYPE_SYNC_INFO;
  syncHdr.blkHdr_.hdrCrc_ = StringTool::CRC32(&syncHdr, (sizeof(SyncBlkHdr) - 4));

  retVal = logFile_.Write(&syncHdr, sizeof(SyncBlkHdr), curPos_);
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("failed to write sync info, file: ({}), pos: {}, err: {}",
      filePath_.c_str(), curPos_, retVal);
    return retVal;
  }
  curPos_ += sizeof(SyncBlkHdr);
  return PdbE_OK;
}

PdbErr_t CommitLogFile::ReadBuf(uint8_t* pBuf, int readLen, int64_t offset)
{
  if (pBuf == nullptr || readLen <= 0 || offset <= 0)
    return PdbE_INVALID_PARAM;

  return logFile_.Read(pBuf, readLen, offset);
}

PdbErr_t CommitLogFile::RecoverPoint(uint64_t* pRepPos, uint64_t* pSyncPos)
{
  //由于打开数据日志文件时已验证第一个同步块，所以一定会得到同步信息
  PdbErr_t retVal = PdbE_OK;
  uint8_t hdrBuf[sizeof(SyncBlkHdr)];
  SyncBlkHdr* pSyncHdr = (SyncBlkHdr*)hdrBuf;
  LogBlkHdr* pLogHdr = (LogBlkHdr*)hdrBuf;

  if (pRepPos == nullptr || pSyncPos == nullptr)
    return PdbE_INVALID_PARAM;

  int64_t curPos = sizeof(LogFileHdr);

  while (curPos < DATA_LOG_FILE_SIZE)
  {
    retVal = ReadBuf(hdrBuf, sizeof(SyncBlkHdr), curPos);
    if (retVal != PdbE_OK)
    {
      LOG_INFO("recover datalog({}) , read block head error, pos: {}, err: {}",
        filePath_.c_str(), curPos, retVal);
      break;
    }

    if (StringTool::CRC32(hdrBuf, (sizeof(SyncBlkHdr) - 4)) != pSyncHdr->blkHdr_.hdrCrc_)
    {
      LOG_INFO("recover datalog({}), block head crc error, pos: {}",
        filePath_.c_str(), curPos);
      break;
    }

    if (pSyncHdr->blkHdr_.blkType_ == BLKTYPE_SYNC_INFO)
    {
      *pRepPos = pSyncHdr->repPos_;
      *pSyncPos = pSyncHdr->syncPos_;
      curPos += sizeof(SyncBlkHdr);
    }
    else if (pSyncHdr->blkHdr_.blkType_ == BLKTYPE_INSERT_CLI
      || pSyncHdr->blkHdr_.blkType_ == BLKTYPE_INSERT_REP)
    {
      curPos += sizeof(SyncBlkHdr);
      curPos += pLogHdr->dataLen_;
    }
    else
      break;
  }

  curPos_ = curPos;
  return PdbE_OK;
}
