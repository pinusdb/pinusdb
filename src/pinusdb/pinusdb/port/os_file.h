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

class OSFile
{
public:
  OSFile();
  ~OSFile();

  bool IsOpen();
  bool ReadOnly() { return readOnly_; }
  PdbErr_t Open(const char* pPath, bool readOnly, bool create, bool noBuf);
  PdbErr_t OpenNoBuf(const char* pPath, bool readOnly);
  PdbErr_t OpenNormal(const char* pPath, bool readOnly);
  PdbErr_t OpenNew(const char* pPath);
  void Close();
  PdbErr_t Read(void* pBuf, size_t count, size_t offset);
  PdbErr_t Write(const void* pBuf, size_t count, size_t offset);
  PdbErr_t Sync();
  size_t FileSize();
  PdbErr_t Grow(size_t bytes);
  PdbErr_t GrowTo(size_t bytes);

private:
  PdbErr_t _Sync();
  void _Seek(size_t offset);
  size_t _FileSize();

private:
#ifdef _WIN32
  HANDLE handle_;
#else
  int    handle_;
#endif
  bool readOnly_;
  std::mutex fileMutex_;
  std::string path_;
};


class FileTool
{
public:
  static bool FileExists(const char* pPath);
  static bool PathExists(const char* pPath);
  static PdbErr_t RemoveFile(const char* pFile);
  static PdbErr_t Rename(const char* pSourceFile, const char* pDestFile);

  static PdbErr_t MakePath(const char* pPath);

  static PdbErr_t MakeParentPath(const char* pPath);
};

