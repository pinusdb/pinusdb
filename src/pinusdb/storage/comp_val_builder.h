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
#include "util/coding.h"
#include "table/db_value.h"
#include "util/arena.h"
#include <new>
#include <list>
#include <vector>

class CompValBuilder
{
public:
  CompValBuilder();
  virtual ~CompValBuilder();

  virtual PdbErr_t AppendVal(DBVal val) = 0;
  virtual uint8_t* Flush(uint8_t* pBuf, uint8_t* pLimit);
  virtual size_t GetValLen() const { return (pCur_ - pData_); }
  virtual void Clear();

  PdbErr_t GrowDataBuf();

protected:
  uint8_t* pData_;
  uint8_t* pCur_;
  size_t dataLen_;

  int64_t  i64Val_;
  uint64_t u64Val_;
  uint32_t u32Val_;
};

class CompTsValBuilder : public CompValBuilder
{
public:
  CompTsValBuilder() { }
  virtual ~CompTsValBuilder() {}

  virtual PdbErr_t AppendVal(DBVal val);
};

class CompBoolValBuilder : public CompValBuilder
{
public:
  CompBoolValBuilder() { }
  virtual ~CompBoolValBuilder() {}

  virtual PdbErr_t AppendVal(DBVal val);
};

class CompBigIntValBuilder : public CompValBuilder
{
public:
  CompBigIntValBuilder() { }
  virtual ~CompBigIntValBuilder() {}

  virtual PdbErr_t AppendVal(DBVal val);
};

class CompDateTimeValBuilder : public CompValBuilder
{
public:
  CompDateTimeValBuilder() { }
  virtual ~CompDateTimeValBuilder() {}

  virtual PdbErr_t AppendVal(DBVal val);
};

class CompDoubleValBuilder : public CompValBuilder
{
public:
  CompDoubleValBuilder() { }
  virtual ~CompDoubleValBuilder() {}

  virtual PdbErr_t AppendVal(DBVal val);
};

template<int ValType>
class CompStrBlobBuilder : public CompValBuilder
{
public:
  CompStrBlobBuilder()
  {
    valTotalLen_ = 0;
  }

  virtual ~CompStrBlobBuilder() {}

  virtual PdbErr_t AppendVal(DBVal val)
  {
    if (DBVAL_GET_TYPE(&val) != ValType)
      return PdbE_VALUE_MISMATCH;

    valList_.push_back(val);
    valTotalLen_ += DBVAL_GET_LEN(&val);
    if (DBVAL_GET_LEN(&val) < (1 << 7))
      valTotalLen_++;
    else if (DBVAL_GET_LEN(&val) < (1 << 14))
      valTotalLen_ += 2;
    else
      valTotalLen_ += 3;

    return PdbE_OK;
  }

  virtual uint8_t* Flush(uint8_t* pBuf, uint8_t* pLimit)
  {
    if ((pBuf + 3 + valTotalLen_) > pLimit)
      return nullptr;

    pBuf = Coding::VarintEncode32(pBuf, static_cast<uint32_t>(valTotalLen_));
    for (auto valIt = valList_.begin(); valIt != valList_.end(); valIt++)
    {
      pBuf = Coding::VarintEncode32(pBuf, static_cast<uint32_t>(valIt->dataLen_));
      memcpy(pBuf, valIt->val_.pData_, valIt->dataLen_);
      pBuf += valIt->dataLen_;
    }

    return pBuf;
  }

  virtual size_t GetValLen() const { return valTotalLen_; }

  virtual void Clear()
  {
    valList_.clear();
    valTotalLen_ = 0;
  }

protected:
  std::list<DBVal> valList_;
  size_t valTotalLen_;
};

