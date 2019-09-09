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
#include "storage/comp_block_builder.h"
#include "storage/comp_format.h"
#include "table/field_info.h"
#include "port/os_file.h"
#include <list>

class CompPartBuilder
{
public:
  CompPartBuilder();
  ~CompPartBuilder();

  PdbErr_t Create(uint32_t partCode, const char* pDataPath, 
    const std::vector<FieldInfo>& fieldVec);
  
  PdbErr_t Append(const DBVal* pVal, size_t fieldCnt);
  PdbErr_t Flush();
  PdbErr_t Finish();
  PdbErr_t Abandon();

private:
  PdbErr_t WriteIdxBlk();

private:
  size_t dataOffset_;
  int64_t dayBgTs_;
  uint8_t* pCompBuf_;
  uint8_t* pRawBuf_;

  Arena arena_;
  OSFile dataFile_;

  std::string tmpDataPath_;
  std::string dataPath_;

  CompBlockBuilder blkBuilder_;

  typedef std::pair<int64_t, std::vector<CompBlkIdx>> DevIdxType;
  std::list<DevIdxType> devIdxList_;
};
