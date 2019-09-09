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

class MemMapFile
{
public:
  MemMapFile();
  ~MemMapFile();

  bool IsOpened();
  PdbErr_t Open(const char* pFilePath, bool readOnly);
  uint8_t* GetBaseAddr() { return static_cast<uint8_t*>(pBase_); }
  size_t MemMapSize() const { return fileSize_; }
  PdbErr_t Sync();
  void Close();

private:
  HANDLE fileHandle_;
  HANDLE mapHandle_;
  LPVOID pBase_;
  size_t fileSize_;
  bool readOnly_;
};
