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
#include "util/arena.h"
#include <string.h>

typedef unsigned char byte;

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

