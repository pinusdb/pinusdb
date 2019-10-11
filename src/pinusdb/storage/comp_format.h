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

typedef struct _CompBlkIdx
{
  int64_t blkPos_;
  int32_t blkLen_;
  int32_t bgTsForDay_;
}CompBlkIdx;

typedef struct _CompDevId
{
  int64_t devId_;
  int64_t bgPos_;
  int32_t blkIdxCnt_;
  uint32_t allBlkIdxCrc_;
}CompDevId;

typedef struct _CompDataFooter
{
  int64_t blkDataLen_;
  int64_t devIdsPos_;
  int64_t devCnt_;
  uint32_t devCrc_;
  uint32_t footCrc_;
}CompDataFooter;

#define HIS_BLOCK_DATA_MAGIC    0x7CEF82D9 
#define HIS_BLOCK_PAD_MAGIC     0xD9EF7C82

typedef struct _CompBlockHead
{
  uint32_t magic_;
  uint32_t dataLen_;
}CompBlockHead;

typedef struct _CompPageHead
{
  uint32_t pageCrc_;
  uint32_t dataLen_;
  int64_t  devId_;
  uint16_t fieldCnt_;
  uint16_t recCnt_;
  uint32_t padding_;
}CompPageHead;
