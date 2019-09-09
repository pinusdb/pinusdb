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

#include "util/object_pool.h"

ObjectPool::ObjectPool(int objSize, int maxCnt, int cntPerBlk)
{
  INIT_LIST_HEAD(&(this->freeList_));

  this->objSize_ = (objSize + sizeof(void*) - 1) & (~(sizeof(void*) - 1));
  this->cntPerBlk_ = cntPerBlk;
  this->maxBlkCnt_ = (maxCnt + cntPerBlk - 1) / cntPerBlk;
  this->curBlkCnt_ = 0;
  this->ppBlk_ = new uint8_t*[maxBlkCnt_];
}

ObjectPool::~ObjectPool()
{
  for (int i = 0; i < curBlkCnt_; i++)
  {
    delete[] ppBlk_[i];
  }

  delete[]this->ppBlk_;
}

uint8_t* ObjectPool::MallocObject()
{
  if (list_empty(&freeList_) && curBlkCnt_ < maxBlkCnt_)
  {
    uint8_t* pNewBlk = new uint8_t[((sizeof(struct list_head) + objSize_) * cntPerBlk_)];
    ppBlk_[curBlkCnt_++] = pNewBlk;

    for (int i = 0; i < cntPerBlk_; i++)
    {
      uint8_t* pTmpData = (pNewBlk + (i * (sizeof(struct list_head) + objSize_)));

      struct list_head* pNode = (struct list_head*)(pTmpData);
      INIT_NODE(pNode);
      list_add(pNode, (&freeList_));
    }
  }

  if (!list_empty(&freeList_))
  {
    struct list_head* pTmp = freeList_.next;
    list_del(pTmp);
    return (((uint8_t*)pTmp) + sizeof(struct list_head));
  }

  return nullptr;
}

void ObjectPool::FreeObject(uint8_t* pObj)
{
  struct list_head* pNode = (struct list_head*)(pObj - sizeof(struct list_head));
  INIT_NODE(pNode);
  list_add_tail(pNode, (&freeList_));
}


