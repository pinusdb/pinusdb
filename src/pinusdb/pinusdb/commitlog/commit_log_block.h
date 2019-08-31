#pragma once
#include "internal.h"
#include "util/arena.h"

class CommitLogBlock
{
public:
  CommitLogBlock() {}
  ~CommitLogBlock(){}

  PdbErr_t AppendRec(const byte* pRec, size_t recLen)
  {
    PdbBlob blob;
    if (pRec == nullptr || recLen <= 0)
      return PdbE_INVALID_PARAM;

    byte* pBuf = (byte*)arena_.Allocate(recLen);
    if (pBuf == nullptr)
      return PdbE_NOMEM;

    memcpy(pBuf, pRec, recLen);
    blob.pBlob_ = pBuf;
    blob.len_ = recLen;
    recList_.push_back(blob);
    return PdbE_OK;
  }
  const std::list<PdbBlob>& GetRecList() const
  {
    return recList_;
  }

private:
  Arena arena_;
  std::list<PdbBlob> recList_;
};

