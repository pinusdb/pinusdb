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

#ifndef _WIN32

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

PDB_CONSTEXPR size_t kWritableFileBufferSize = 65536;

class LinuxNormalFile : public NormalFile
{
public:
  LinuxNormalFile(const char* pPath, int fd)
    : fd_(fd), path_(pPath) { }

  ~LinuxNormalFile() override
  {
    if (fd_ != -1)
    {
      ::close(fd_);
    }
  }

  PdbErr_t Read(size_t offset, char* pBuf, size_t bytes) override
  {
    if (fd_ < 0)
      return PdbE_INVALID_HANDLE;

    ssize_t ret = pread(fd_, pBuf, bytes, offset);
    if (ret == bytes)
      return PdbE_OK;

    LOG_ERROR("failed to read({}) offset({}), count({}), pBuf({}), err:{}",
      path_.c_str(), offset, bytes, reinterpret_cast<uintptr_t>(pBuf), errno);
    return PdbE_IOERR;
  }

  PdbErr_t Write(size_t offset, const char* pBuf, size_t bytes) override
  {
    if (fd_ < 0)
      return PdbE_INVALID_HANDLE;

    ssize_t ret = pwrite(fd_, pBuf, bytes, offset);
    if (ret == bytes)
      return PdbE_OK;

    LOG_ERROR("failed to wirite({}) offset({}), count({}), pBuf({}), err:{}",
      path_.c_str(), offset, bytes, reinterpret_cast<uintptr_t>(pBuf), errno);
    return PdbE_IOERR;
  }

  PdbErr_t Grow(size_t bytes) override
  {
    PdbErr_t retVal = PdbE_OK;
    if (bytes <= 0)
      return PdbE_INVALID_PARAM;

    size_t oldSize = 0;
    retVal = _FileSize(&oldSize);
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("failed to grow({}) get file size error:({})", path_.c_str(), retVal);
      return retVal;
    }

    size_t newSize = oldSize + bytes;
    if (ftruncate(fd_, newSize) < 0)
    {
      LOG_ERROR("grow file ({}:{}) add ({}) failed, errno:({})",
        path_.c_str(), oldSize, bytes, errno);
      return PdbE_IOERR;
    }
    
    return PdbE_OK;
  }

  PdbErr_t Sync() override
  {
    if (fd_ < 0)
      return PdbE_INVALID_HANDLE;

    //if (::fcntl(fd_, F_FULLFSYNC) == 0) {
    //  return PdbE_OK;
    //}

    if (fsync(fd_) != 0)
    {
      LOG_ERROR("failed to fsync({}), err:{}", path_.c_str(), errno);
      return PdbE_IOERR;
    }

    return PdbE_OK;
  }

private:
  PdbErr_t _FileSize(size_t* pFileSize)
  {
    struct stat statbuf;
    if (fd_ < 0)
      return PdbE_INVALID_PARAM;

    if (fstat(fd_, &statbuf) < 0)
    {
      LOG_ERROR("failed to fstat({}), err:{}", path_.c_str(), errno);
      return PdbE_IOERR;
    }

    if (pFileSize != nullptr)
      *pFileSize = statbuf.st_size;

    return PdbE_OK;
  }

private:
  int fd_;
  std::string path_;
};

class LinuxNoBufferFile : public NoBufferFile
{
public:
  LinuxNoBufferFile(const char* pPath, int fd)
    : fd_(fd), path_(pPath) {}

  ~LinuxNoBufferFile() override
  {
    if (fd_ >= 0)
    {
      _Sync();
      close(fd_);
    }
  }

  PdbErr_t Write(uint64_t offset, const char* pBuf, size_t bytes)
  {
    if (fd_ < 0)
      return PdbE_INVALID_HANDLE;

    const char* pTmp = pBuf;
    
    while (bytes > 0) {
      ssize_t write_result = ::write(fd_, pTmp, bytes);
      if (write_result < 0) {
        if (errno == EINTR) {
          continue;
        }

        LOG_ERROR("write file ({}) failed, bytes ({}), errno:{}",
          path_.c_str(), bytes, errno);
        return PdbE_IOERR;
      }

      pTmp += write_result;
      bytes -= write_result;
    }

    return PdbE_OK;
  }

  PdbErr_t Read(uint64_t offset, char* pBuf, size_t bytes)
  {
    if (fd_ < 0)
      return PdbE_INVALID_HANDLE;

    ssize_t ret = pread(fd_, pBuf, bytes, offset);
    if (ret == (ssize_t)bytes)
      return PdbE_OK;
    
    LOG_ERROR("failed to read({}) offset({}) count({}) pBuf({}) err:{}",
      path_.c_str(), offset, bytes, reinterpret_cast<uintptr_t>(pBuf), errno);
    return PdbE_IOERR;
  }

  PdbErr_t Grow(size_t bytes) override
  {
    if (bytes <= 0)
      return PdbE_INVALID_PARAM;

    if (fd_ < 0)
      return PdbE_INVALID_HANDLE;

    struct stat statbuf;
    if (fstat(fd_, &statbuf) < 0)
    {
      LOG_ERROR("grow file ({}) fstat error, err:{}", path_.c_str(), errno);
      return PdbE_IOERR;
    }

    int64_t newBytes = statbuf.st_size + bytes;
    if (ftruncate(fd_, newBytes) < 0)
    {
      LOG_ERROR("grow file ({}) to ({}) failed, errno:{}",
        path_.c_str(), newBytes, errno);
      return PdbE_IOERR;
    }

    return PdbE_OK;
  }


private:
  PdbErr_t _Sync()
  {
    if (fd_ < 0)
      return PdbE_INVALID_HANDLE;

    if (fsync(fd_) < 0)
    {
      LOG_ERROR("failed to fsync({}) err:{}", path_.c_str(), errno);
      return PdbE_IOERR;
    }

    return PdbE_OK;
  }

private:
  int fd_;
  std::string path_;
};

class LinuxMmapFile : public MmapFile
{
public:
  LinuxMmapFile(const char* pPath, bool readOnly)
    : readOnly_(readOnly), fd_(-1),mapFd_(-1),
    pBasePtr_(nullptr), fileSize_(0), path_(pPath)
  {}

  ~LinuxMmapFile() override
  {
    Close();
  }

  bool IsOpened() override
  {
    return fd_ >= 0;
  }

  PdbErr_t Open() override
  {
    if (fd_ >= 0 || pBasePtr_ != nullptr)
      return PdbE_OBJECT_INITIALIZED;

    int flags = O_LARGEFILE | O_NOATIME | O_RDWR;
    int tmpHandle = open(path_.c_str(), flags);
    if (tmpHandle < 0)
    {
      LOG_ERROR("mmap failed, open file ({}) failed, err:({})",
        path_.c_str(), errno);
      return PdbE_IOERR;
    }

    struct stat sbuf;
    if (fstat(tmpHandle, &sbuf) < 0)
    {
      LOG_ERROR("mmap get file({}) size failed, err:({})",
        path_.c_str(), errno);
      close(tmpHandle);
      return PdbE_IOERR;
    }

    fileSize_ = sbuf.st_size;

    void* pTmpAddr = mmap(nullptr, fileSize_,
      (PROT_READ | PROT_WRITE), MAP_SHARED, tmpHandle, 0);
    if (pTmpAddr == MAP_FAILED)
    {
      LOG_ERROR("mmap failed, file({}), err:({})", MAP_SHARED, tmpHandle, 0);
      close(tmpHandle);
      return PdbE_IOERR;
    }

    fd_ = tmpHandle;
    pBasePtr_ = pTmpAddr;
    return PdbE_OK;
  }

  PdbErr_t Write(uint64_t offset, const char* pBuf, size_t bytes) override
  {
    if (fd_ < 0)
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
    if (fd_ < 0)
      return PdbE_INVALID_PARAM;

    if (offset + bytes > fileSize_)
      return PdbE_INVALID_PARAM;

    *ppBuf = static_cast<char*>(pBasePtr_) + offset;
    return PdbE_OK;
  }

  PdbErr_t Close() override
  {
    if (pBasePtr_ != nullptr)
    {
      Sync();
      if (munmap(pBasePtr_, fileSize_) < 0)
      {
        LOG_ERROR("munmap failed");
      }

      pBasePtr_ = nullptr;
    }

    if (fd_ >= 0)
    {
      close(fd_);
      fd_ = -1;
    }

    fileSize_ = 0;
    return PdbE_OK;
  }

  PdbErr_t Sync() override
  {
    if (msync(pBasePtr_, fileSize_, MS_SYNC) != 0)
    {
      LOG_ERROR("msync mmap file failed, err:({})", errno);
      return PdbE_IOERR;
    }

    return PdbE_OK;
  }

private:
  bool readOnly_;
  int fd_;
  int mapFd_;
  void* pBasePtr_;
  size_t fileSize_;
  std::string path_;
};

class LinuxEnv : public Env
{
public:
  LinuxEnv() = default;
  ~LinuxEnv() override = default;

  PdbErr_t CreateNormalFile(const char* pPath, NormalFile** ppFile) override
  {
    if (pPath == nullptr || ppFile == nullptr)
      return PdbE_INVALID_PARAM;

    int flags = O_LARGEFILE | O_NOATIME | O_RDWR | O_CREAT;
    mode_t modes = S_IRUSR | S_IWUSR;
    
    int tmpFd = open(pPath, flags, modes);
    if (tmpFd < 0)
    {
      LOG_ERROR("create file ({}) failed({})",
        pPath, errno);
      return PdbE_IOERR;
    }

    *ppFile = new LinuxNormalFile(pPath, tmpFd);
    return PdbE_OK;
  }

  PdbErr_t OpenNormalFile(const char* pPath, NormalFile** ppFile) override
  {
    if (pPath == nullptr || ppFile == nullptr)
      return PdbE_INVALID_PARAM;

    int flags = O_LARGEFILE | O_NOATIME | O_RDWR;
    mode_t modes = 0;

    int tmpFd = open(pPath, flags, modes);
    if (tmpFd < 0)
    {
      LOG_ERROR("open normal file ({}) failed ({})",
        pPath, errno);
      return PdbE_IOERR;
    }

    *ppFile = new LinuxNormalFile(pPath, tmpFd);
    return PdbE_OK;
  }

  PdbErr_t OpenNoBufferFile(const char* pPath, NoBufferFile** ppFile) override
  {
    if (pPath == nullptr || ppFile == nullptr)
      return PdbE_INVALID_PARAM;

    int flags = O_LARGEFILE | O_NOATIME | O_RDWR | O_DIRECT;
    mode_t modes = 0;

    int tmpFd = open(pPath, flags, modes);
    if (tmpFd < 0)
    {
      LOG_ERROR("open no buffer file ({}) failed ({})",
        pPath, errno);
      return PdbE_IOERR;
    }

    *ppFile = new LinuxNoBufferFile(pPath, tmpFd);
    return PdbE_OK;
  }

  PdbErr_t OpenMmapFile(const char* pPath, bool readOnly, MmapFile** ppFile) override
  {
    if (pPath == nullptr || ppFile == nullptr)
      return PdbE_INVALID_PARAM;

    LinuxMmapFile* pMmapFile = new LinuxMmapFile(pPath, readOnly);
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
    struct stat sbuf;
    if (stat(pPath, &sbuf) < 0)
      return false;

    return S_ISREG(sbuf.st_mode);
  }

  bool PathExists(const char* pPath) override
  {
    struct stat sbuf;
    if (stat(pPath, &sbuf) < 0)
      return false;

    return S_ISDIR(sbuf.st_mode);
  }

  PdbErr_t DelFile(const char* pPath)  override
  {
    if (pPath == nullptr)
      return PdbE_INVALID_PARAM;

    if (unlink(pPath) < 0)
    {
      LOG_ERROR("failed to unlink ({}), errno:{}", pPath, errno);
      return errno;
    }

    return PdbE_OK;
  }

  PdbErr_t CreateDir(const char* pPath)  override
  {
    if (mkdir(pPath, S_IRWXU | S_IRWXG | S_IROTH) < 0)
      return PdbE_IOERR;

    return PdbE_OK;
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

    struct stat buf;
    int fd = open(pPath, O_RDONLY);
    if (fd < 0)
      return PdbE_IOERR;

    fstat(fd, &buf);
    if (pFileSize != nullptr)
      *pFileSize = buf.st_size;

    close(fd);
    return PdbE_OK;
  }

  PdbErr_t RenameFile(const char* pSrcPath, const char* pTargetPath)  override
  {
    if (pSrcPath == nullptr || pTargetPath == nullptr)
      return PdbE_INVALID_PARAM;

    if (rename(pSrcPath, pTargetPath) < 0)
      return errno;

    return PdbE_OK;
  }

  PdbErr_t GetChildrenFiles(const char* pPath, std::vector<std::string>* pSubVec) override
  {
    DIR* pDir = nullptr;
    struct dirent* ptr = nullptr;

    if ((pDir = opendir(pPath)) == nullptr) {
      return PdbE_IOERR;
    }

    while ((ptr = readdir(pDir)) != nullptr)
    {
      if (strcmp(ptr->d_name, ".") || strcmp(ptr->d_name, "..") == 0)
        continue;

      if (ptr->d_type == DT_REG)
      {
        pSubVec->push_back(ptr->d_name);
      }
    }

    closedir(pDir);
    return PdbE_OK;
  }
};

Env* Env::Default()
{
  return new LinuxEnv();
}

#endif


