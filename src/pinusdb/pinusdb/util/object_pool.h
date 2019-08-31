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

