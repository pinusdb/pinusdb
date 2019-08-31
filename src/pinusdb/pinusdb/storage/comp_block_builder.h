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




