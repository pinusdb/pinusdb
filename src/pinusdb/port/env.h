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

#pragma once
#include "internal.h"

class NormalFile;
class NoBufferFile;
class MmapFile;

class Env
{
public:
  Env() = default;
  Env(const Env&) = delete;
  Env& operator=(const Env&) = delete;

  virtual ~Env() = default;
  static Env* Default();

  virtual PdbErr_t CreateNormalFile(const char* pPath, NormalFile** ppFile) = 0;
  virtual PdbErr_t OpenNormalFile(const char* pPath, NormalFile** ppFile) = 0;
  virtual PdbErr_t OpenNoBufferFile(const char* pPath, NoBufferFile** ppFile) = 0;
  virtual PdbErr_t OpenMmapFile(const char* pPath, bool readOnly, MmapFile** ppFile) = 0;

  virtual bool FileExists(const char* pPath) = 0;
  virtual bool PathExists(const char* pPath) = 0;

  virtual PdbErr_t DelFile(const char* pPath) = 0;
  virtual PdbErr_t CreateDir(const char* pPath) = 0;
  virtual PdbErr_t CreateParentDir(const char* pPath) = 0;
  //virtual PdbErr_t DelDir(const char* pPath) = 0;
  virtual PdbErr_t GetFileSize(const char* pPath, uint64_t* pFileSize) = 0;
  virtual PdbErr_t RenameFile(const char* pSrcPath, const char* pTargetPath) = 0;

  virtual PdbErr_t GetChildrenFiles(const char* pPath, std::vector<std::string>* pSubVec) = 0;
};

class NormalFile
{
public:
  NormalFile() = default;
  NormalFile(const NormalFile&) = delete;
  NormalFile& operator=(const NormalFile&) = delete;

  virtual ~NormalFile() = default;

  virtual PdbErr_t Read(size_t offset, char* pBuf, size_t bytes) = 0;
  virtual PdbErr_t Write(size_t offset, const char* pBuf, size_t bytes) = 0;
  virtual PdbErr_t Grow(size_t bytes) = 0;
  virtual PdbErr_t Sync() = 0;
};

class NoBufferFile
{
public:
  NoBufferFile() = default;
  NoBufferFile(const NoBufferFile&) = delete;
  NoBufferFile& operator=(const NoBufferFile&) = delete;

  virtual ~NoBufferFile() = default;

  virtual PdbErr_t Write(uint64_t offset, const char* pBuf, size_t bytes) = 0;
  virtual PdbErr_t Read(uint64_t offset, char* pBuf, size_t bytes) = 0;
  virtual PdbErr_t Grow(size_t bytes) = 0;
};

class MmapFile
{
public:
  MmapFile() = default;
  MmapFile(const MmapFile&) = delete;
  MmapFile& operator=(const MmapFile&) = delete;

  virtual ~MmapFile() = default;

  virtual bool IsOpened() = 0;
  virtual PdbErr_t Open() = 0;
  virtual PdbErr_t Write(uint64_t offset, const char* pBuf, size_t bytes) = 0;
  virtual PdbErr_t Read(uint64_t offset, char** ppBuf, size_t bytes) = 0;
  virtual PdbErr_t Close() = 0;
  virtual PdbErr_t Sync() = 0;
};
