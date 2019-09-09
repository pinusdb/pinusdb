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
#include "comp_val_builder.h"
#include <new>
#include <list>
#include <vector>
#include "table\field_info.h"

class CompBlockBuilder
{
public:
  CompBlockBuilder();
  ~CompBlockBuilder();

  PdbErr_t Init(const std::vector<FieldInfo>& fieldVec);
  int64_t GetDevId() const { return devId_; }
  PdbErr_t Append(const DBVal* pVal, size_t fieldCnt);
  size_t GetBlkLen() const { return blkLen_; }
  PdbErr_t Flush(uint8_t* pBuf, size_t* pDataLen, size_t* pRecCnt,
    int64_t* pBgTs, int64_t* pEdTs);

private:
  int64_t devId_;
  int64_t bgTs_;
  int64_t edTs_;
  size_t recCnt_;
  size_t blkLen_;

  std::vector<CompValBuilder*> valVec_;
  Arena* pArena_;
};




