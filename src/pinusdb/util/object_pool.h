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

#include <stdint.h>
#include "util/ker_list.h"

class ObjectPool
{
public:
  ObjectPool(int objSize, int maxCnt, int cntPerBlk);

  virtual ~ObjectPool();

  uint8_t* MallocObject();

  void FreeObject(uint8_t* pObj);

private:
  struct list_head freeList_;     // 空闲链表

  int objSize_;                   // 对象大小
  int cntPerBlk_;                 // 每个块存储的对象数

  int maxBlkCnt_;                 // 最大块数量
  int curBlkCnt_;                 // 当前块数量
  uint8_t** ppBlk_;                  // 数据块指针
};

