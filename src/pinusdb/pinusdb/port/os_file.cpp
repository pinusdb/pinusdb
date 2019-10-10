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

#include "port/os_file.h"
#include "util/log_util.h"

#ifdef _WIN32

#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#ifdef _WIN32
OSFile::OSFile()
{
  handle_ = INVALID_HANDLE_VALUE;
  readOnly_ = true;
}

OSFile::~OSFile()
{
  Close();
}

bool OSFile::IsOpen()
{
  return handle_ != INVALID_HANDLE_VALUE;
}

PdbErr_t OSFile::Open(const char* pPath, bool readOnly, bool create, bool noBuf)
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ != INVALID_HANDLE_VALUE)
    return PdbE_OPENED;

  DWORD accessFlag = readOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);
  DWORD createFlag = create ? CREATE_ALWAYS : OPEN_EXISTING;
  DWORD dwFlag = noBuf ? (FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH): FILE_ATTRIBUTE_NORMAL;

  handle_ = CreateFile(pPath,
    accessFlag,
    0,
    NULL,
    createFlag,
    dwFlag,
    NULL);
  if (INVALID_HANDLE_VALUE == handle_)
  {
    LOG_ERROR("failed to CreateFile, path: ({}), err:{} ", 
      pPath, GetLastError());
    return PdbE_IOERR;
  }

  this->readOnly_ = readOnly;
  this->path_ = pPath;
  return PdbE_OK;
}

void OSFile::Close()
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ != INVALID_HANDLE_VALUE)
  {
    if (!readOnly_)
      _Sync();

    CloseHandle(handle_);
    handle_ = INVALID_HANDLE_VALUE;
  }
}

PdbErr_t OSFile::Read(void* pBuf, size_t count, size_t offset)
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ == INVALID_HANDLE_VALUE)
    return PdbE_INVALID_HANDLE;

  _Seek(offset);

  DWORD needReadLen = static_cast<DWORD>(count);
  DWORD readLen = 0;
  if (!ReadFile(handle_, pBuf, needReadLen, &readLen, 0))
    readLen = 0;

  if (readLen == needReadLen)
    return PdbE_OK;

  LOG_ERROR("failed to ReadFile ({}) offset({}), count({}), pBuf({}), err:{}",
    path_.c_str(), offset, count, reinterpret_cast<uintptr_t>(pBuf), GetLastError());
  return PdbE_IOERR;
}

PdbErr_t OSFile::Write(const void* pBuf, size_t count, size_t offset)
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ == INVALID_HANDLE_VALUE)
    return PdbE_INVALID_HANDLE;

  if (readOnly_)
    return PdbE_FILE_READONLY;

  _Seek(offset);

  BOOL writeRet = 0;
  DWORD needWriteLen = static_cast<DWORD>(count);
  DWORD writeLen = 0;
  const uint8_t* pTmp = (const uint8_t*)pBuf;

  do {
    writeRet = WriteFile(handle_, pTmp, needWriteLen, &writeLen, NULL);
    needWriteLen -= writeLen;
    pTmp += writeLen;
  } while (writeRet && needWriteLen > 0 && writeLen > 0);

  if (!writeRet || needWriteLen > 0)
  {
    LOG_DEBUG("failed to WriteFile ({}), offset({}), count({}), err:{}",
      path_.c_str(), offset, count, GetLastError());
    return PdbE_IOERR;
  }

  return PdbE_OK;
}


size_t OSFile::FileSize()
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ == INVALID_HANDLE_VALUE)
    return 0;

  return _FileSize();
}

PdbErr_t OSFile::Grow(size_t bytes)
{
  if (bytes <= 0)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ == INVALID_HANDLE_VALUE)
  {
    return PdbE_INVALID_HANDLE;
  }

  PdbErr_t ret = PdbE_OK;

  int64_t oldBytes = _FileSize();
  int64_t newBytes = oldBytes + bytes;

  LONG upperBits = newBytes >> 32;
  LONG lowerBites = newBytes & 0xFFFFFFFF;
  SetFilePointer(handle_, lowerBites, &upperBits, FILE_BEGIN);
  SetEndOfFile(handle_);
  return PdbE_OK;
}

PdbErr_t OSFile::GrowTo(size_t bytes)
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ == INVALID_HANDLE_VALUE)
  {
    return PdbE_INVALID_HANDLE;
  }

  PdbErr_t retVal = PdbE_OK;

  size_t oldBytes = _FileSize();
  if (bytes < oldBytes)
  {
    return PdbE_OK;
  }

  LONG upperBits = bytes >> 32;
  LONG lowerBits = bytes & 0xFFFFFFFF;
  SetFilePointer(handle_, lowerBits, &upperBits, FILE_BEGIN);
  SetEndOfFile(handle_);
  return PdbE_OK;
}

PdbErr_t OSFile::_Sync()
{
  if (handle_ == INVALID_HANDLE_VALUE)
    return PdbE_INVALID_HANDLE;

  if (FlushFileBuffers(handle_))
    return PdbE_OK;

  LOG_DEBUG("failed to FlushFileBuffers ({}), err: {}", path_.c_str(), GetLastError());
  return PdbE_IOERR;
}

void OSFile::_Seek(size_t offset)
{
  if (handle_ != INVALID_HANDLE_VALUE)
  {
    LONG upperBits = offset >> 32;
    LONG lowerBits = offset & 0xFFFFFFFF;

    SetFilePointer(handle_, lowerBits, &upperBits, FILE_BEGIN);
  }
}

size_t OSFile::_FileSize()
{
  if (handle_ == INVALID_HANDLE_VALUE)
    return 0;

  DWORD upperBits, lowerBits;
  lowerBits = GetFileSize(handle_, &upperBits);
  return (((size_t)upperBits) << 32) + lowerBits;
}

#else

OSFile::OSFile()
{
  handle_ = -1;
  readOnly_ = true;
}

OSFile::~OSFile()
{
  Close();
}

bool OSFile::IsOpen()
{
  return handle_ >= 0;
}

PdbErr_t OSFile::Open(const char* pPath, bool readOnly, bool create, bool noBuf)
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ >= 0)
    return PdbE_OPENED;

  int flags = O_LARGEFILE | O_NOATIME;
  mode_t modes = 0;

  if (readOnly)
  {
    flags |= O_RDONLY;
  }
  else
  {
    flags |= O_RDWR;

    if (create)
    {
      flags |= O_CREAT;
      modes = S_IRUSR | S_IWUSR;
    }
  }

  if (noBuf)
  {
    flags |= O_DIRECT;
  }

  int tmpHandle = open(pPath, flags, modes);
  if (tmpHandle < 0)
    return errno;

  this->handle_ = tmpHandle;
  this->readOnly_ = readOnly;
  return PdbE_OK;
}

void OSFile::Close()
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ >= 0)
  {
    if (!readOnly_)
      _Sync();

    close(handle_);
    handle_ = -1;
  }
}

PdbErr_t OSFile::Read(void* pBuf, size_t count, size_t offset)
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ < 0)
    return PdbE_INVALID_HANDLE;

  _Seek(offset);
  ssize_t ret = read(handle_, pBuf, count);
  if (ret == count)
    return PdbE_OK;

  LOG_ERROR("failed to read ({}) offset({}), count({}), pBuf({}), err:{}",
    path_.c_str(), offset, count, reinterpret_cast<uintptr_t>(pBuf), errno);
  return PdbE_IOERR;
}

PdbErr_t OSFile::Write(const void* pBuf, size_t count, size_t offset)
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ < 0)
    return PdbE_INVALID_HANDLE;

  if (readOnly_)
    return PdbE_FILE_READONLY;

  _Seek(offset);

  int retryCnt = 0;
  size_t needWrite = count;
  ssize_t ret = 0;
  const void* pTmp = pBuf;

  while (needWrite > 0 && (ret = write(handle_, pTmp, needWrite)) >= 0)
  {
    needWrite -= ret;
    pTmp = ((const char*)pTmp) + ret;

    if (ret == 0)
    {
      retryCnt++;
      if (retryCnt > 3)
        break;
    }
    else
    {
      retryCnt = 0;
    }
  }

  if (ret < 0)
  {
    LOG_DEBUG("failed to write ({}), offset({}), count({}), err:{}",
      path_.c_str(), offset, needWrite, errno);
    return PdbE_IOERR;
  }

  return PdbE_OK;
}

size_t OSFile::FileSize()
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ < 0)
    return 0;

  return _FileSize();
}

PdbErr_t OSFile::Grow(size_t bytes)
{
  if (bytes <= 0)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ < 0)
    return PdbE_INVALID_HANDLE;

  if (readOnly_)
    return PdbE_FILE_READONLY;

  int64_t newBytes = _FileSize() + bytes;

  if (ftruncate(handle_, newBytes) < 0)
  {
    LOG_ERROR("grow file ({}) to ({}) failed, errno:({})", 
      path_.c_str(), newBytes, errno);
    return errno;
  }

  return PdbE_OK;
}

PdbErr_t OSFile::GrowTo(size_t bytes)
{
  if (bytes <= 0)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ < 0)
    return PdbE_INVALID_HANDLE;

  if (readOnly_)
    return PdbE_FILE_READONLY;

  size_t oldBytes = _FileSize();
  if (bytes <= oldBytes)
    return PdbE_OK;

  if (ftruncate(handle_, bytes) < 0)
  {
    LOG_ERROR("grow file ({}) to ({}) failed, errno:({})",
      path_.c_str(), bytes, errno);
    return errno;
  }

  return PdbE_OK;
}

PdbErr_t OSFile::_Sync()
{
  if (handle_ < 0)
    return PdbE_INVALID_HANDLE;

  if (readOnly_)
    return PdbE_FILE_READONLY;

  if (fsync(handle_) < 0)
  {
    LOG_ERROR("failed to fsync ({}), err: {}", path_.c_str(), errno);
    return PdbE_IOERR;
  }

  return PdbE_OK;
}

void OSFile::_Seek(size_t offset)
{
  if (handle_ >= 0)
  {
    lseek(handle_, offset, SEEK_SET);
  }
}

size_t OSFile::_FileSize()
{
  struct stat statbuf;
  if (handle_ < 0)
    return 0;

  if (fstat(handle_, &statbuf) < 0)
  {
    LOG_ERROR("failed to fstat ({}), err: {}", path_.c_str(), errno);
    return 0;
  }

  return statbuf.st_size;
}

#endif

PdbErr_t OSFile::OpenNoBuf(const char* pPath, bool readOnly)
{
  return Open(pPath, readOnly, false, true);
}

PdbErr_t OSFile::OpenNormal(const char* pPath, bool readOnly)
{
  return Open(pPath, readOnly, false, false);
}

PdbErr_t OSFile::OpenNew(const char* pPath)
{
  return Open(pPath, false, true, false);
}

PdbErr_t OSFile::Sync()
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  return _Sync();
}

///////////////////////////////////////////////////////////////////

#ifdef _WIN32
bool FileTool::FileExists(const char* pPath)
{
  DWORD dwAttribute = GetFileAttributes(pPath);
  return (INVALID_FILE_ATTRIBUTES != dwAttribute) && ((FILE_ATTRIBUTE_DIRECTORY & dwAttribute) == 0);
}

bool FileTool::PathExists(const char* pPath)
{
  DWORD dwAttribute = GetFileAttributes(pPath);
  return (INVALID_FILE_ATTRIBUTES != dwAttribute) && ((FILE_ATTRIBUTE_DIRECTORY & dwAttribute) != 0);
}

PdbErr_t FileTool::RemoveFile(const char* pFile)
{
  if (pFile == nullptr)
    return PdbE_INVALID_PARAM;

  if (DeleteFile(pFile) != FALSE)
  {
    return PdbE_OK;
  }

  return static_cast<PdbErr_t>(GetLastError());
}

PdbErr_t FileTool::Rename(const char* pSourceFile, const char* pDestFile)
{
  if (pSourceFile == nullptr || pDestFile == nullptr)
    return PdbE_INVALID_PARAM;

  if (MoveFile(pSourceFile, pDestFile) != FALSE)
    return PdbE_OK;

  return static_cast<PdbErr_t>(GetLastError());
}

PdbErr_t FileTool::MakePath(const char* pPath)
{
  if (CreateDirectory(pPath, nullptr))
    return PdbE_OK;

  return GetLastError();
}

#else

bool FileTool::FileExists(const char* pPath)
{
  struct stat sbuf;
  if (stat(pPath, &sbuf) < 0)
    return false;

  return S_ISREG(sbuf.st_mode);
}

bool FileTool::PathExists(const char* pPath)
{
  struct stat sbuf;
  if (stat(pPath, &sbuf) < 0)
    return false;

  return S_ISDIR(sbuf.st_mode);
}

PdbErr_t FileTool::RemoveFile(const char* pFile)
{
  if (pFile == nullptr)
    return PdbE_INVALID_PARAM;

  if (unlink(pFile) < 0)
  {
    LOG_ERROR("failed to unlink ({}), errno:{}", pFile, errno);
    return errno;
  }

  return PdbE_OK;
}

PdbErr_t FileTool::Rename(const char* pSourceFile, const char* pDestFile)
{
  if (pSourceFile == nullptr || pDestFile == nullptr)
    return PdbE_INVALID_PARAM;

  if (rename(pSourceFile, pDestFile) < 0)
    return errno;

  return PdbE_OK;
}

PdbErr_t FileTool::MakePath(const char* pPath)
{
  if (mkdir(pPath, S_IRWXU | S_IRWXG | S_IROTH) < 0)
    return PdbE_IOERR;

  return PdbE_OK;
}
#endif

PdbErr_t FileTool::MakeParentPath(const char* pPath)
{
  PdbErr_t retVal = PdbE_OK;
  char tmpPath[MAX_PATH];
  const char* pTmp = pPath;

  while (*pTmp != '\0')
  {
    if ((*pTmp == '\\' || *pTmp == '/') && (pTmp != pPath))
    {
      tmpPath[(pTmp - pPath)] = '\0';
      if (!FileTool::PathExists(tmpPath))
      {
        retVal = FileTool::MakePath(tmpPath);
        if (retVal != PdbE_OK)
          return retVal;
      }
    }

    tmpPath[(pTmp - pPath)] = *pTmp;
    pTmp++;
  }

  return PdbE_OK;
}