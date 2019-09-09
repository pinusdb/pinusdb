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

  bool IsOpen() { return handle_ != INVALID_HANDLE_VALUE; }
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
  HANDLE handle_;
  bool readOnly_;
  std::mutex fileMutex_;
  std::string path_;
};


class FileTool
{
public:
  static void FlushAll();
  static bool FileExists(const char* pPath);
  static bool PathExists(const char* pPath);
  static PdbErr_t RemoveFile(const char* pFile);
  static PdbErr_t Rename(const char* pSourceFile, const char* pDestFile);

  //获取目录所在磁盘的扇区大小
  static PdbErr_t GetBytesPerSector(const char* pPath, int* pBytes);

  static PdbErr_t MakePath(const char* pPath);

  static PdbErr_t MakeParentPath(char* pPath);
};

