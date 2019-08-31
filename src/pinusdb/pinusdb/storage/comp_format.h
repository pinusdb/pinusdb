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
