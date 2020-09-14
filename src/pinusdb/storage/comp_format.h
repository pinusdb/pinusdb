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

typedef struct _CmpBlockHead
{
  char fieldType_[2];
  char fieldPos_[2];
  char recCnt_[4];
  char dataLen_[4];
  char dataCrc_[4];
  char crc_[4];
}CmpBlockHead;

typedef struct _TsBlkIdx
{
  char devId_[8];
  char bgTs_[8];
  char blkPos_[8];
  char recCnt_[4];
  char crc_[4];
}TsBlkIdx;

typedef struct _CmpBlkIdx
{
  char blkPos_[8];
  char blkLen_[4];
  char crc_[4];
}CmpBlkIdx;

constexpr uint32_t CmpFooterMagic = 0x7FEC8D92;
typedef struct _CmpFooter
{
  char magic_[4];
  char tsIdxCnt_[4];  //时间戳索引的数据块数量
  char tsIdxPos_[8];  //索引的起始地址
  char pad32_[4];
  char crc_[4];
}CmpFooter;

