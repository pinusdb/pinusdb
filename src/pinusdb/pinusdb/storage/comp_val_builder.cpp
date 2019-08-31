#include "storage\comp_val_builder.h"

CompValBuilder::CompValBuilder()
{
  this->pData_ = nullptr;
  this->pCur_ = nullptr;
  this->dataLen_ = 0;

  this->i64Val_ = 0;
  this->u64Val_ = 0;
  this->u32Val_ = 0;
}

CompValBuilder::~CompValBuilder()
{
  if (pData_ != nullptr)
    delete[] pData_;
}

uint8_t* CompValBuilder::Flush(uint8_t* pBuf, uint8_t* pLimit)
{
  size_t valsLen = pCur_ - pData_;

  if ((pBuf + 3 + valsLen) > pLimit)
    return nullptr;

  pBuf = Coding::VarintEncode32(pBuf, static_cast<uint32_t>(valsLen));
  memcpy(pBuf, pData_, valsLen);
  return pBuf + valsLen;
}

void CompValBuilder::Clear()
{
  if (pData_ != nullptr)
    delete[] pData_;

  pData_ = nullptr;
  pCur_ = nullptr;
  dataLen_ = 0;

  i64Val_ = 0;
  u64Val_ = 0;
  u32Val_ = 0;
}

PdbErr_t CompValBuilder::GrowDataBuf()
{
  if (pData_ == nullptr)
  {
    dataLen_ = 512;
    pData_ = new (std::nothrow) uint8_t[dataLen_];
    if (pData_ == nullptr)
      return PdbE_NOMEM;

    memset(pData_, 0, dataLen_);
    pCur_ = pData_;
  }
  else
  {
    size_t newLen = dataLen_ * 2;
    uint8_t* pTmp = new (std::nothrow) uint8_t[newLen];
    if (pTmp == nullptr)
      return PdbE_NOMEM;

    memset(pTmp, 0, newLen);
    memcpy(pTmp, pData_, dataLen_);
    pCur_ = pTmp + (pCur_ - pData_);
    dataLen_ = newLen;
    delete[] pData_;
    pData_ = pTmp;
  }

  return PdbE_OK;
}

//////////////////////////////////////////////////////////////////

PdbErr_t CompTsValBuilder::AppendVal(DBVal val)
{
  if (!DBVAL_IS_DATETIME(&val))
    return PdbE_VALUE_MISMATCH;

  if (DBVAL_GET_DATETIME(&val) <= i64Val_)
    return PdbE_TSTAMP_DISORDER;

  if ((pCur_ + 10) >= (pData_ + dataLen_))
  {
    PdbErr_t retVal = GrowDataBuf();
    if (retVal != PdbE_OK)
      return retVal;
  }

  pCur_ = Coding::VarintEncode64(pCur_, (DBVAL_GET_DATETIME(&val) - i64Val_));
  i64Val_ = DBVAL_GET_DATETIME(&val);
  return PdbE_OK;
}

PdbErr_t CompBoolValBuilder::AppendVal(DBVal val)
{
  if (!DBVAL_IS_BOOL(&val))
    return PdbE_VALUE_MISMATCH;

  if ((pCur_ + 1) >= (pData_ + dataLen_))
  {
    PdbErr_t retVal = GrowDataBuf();
    if (retVal != PdbE_OK)
      return retVal;
  }

  if (DBVAL_GET_BOOL(&val))
    BIT_MAP_SET(pData_, i64Val_);

  i64Val_++;
  pCur_ = (pData_ + (i64Val_ + 7) / 8);
  return PdbE_OK;
}

PdbErr_t CompBigIntValBuilder::AppendVal(DBVal val)
{
  if (!DBVAL_IS_INT64(&val))
    return PdbE_VALUE_MISMATCH;

  if ((pCur_ + 10) >= (pData_ + dataLen_))
  {
    PdbErr_t retVal = GrowDataBuf();
    if (retVal != PdbE_OK)
      return retVal;
  }

  pCur_ = Coding::VarintEncode64(pCur_,
    Coding::ZigzagEncode64(DBVAL_GET_INT64(&val) - i64Val_));
  i64Val_ = DBVAL_GET_INT64(&val);
  return PdbE_OK;
}

PdbErr_t CompDateTimeValBuilder::AppendVal(DBVal val)
{
  if (!DBVAL_IS_DATETIME(&val))
    return PdbE_VALUE_MISMATCH;

  if ((pCur_ + 10) >= (pData_ + dataLen_))
  {
    PdbErr_t retVal = GrowDataBuf();
    if (retVal != PdbE_OK)
      return retVal;
  }

  pCur_ = Coding::VarintEncode64(pCur_,
    Coding::ZigzagEncode64(DBVAL_GET_DATETIME(&val) - i64Val_));
  i64Val_ = DBVAL_GET_DATETIME(&val);
  return PdbE_OK;
}

PdbErr_t CompDoubleValBuilder::AppendVal(DBVal val)
{
  uint32_t zeroCnt = 0;
  uint64_t tmpVal = 0;

  if (!DBVAL_IS_DOUBLE(&val))
    return PdbE_VALUE_MISMATCH;

  if ((pCur_ + 10) >= (pData_ + dataLen_))
  {
    PdbErr_t retVal = GrowDataBuf();
    if (retVal != PdbE_OK)
      return retVal;
  }

  tmpVal = val.val_.u64Val_ ^ u64Val_;
  if (tmpVal == 0)
  {
    *pCur_ = 0;
    pCur_++;
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
      *pCur_ = static_cast<uint8_t>((zeroCnt << 3) | (tmpVal & 0x3));
      pCur_++;
    }
    else
    {
      *pCur_ = static_cast<uint8_t>((zeroCnt << 3) | 0x4 | (tmpVal & 0x3));
      pCur_++;
      tmpVal >>= 2;
      pCur_ = Coding::VarintEncode64(pCur_, tmpVal);
    }
  }

  u64Val_ = val.val_.u64Val_;
  return PdbE_OK;
}


