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

OSFile::OSFile()
{
  handle_ = INVALID_HANDLE_VALUE;
  readOnly_ = true;
}

OSFile::~OSFile()
{
  Close();
}

PdbErr_t OSFile::Open(const char* pPath, bool readOnly, bool create, bool noBuf)
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  if (handle_ != INVALID_HANDLE_VALUE)
    return PdbE_OPENED;

  DWORD accessFlag = readOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);
  DWORD createFlag = create ? CREATE_ALWAYS : OPEN_EXISTING;
  DWORD dwFlag = noBuf ? FILE_FLAG_NO_BUFFERING : FILE_ATTRIBUTE_NORMAL;

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

PdbErr_t OSFile::Sync()
{
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  return _Sync();
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

///////////////////////////////////////////////////////////////////

void FileTool::FlushAll()
{
  FlushProcessWriteBuffers();
}

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

PdbErr_t FileTool::GetBytesPerSector(const char* pPath, int* pBytes)
{
  if (pPath == nullptr || pBytes == nullptr)
    return PdbE_INVALID_PARAM;

  DWORD bytesPerSector = 0;

  if (GetDiskFreeSpace(pPath, NULL, &bytesPerSector, NULL, NULL) == FALSE)
  {
    return PdbE_IOERR;
  }

  *pBytes = bytesPerSector;

  return PdbE_OK;
}

PdbErr_t FileTool::MakePath(const char* pPath)
{
  if (CreateDirectory(pPath, nullptr))
    return PdbE_OK;

  return GetLastError();
}

PdbErr_t FileTool::MakeParentPath(char* pPath)
{
  PdbErr_t retVal = PdbE_OK;
  char* pTmp = pPath;
  bool isFastPath = true;
  char oldChar = '\0';

  while (*pTmp != '\0')
  {
    if (*pTmp == '\\' || *pTmp == '/')
    {
      oldChar = *pTmp;
      *pTmp = '\0';
      if (!FileTool::PathExists(pPath))
      {
        if (isFastPath)
          return PdbE_PATH_NOT_FOUND;

        retVal = FileTool::MakePath(pPath);
        if (retVal != PdbE_OK)
          return retVal;
      }

      *pTmp = oldChar;
      isFastPath = false;
    }

    pTmp++;
  }

  return PdbE_OK;
}