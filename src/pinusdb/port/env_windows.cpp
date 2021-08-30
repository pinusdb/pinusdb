/*
* Copyright (c) 2020 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
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

#include "env.h"
#include "util/log_util.h"
#include <algorithm>

#ifdef _WIN32

PDB_CONSTEXPR size_t kWritableFileBufferSize = 65536;

class WindowsNormalFile : public NormalFile
{
public:
  WindowsNormalFile(const char* pPath, HANDLE handle)
    : handle_(handle), path_(pPath) { }

  ~WindowsNormalFile() override
  {
    if (handle_ != INVALID_HANDLE_VALUE)
    {
      ::CloseHandle(handle_);
    }
  }

  PdbErr_t Read(size_t offset, char* pBuf, size_t bytes) override
  {
    DWORD errCode = ERROR_SUCCESS;
    DWORD bytesRead = 0;
    LARGE_INTEGER distance;
    distance.QuadPart = offset;
    if (!::SetFilePointerEx(handle_, distance, NULL, FILE_BEGIN))
    {
      errCode = ::GetLastError();
      LOG_ERROR(LOGFMT_READ_NORMAL_FILE_SETFILEPOINTEREX_FAILED_3PARAM,
        path_.c_str(), offset, errCode);
      return PdbE_IOERR;
    }

    if (!::ReadFile(handle_, pBuf, static_cast<DWORD>(bytes), &bytesRead, NULL))
    {
      errCode = ::GetLastError();
      LOG_ERROR(LOGFMT_READ_NORMAL_FILE_FAILED_4PARAM, 
        path_.c_str(), offset, bytes, errCode);
      return PdbE_IOERR;
    }

    if (static_cast<DWORD>(bytes) != bytesRead)
    {
      LOG_ERROR(LOGFMT_READ_NORMAL_FILE_DATA_LESS_4PARAM,
        path_.c_str(), offset, bytes, bytesRead);
      return PdbE_IOERR;
    }

    return PdbE_OK;
  }

  PdbErr_t Write(size_t offset, const char* pBuf, size_t bytes) override
  {
    DWORD errCode = ERROR_SUCCESS;
    DWORD bytesWritten;
    LARGE_INTEGER distance;
    distance.QuadPart = offset;
    if (!::SetFilePointerEx(handle_, distance, NULL, FILE_BEGIN))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNormalFile::Write SetFilePointerEx ({}) failed, offset({}) errCode:({})",
        path_.c_str(), bytes, errCode);
      return PdbE_IOERR;
    }

    if (!::WriteFile(handle_, pBuf, static_cast<DWORD>(bytes), &bytesWritten, NULL))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNormalFile::Write WriteFile ({}) failed, bytes:({}) errCode:({})",
        path_.c_str(), bytes, errCode);
      return PdbE_IOERR;
    }

    return PdbE_OK;
  }

  PdbErr_t Grow(size_t bytes) override
  {
    DWORD errCode = ERROR_SUCCESS;
    LARGE_INTEGER distance;
    if (!::GetFileSizeEx(handle_, &distance))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNormalFile::Grow GetFileSizeEx ({}) failed, errCode:({})",
        path_.c_str(), errCode);
      return PdbE_IOERR;
    }
    distance.QuadPart += bytes;
    if (!::SetFilePointerEx(handle_, distance, NULL, FILE_BEGIN))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNormalFile::Grow SetFilePointerEx ({}) failed, errCode:({})",
        path_.c_str(), errCode);
      return PdbE_IOERR;
    }

    if (!::SetEndOfFile(handle_))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNormalFile::Grow SetEndOfFile ({}) failed, errCode:({})",
        path_.c_str(), errCode);
      return PdbE_IOERR;
    }
    return PdbE_OK;
  }

  PdbErr_t Sync() override
  {
    if (!::FlushFileBuffers(handle_))
    {
      DWORD errCode = ::GetLastError();
      LOG_ERROR("WindowsNormalFile::Sync FlushFileBuffers ({}) failed, errCode:({})",
        path_.c_str(), errCode);
      return PdbE_IOERR;
    }

    return PdbE_OK;
  }

private:
  HANDLE handle_;
  std::string path_;
};

class WindowsNoBufferFile : public NoBufferFile
{
public:
  WindowsNoBufferFile(const char* pPath, HANDLE handle)
    : handle_(handle), path_(pPath) { }

  ~WindowsNoBufferFile() override
  {
    if (handle_ != INVALID_HANDLE_VALUE)
    {
      ::CloseHandle(handle_);
    }
  }

  PdbErr_t Write(uint64_t offset, const char* pBuf, size_t bytes)
  {
    DWORD errCode = ERROR_SUCCESS;
    DWORD bytesWrite = 0;
    LARGE_INTEGER distance;
    distance.QuadPart = offset;
    if (::SetFilePointerEx(handle_, distance, NULL, FILE_BEGIN))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNoBufferFile::Write SetFilePointerEx ({}) failed, offset:({}) errCode:({})",
        path_.c_str(), offset, errCode);
      return PdbE_IOERR;
    }

    if (!::WriteFile(handle_, pBuf, static_cast<DWORD>(bytes), &bytesWrite, NULL))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNoBufferFile::Write WriteFile ({}) failed, offset:({}) bytes:({}) errCode:({})",
        path_.c_str(), offset, bytes, errCode);
      return PdbE_IOERR;
    }

    if (static_cast<DWORD>(bytes) != bytesWrite)
    {
      LOG_ERROR("WindowsNoBufferFile::Write WriteFile ({}) failed, offset:({}) needWrite:({}) writeBytes:({})",
        path_.c_str(), offset, bytes, bytesWrite);
      return PdbE_IOERR;
    }

    return PdbE_OK;
  }

  PdbErr_t Read(uint64_t offset, char* pBuf, size_t bytes)
  {
    DWORD errCode = ERROR_SUCCESS;
    DWORD bytesRead = 0;
    LARGE_INTEGER distance;
    distance.QuadPart = offset;
    if (::SetFilePointerEx(handle_, distance, NULL, FILE_BEGIN))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNoBufferFile::Read SetFilePointerEx ({}) failed, offset:({}) errCode:({})",
        path_.c_str(), offset, errCode);
      return PdbE_IOERR;
    }

    if (!::ReadFile(handle_, pBuf, static_cast<DWORD>(bytes), &bytesRead, nullptr))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNoBufferFile::Read ReadFile ({}) failed, offset:({}) needRead:({}) readBytes:({})",
        path_.c_str(), offset, bytes, bytesRead);
      return PdbE_IOERR;
    }

    return PdbE_OK;
  }
  
  PdbErr_t Grow(size_t bytes)
  {
    DWORD errCode = ERROR_SUCCESS;
    LARGE_INTEGER distance;
    if (!::GetFileSizeEx(handle_, &distance))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNoBufferFile::Grow GetFileSizeEx ({}) failed, errCode:({})",
        path_.c_str(), errCode);
      return PdbE_IOERR;
    }
    distance.QuadPart += bytes;
    if (!::SetFilePointerEx(handle_, distance, NULL, FILE_BEGIN))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNoBufferFile::Grow SetFilePointerEx ({}) failed, errCode:({})",
        path_.c_str(), errCode);
      return PdbE_IOERR;
    }

    if (!::SetEndOfFile(handle_))
    {
      errCode = ::GetLastError();
      LOG_ERROR("WindowsNoBufferFile::Grow SetEndOfFile ({}) failed, errCode:({})",
        path_.c_str(), errCode);
      return PdbE_IOERR;
    }

    return PdbE_OK;
  }

private:
  HANDLE handle_;
  std::string path_;
};

class WindowsMmapFile : public MmapFile
{
public:
  WindowsMmapFile(const char* pPath, bool readOnly)
    : readOnly_(readOnly), fileHandle_(INVALID_HANDLE_VALUE), mapHandle_(NULL),
    pBasePtr_(nullptr), fileSize_(0), path_(pPath) { }

  ~WindowsMmapFile() override
  {
    Close();
  }

  bool IsOpened() override
  {
    return fileHandle_ != INVALID_HANDLE_VALUE;
  }

  PdbErr_t Open() override
  {
    PdbErr_t retVal = PdbE_OK;
    LARGE_INTEGER tmpSize;
    DWORD errCode = ERROR_SUCCESS;

    if (fileHandle_ == INVALID_HANDLE_VALUE)
    {
      do {
        fileHandle_ = CreateFile(path_.c_str(),
          readOnly_ ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
          0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == fileHandle_)
        {
          retVal = PdbE_IOERR;
          errCode = GetLastError();
          LOG_ERROR("WindowsMmapFile::Open CreateFile ({}) failed, errCode:({})",
            path_.c_str(), errCode);
          break;
        }

        if (!GetFileSizeEx(fileHandle_, &tmpSize))
        {
          retVal = PdbE_IOERR;
          errCode = GetLastError();
          LOG_ERROR("WindowsMmapFile::Open GetFileSizeEx ({}) failed, errCode:({})",
            path_.c_str(), errCode);
          break;
        }

        fileSize_ = tmpSize.QuadPart;
        mapHandle_ = CreateFileMapping(fileHandle_, NULL,
          readOnly_ ? PAGE_READONLY : PAGE_READWRITE,
          0, 0, NULL);
        if (NULL == mapHandle_)
        {
          retVal = PdbE_IOERR;
          errCode = GetLastError();
          LOG_ERROR("WindowsMmapFile::Open CreateFileMapping ({}) failed, errCode:({})",
            path_.c_str(), errCode);
          break;
        }

        pBasePtr_ = MapViewOfFile(mapHandle_,
          readOnly_ ? FILE_MAP_READ : (FILE_MAP_READ | FILE_MAP_WRITE),
          0, 0, 0);
        if (pBasePtr_ == NULL || pBasePtr_ == nullptr)
        {
          retVal = PdbE_IOERR;
          errCode = GetLastError();
          LOG_ERROR("WindowsMmapFile::Open MapViewOfFile ({}) failed, errCode:({})",
            path_.c_str(), errCode);
          break;
        }
      } while (false);

      if (retVal != PdbE_OK)
      {
        Close();
      }
    }

    return retVal;
  }

  PdbErr_t Write(uint64_t offset, const char* pBuf, size_t bytes) override
  {
    if (fileHandle_ == INVALID_HANDLE_VALUE)
      return PdbE_INVALID_PARAM;

    if (readOnly_)
      return PdbE_FILE_READONLY;

    if (offset + bytes > fileSize_)
      return PdbE_INVALID_PARAM;

    char* pDest = static_cast<char*>(pBasePtr_) + offset;
    std::memcpy(pDest, pBuf, bytes);
    return PdbE_OK;
  }

  PdbErr_t Read(uint64_t offset, char** ppBuf, size_t bytes) override
  {
    if (fileHandle_ == INVALID_HANDLE_VALUE || ppBuf == nullptr)
      return PdbE_INVALID_PARAM;

    if (offset + bytes > fileSize_)
      return PdbE_INVALID_PARAM;

    *ppBuf = static_cast<char*>(pBasePtr_) + offset;
    return PdbE_OK;
  }

  PdbErr_t Close() override
  {
    if (fileHandle_ != INVALID_HANDLE_VALUE && !readOnly_)
    {
      Sync();
    }

    if (pBasePtr_ != NULL)
    {
      UnmapViewOfFile(pBasePtr_);
      pBasePtr_ = NULL;
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
    return PdbE_OK;
  }

  PdbErr_t Sync() override
  {
    if (fileHandle_ != INVALID_HANDLE_VALUE && !readOnly_)
      FlushViewOfFile(pBasePtr_, fileSize_);

    return PdbE_OK;
  }

private:
  bool readOnly_;
  HANDLE fileHandle_;
  HANDLE mapHandle_;
  LPVOID pBasePtr_;
  size_t fileSize_;
  std::string path_;
};

class WindowsEnv : public Env
{
public:
  WindowsEnv() = default;
  ~WindowsEnv() override = default;

  PdbErr_t CreateNormalFile(const char* pPath, NormalFile** ppFile) override
  {
    if (pPath == nullptr || ppFile == nullptr)
      return PdbE_INVALID_PARAM;

    HANDLE handle = ::CreateFile(pPath,
      (GENERIC_READ | GENERIC_WRITE),
      0,
      NULL,
      CREATE_NEW,
      FILE_ATTRIBUTE_NORMAL,
      NULL);
    if (INVALID_HANDLE_VALUE == handle)
    {
      DWORD errCode = ::GetLastError();
      LOG_ERROR("WindowsEnv::CreateNormalFile CreateFile ({}) failed, errCode:({})",
        pPath, errCode);
      return PdbE_IOERR;
    }

    *ppFile = new WindowsNormalFile(pPath, handle);
    return PdbE_OK;
  }

  PdbErr_t OpenNormalFile(const char* pPath, NormalFile** ppFile)  override
  {
    if (pPath == nullptr || ppFile == nullptr)
      return PdbE_INVALID_PARAM;

    HANDLE handle = ::CreateFile(pPath,
      (GENERIC_READ | GENERIC_WRITE),
      0,
      NULL,
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL,
      NULL);
    if (INVALID_HANDLE_VALUE == handle)
    {
      DWORD errCode = ::GetLastError();
      LOG_ERROR("WindowsEnv::OpenNormalFile CreateFile ({}) failed, errCode:({})",
        pPath, errCode);
      return PdbE_IOERR;
    }

    *ppFile = new WindowsNormalFile(pPath, handle);
    return PdbE_OK;
  }

  PdbErr_t OpenNoBufferFile(const char* pPath, NoBufferFile** ppFile)  override
  {
    if (pPath == nullptr || ppFile == nullptr)
      return PdbE_INVALID_PARAM;

    HANDLE handle = ::CreateFile(pPath,
      (GENERIC_READ | GENERIC_WRITE),
      0,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_NO_BUFFERING,
      NULL);
    if (INVALID_HANDLE_VALUE == handle)
    {
      DWORD errCode = ::GetLastError();
      LOG_ERROR("WindowsEnv::OpenNoBufferFile CreateFile ({}) failed, errCode:({})",
        pPath, errCode);
      return PdbE_IOERR;
    }

    *ppFile = new WindowsNoBufferFile(pPath, handle);
    return PdbE_OK;
  }

  PdbErr_t OpenMmapFile(const char* pPath, bool readOnly, MmapFile** ppFile)  override
  {
    if (pPath == nullptr || ppFile == nullptr)
      return PdbE_INVALID_PARAM;

    WindowsMmapFile* pMmapFile = new WindowsMmapFile(pPath, readOnly);
    PdbErr_t retVal = pMmapFile->Open();
    if (retVal != PdbE_OK)
    {
      delete pMmapFile;
      return retVal;
    }

    *ppFile = pMmapFile;
    return PdbE_OK;
  }

  bool FileExists(const char* pPath)  override
  {
    DWORD dwAttribute = GetFileAttributes(pPath);
    return (INVALID_FILE_ATTRIBUTES != dwAttribute) && (!(dwAttribute & FILE_ATTRIBUTE_DIRECTORY));
  }

  bool PathExists(const char* pPath) override
  {
    DWORD dwAttribute = GetFileAttributes(pPath);
    return (INVALID_FILE_ATTRIBUTES != dwAttribute) && ((FILE_ATTRIBUTE_DIRECTORY & dwAttribute) != 0);
  }

  PdbErr_t DelFile(const char* pPath)  override
  {
    if (pPath == nullptr)
      return PdbE_INVALID_PARAM;

    if (::DeleteFile(pPath) != FALSE)
    {
      return PdbE_OK;
    }

    LOG_ERROR("WindowsEnv::DelFile ({}) failed, errCode:({})",
      pPath, GetLastError());
    return PdbE_IOERR;
  }

  PdbErr_t CreateDir(const char* pPath)  override
  {
    if (pPath == nullptr)
      return PdbE_INVALID_PARAM;

    if (::CreateDirectory(pPath, nullptr) != FALSE)
      return PdbE_OK;

    LOG_ERROR("WindowsEnv::CreateDir ({}) failed, errCode:({})",
      pPath, GetLastError());
    return PdbE_IOERR;
  }

  PdbErr_t CreateParentDir(const char* pPath) override
  {
    if (pPath == nullptr)
      return PdbE_INVALID_PARAM;

    PdbErr_t retVal = PdbE_OK;
    char tmpPath[MAX_PATH];
    const char* pTmp = pPath;

    while (*pTmp != '\0')
    {
      if ((*pTmp == '\\' || *pTmp == '/') && (pTmp != pPath))
      {
        tmpPath[(pTmp - pPath)] = '\0';
        if (!PathExists(tmpPath))
        {
          retVal = CreateDir(tmpPath);
          if (retVal != PdbE_OK)
            return retVal;
        }
      }

      tmpPath[(pTmp - pPath)] = *pTmp;
      pTmp++;
    }

    return PdbE_OK;
  }

  PdbErr_t GetFileSize(const char* pPath, uint64_t* pFileSize)  override
  {
    if (pPath == nullptr)
      return PdbE_INVALID_PARAM;

    ULARGE_INTEGER fileSize;
    WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
    if (::GetFileAttributesEx(pPath, GetFileExInfoStandard, &fileAttributes)
      != FALSE)
    {
      fileSize.HighPart = fileAttributes.nFileSizeHigh;
      fileSize.LowPart = fileAttributes.nFileSizeLow;

      if (pFileSize != nullptr)
        *pFileSize = fileSize.QuadPart;
      
      return PdbE_OK;
    }

    LOG_ERROR("WindowsEnv::GetFileSize ({}) failed, errCode:({})",
      pPath, GetLastError());
    return PdbE_IOERR;
  }

  PdbErr_t RenameFile(const char* pSrcPath, const char* pTargetPath)  override
  {
    if (pSrcPath == nullptr || pTargetPath == nullptr)
      return PdbE_INVALID_PARAM;

    if (::MoveFile(pSrcPath, pTargetPath) != FALSE)
    {
      return PdbE_OK;
    }

    LOG_ERROR("WindowsEnv::RenameFile ({}) to ({}) failed, errCode:({})",
      pSrcPath, pTargetPath, GetLastError());
    return PdbE_IOERR;
  }

  PdbErr_t GetChildrenFiles(const char* pPath, std::vector<std::string>* pSubVec) override
  {
    std::string fullPath;
    std::string findPattern = pPath; 
    findPattern.append("\\*");
    WIN32_FIND_DATAA findData;
    HANDLE dirHandle = ::FindFirstFileA(findPattern.c_str(), &findData);
    if (dirHandle == INVALID_HANDLE_VALUE) {
      DWORD lastError = ::GetLastError();
      if (lastError == ERROR_FILE_NOT_FOUND) {
        return PdbE_OK;
      }
      return PdbE_IOERR;
    }
    do {
      fullPath.clear();
      fullPath.append(pPath);
      fullPath.append("/");
      fullPath.append(findData.cFileName);
      DWORD dwAttr = GetFileAttributes(fullPath.c_str());
      if (INVALID_FILE_ATTRIBUTES != dwAttr && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
      {
        pSubVec->push_back(findData.cFileName);
      }
    } while (::FindNextFileA(dirHandle, &findData));
    DWORD last_error = ::GetLastError();
    ::FindClose(dirHandle);
    if (last_error != ERROR_NO_MORE_FILES) {
      return PdbE_IOERR;
    }
    return PdbE_OK;
  }
};

Env* Env::Default()
{
  return new WindowsEnv();
}

#endif