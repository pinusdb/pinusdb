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

#include "port/mem_map_file.h"
#include "util/log_util.h"

#ifdef _WIN32

MemMapFile::MemMapFile()
{
  fileHandle_ = INVALID_HANDLE_VALUE;
  mapHandle_ = NULL;
  pBase_ = NULL;
  fileSize_ = 0;
  readOnly_ = true;
}

MemMapFile::~MemMapFile()
{
  Close();
}

bool MemMapFile::IsOpened()
{
  return fileHandle_ != INVALID_HANDLE_VALUE;
}

PdbErr_t MemMapFile::Open(const char* pFilePath, bool readOnly)
{
  PdbErr_t retVal = PdbE_OK;
  LARGE_INTEGER tmpSize;

  readOnly_ = readOnly;
  if (fileHandle_ != INVALID_HANDLE_VALUE)
    return PdbE_OBJECT_INITIALIZED;

  do {

    fileHandle_ = CreateFile(pFilePath,
      readOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
      0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == fileHandle_)
    {
      retVal = PdbE_IOERR;
      break;
    }

    if (GetFileSizeEx(fileHandle_, &tmpSize) == FALSE)
    {
      retVal = PdbE_IOERR;
      break;
    }

    fileSize_ = tmpSize.QuadPart;
    mapHandle_ = CreateFileMapping(fileHandle_, NULL,
      readOnly ? PAGE_READONLY : PAGE_READWRITE,
      0, 0, NULL);
    if (NULL == mapHandle_)
    {
      retVal = PdbE_IOERR;
      break;
    }

    pBase_ = MapViewOfFile(mapHandle_,
      readOnly ? FILE_MAP_READ : (FILE_MAP_READ | FILE_MAP_WRITE),
      0, 0, 0);
    if (NULL == pBase_)
    {
      retVal = PdbE_IOERR;
      break;
    }
  } while (false);

  if (retVal != PdbE_OK)
  {
    Close();
  }

  return retVal;
}

PdbErr_t MemMapFile::Sync()
{
  if (!readOnly_)
    FlushViewOfFile(pBase_, fileSize_);

  return PdbE_OK;
}

PdbErr_t MemMapFile::GrowFile(size_t growSize)
{
  if (readOnly_)
    return PdbE_FILE_READONLY;

  size_t newSize = fileSize_ + growSize;
  DWORD upperBits = (newSize >> 32);
  DWORD lowerBits = (newSize & 0xFFFFFFFF);

  FlushViewOfFile(pBase_, fileSize_);

  HANDLE tmpMapHandle = CreateFileMapping(fileHandle_, NULL, PAGE_READWRITE, upperBits, lowerBits, NULL);
  if (tmpMapHandle == NULL)
  {
    return PdbE_IOERR;
  }

  LPVOID pTmpBase = MapViewOfFile(tmpMapHandle, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, newSize);
  if (NULL == pTmpBase)
  {
    CloseHandle(tmpMapHandle);
    return PdbE_IOERR;
  }

  UnmapViewOfFile(pBase_);
  CloseHandle(mapHandle_);

  fileSize_ = newSize;
  pBase_ = pTmpBase;
  mapHandle_ = tmpMapHandle;
  return PdbE_OK;
}

void MemMapFile::Close()
{
  if (pBase_ != NULL)
  {
    if (!readOnly_)
      FlushViewOfFile(pBase_, fileSize_);

    UnmapViewOfFile(pBase_);
    pBase_ = NULL;
  }

  if (mapHandle_ != NULL)
  {
    CloseHandle(mapHandle_);
    mapHandle_ = NULL;
  }

  if (fileHandle_ != INVALID_HANDLE_VALUE)
  {
    CloseHandle(fileHandle_);
    fileHandle_ = INVALID_HANDLE_VALUE;
  }

  fileSize_ = 0;
  readOnly_ = true;
}

#else

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

MemMapFile::MemMapFile()
{
  fileHandle_ = -1;
  pBase_ = nullptr;
  fileSize_ = 0;
  readOnly_ = true;
}

MemMapFile::~MemMapFile()
{
  Close();
}

bool MemMapFile::IsOpened()
{
  return pBase_ != nullptr;
}

PdbErr_t MemMapFile::Open(const char* pFilePath, bool readOnly)
{
  if (fileHandle_ >= 0 || pBase_ != nullptr)
    return PdbE_OBJECT_INITIALIZED;

  readOnly_ = readOnly;
  int flags = O_LARGEFILE | O_NOATIME;
  if (readOnly)
    flags |= O_RDONLY;
  else
    flags |= O_RDWR;

  int tmpHandle = open(pFilePath, flags);
  if (tmpHandle < 0)
  {
    LOG_ERROR("mmap failed, open file({}) failed, err:({})", pFilePath, errno);
    return PdbE_IOERR;
  }

  struct stat sbuf;
  if (fstat(tmpHandle, &sbuf) < 0)
  {
    LOG_ERROR("mmap failed, get file({}) size failed, err:({})", pFilePath, errno);
    close(tmpHandle);
    return PdbE_IOERR;
  }

  fileSize_ = sbuf.st_size;

  void* pTmpAddr = mmap(nullptr, fileSize_, 
    (readOnly ? PROT_READ : PROT_READ | PROT_WRITE), MAP_SHARED, tmpHandle, 0);
  if (pTmpAddr == MAP_FAILED)
  {
    LOG_ERROR("mmap failed, file({}), err:({})", pFilePath, errno);
    close(tmpHandle);
    return PdbE_IOERR;
  }

  fileHandle_ = tmpHandle;
  pBase_ = pTmpAddr;
  return PdbE_OK;
}

PdbErr_t MemMapFile::Sync()
{
  if (msync(pBase_, fileSize_, MS_SYNC) != 0)
  {
    LOG_ERROR("msync mmap file failed, err:({})", errno);
    return PdbE_IOERR;
  }

  return PdbE_OK;
}


PdbErr_t MemMapFile::GrowFile(size_t growSize)
{
  if (readOnly_)
    return PdbE_FILE_READONLY;

  size_t newSize = fileSize_ + growSize;
  if (ftruncate(fileHandle_, newSize) < 0)
  {
    LOG_ERROR("grow mmap file to ({}) failed, errno:({})",
      newSize, errno);
    return PdbE_IOERR;
  }

  void* pTmpAddr = mmap(nullptr, newSize, (PROT_READ | PROT_WRITE),
    MAP_SHARED, fileHandle_, 0);
  if (pTmpAddr == MAP_FAILED)
  {
    LOG_ERROR("grow devid file failed, mmap errno:({})", errno);
    return PdbE_IOERR;
  }

  munmap(pBase_, fileSize_);
  
  fileSize_ = newSize;
  pBase_ = pTmpAddr;
  return PdbE_OK;
}

void MemMapFile::Close()
{
  if (pBase_ != nullptr)
  {
    Sync();
    if (munmap(pBase_, fileSize_) < 0)
    {
      LOG_ERROR("munmap failed, ");
    }

    pBase_ = nullptr;
  }

  if (fileHandle_ >= 0)
  {
    close(fileHandle_);
    fileHandle_ = -1;
  }
}

#endif