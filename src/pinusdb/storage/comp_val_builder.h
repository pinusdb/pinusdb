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

const size_t CompBlockRecMaxCnt = 16384;

class CompValBuilder
{
public:
  CompValBuilder(int fieldType)
  {
    this->fieldType_ = fieldType;
    this->data_.reserve(PDB_KB_BYTES(4));
    this->valCnt_ = 0;
  }

  virtual ~CompValBuilder(){}

  virtual PdbErr_t AppendVal(DBVal val) = 0;

  size_t GetValLen() const { return data_.size(); }
  size_t GetValCnt() const { return valCnt_; }

  PdbErr_t GetData(char** ppData, size_t* pSize)
  {
    if (valCnt_ == 0) 
      return PdbE_INVALID_PARAM;

    if (ppData != nullptr)
      *ppData = &data_[0];

    if (pSize != nullptr)
      *pSize = data_.size();

    return PdbE_OK;
  }

  void Reset()
  {
    this->valCnt_ = 0;
    this->data_.clear();
    size_t initSize = ((CompBlockRecMaxCnt + 1023) / 1024) * 1024;
    switch (fieldType_)
    {
    case PDB_FIELD_TYPE::TYPE_BOOL: initSize /= 8; break;
    case PDB_FIELD_TYPE::TYPE_INT8: initSize = initSize * 1 + PDB_KB_BYTES(8); break;
    case PDB_FIELD_TYPE::TYPE_INT16: initSize = initSize * 2 + PDB_KB_BYTES(8); break;
    case PDB_FIELD_TYPE::TYPE_INT32: initSize = initSize * 4 + PDB_KB_BYTES(8); break;
    case PDB_FIELD_TYPE::TYPE_FLOAT: initSize = initSize * 4 + PDB_KB_BYTES(8); break;
    default:
      initSize = PDB_KB_BYTES(128);
    }

    if (data_.capacity() != initSize)
    {
      std::string tmp;
      tmp.reserve(initSize);
      data_.swap(tmp);
    }

    ResetInner();
  }

protected:
  virtual void ResetInner()
  {}

protected:
  int fieldType_;
  size_t valCnt_;
  std::string data_;
};

class CompBoolValBuilder : public CompValBuilder
{
public:
  CompBoolValBuilder() : CompValBuilder(PDB_FIELD_TYPE::TYPE_BOOL){ }
  virtual ~CompBoolValBuilder() {}

  PdbErr_t AppendVal(DBVal val) override
  {
    if (!DBVAL_IS_BOOL(&val))
      return PdbE_VALUE_MISMATCH;

    uint8_t* pResult = (uint8_t*)&(data_[0]);
    if (valCnt_ % 8 == 0)
    {
      data_.append(1, '\0');
    }

    if (DBVAL_GET_BOOL(&val))
    {
      BIT_MAP_SET(pResult, valCnt_);
    }
    valCnt_++;
    return PdbE_OK;
  }
};

template<int FieldType>
class CompIntValBuilder : public CompValBuilder
{
public:
  CompIntValBuilder() : CompValBuilder(FieldType)
  {
    preVal_ = 0;
    preDelta_ = 0;
  }
  virtual ~CompIntValBuilder() {}

  PdbErr_t AppendVal(DBVal val) override
  {
    if (DBVAL_GET_TYPE(&val) != FieldType)
      return PdbE_VALUE_MISMATCH;

    int64_t curVal{};
    if constexpr (FieldType == PDB_VALUE_TYPE::VAL_INT8)
    {
      curVal = DBVAL_GET_INT8(&val);
    }
    else if constexpr (FieldType == PDB_VALUE_TYPE::VAL_INT16)
    {
      curVal = DBVAL_GET_INT16(&val);
    }
    else if constexpr (FieldType == PDB_VALUE_TYPE::VAL_INT32)
    {
      curVal = DBVAL_GET_INT32(&val);
    }
    else if constexpr (FieldType == PDB_VALUE_TYPE::VAL_INT64)
    {
      curVal = DBVAL_GET_INT64(&val);
    }
    else if constexpr (FieldType == PDB_VALUE_TYPE::VAL_DATETIME)
    {
      curVal = DBVAL_GET_DATETIME(&val);
    }

    int64_t delta = curVal - preVal_;

    if (valCnt_ == 0)
    {
      Coding::PutVarint64(&data_, Coding::ZigzagEncode64(curVal));
    }
    else if (valCnt_ == 1)
    {
      Coding::PutVarint64(&data_, Coding::ZigzagEncode64(delta));
      preDelta_ = delta;
    }
    else
    {
      int64_t doubleDelta = delta - preDelta_;
      Coding::PutVarint64(&data_, Coding::ZigzagEncode64(doubleDelta));
      preDelta_ = curVal - preVal_;
    }

    valCnt_++;
    preVal_ = curVal;
    return PdbE_OK;
  }

protected:
  void ResetInner() override
  {
    preVal_ = 0;
    preDelta_ = 0;
  }

protected:
  int64_t preVal_;
  int64_t preDelta_;
};

class CompFloatValBuilder : public CompValBuilder
{
public:
  CompFloatValBuilder() : CompValBuilder(PDB_FIELD_TYPE::TYPE_FLOAT)
  {
    preVal_ = 0;
  }

  virtual ~CompFloatValBuilder() {}

  PdbErr_t AppendVal(DBVal val) override
  {
    uint32_t zeroCnt = 0;
    uint32_t tmpVal = 0;

    if (!DBVAL_IS_FLOAT(&val))
      return PdbE_VALUE_MISMATCH;

    uint32_t curVal = Coding::EncodeFloat(DBVAL_GET_FLOAT(&val));
    tmpVal = curVal ^ preVal_;
    if (tmpVal == 0)
    {
      data_.push_back('\0');
    }
    else {
      while ((tmpVal & 0x3) == 0)
      {
        zeroCnt++;
        tmpVal >>= 2;
      }

      if (tmpVal < 8)
      {
        data_.push_back(static_cast<char>((zeroCnt << 4) | (tmpVal & 0x7)));
      }
      else
      {
        data_.push_back(static_cast<char>((zeroCnt << 4) | 0x8 | (tmpVal & 0x7)));
        tmpVal >>= 3;
        Coding::PutVarint32(&data_, tmpVal);
      }
    }

    preVal_ = curVal;
    valCnt_++;
    return PdbE_OK;
  }

protected:
  void ResetInner() override
  {
    preVal_ = 0;
  }

protected:
  uint32_t preVal_;
};

class CompDoubleValBuilder : public CompValBuilder
{
public:
  CompDoubleValBuilder() : CompValBuilder(PDB_FIELD_TYPE::TYPE_DOUBLE)
  {
    preVal_ = 0;
  }

  virtual ~CompDoubleValBuilder() {}

  PdbErr_t AppendVal(DBVal val) override
  {
    uint64_t zeroCnt = 0;
    uint64_t tmpVal = 0;

    if (!DBVAL_IS_DOUBLE(&val))
      return PdbE_VALUE_MISMATCH;

    uint64_t curVal = Coding::EncodeDouble(DBVAL_GET_DOUBLE(&val));
    tmpVal = curVal ^ preVal_;
    if (tmpVal == 0)
    {
      data_.push_back('\0');
    }
    else
    {
      while ((tmpVal & 0x3) == 0)
      {
        zeroCnt++;
        tmpVal >>= 2;
      }

      if (tmpVal < 4)
      {
        data_.push_back(static_cast<char>((zeroCnt << 3) | (tmpVal & 0x3)));
      }
      else
      {
        data_.push_back(static_cast<char>((zeroCnt << 3) | 0x4 | (tmpVal & 0x3)));
        tmpVal >>= 2;
        Coding::PutVarint64(&data_, tmpVal);
      }
    }

    preVal_ = curVal;
    valCnt_++;
    return PdbE_OK;
  }

protected:
  void ResetInner() override
  {
    preVal_ = 0;
  }

protected:
  uint64_t preVal_;
};

template<int FieldType>
class CompStrBlobBuilder : public CompValBuilder
{
public:
  CompStrBlobBuilder() : CompValBuilder(FieldType)
  {
  }

  virtual ~CompStrBlobBuilder() {}

  PdbErr_t AppendVal(DBVal val) override
  {
    if (DBVAL_GET_TYPE(&val) != FieldType)
      return PdbE_VALUE_MISMATCH;

    Coding::PutVarint32(&data_, DBVAL_GET_LEN(&val));
    data_.append(DBVAL_GET_STRING(&val), DBVAL_GET_LEN(&val));
    valCnt_++;
    return PdbE_OK;
  }
};

