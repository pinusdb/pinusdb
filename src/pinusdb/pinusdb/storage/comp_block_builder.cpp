#include "storage/comp_block_builder.h"
#include "storage/comp_data_part.h"

CompBlockBuilder::CompBlockBuilder()
{
  devId_ = 0;
  bgTs_ = 0;
  edTs_ = 0;
  recCnt_ = 0;
  blkLen_ = 0;
  pArena_ = nullptr;
}

CompBlockBuilder::~CompBlockBuilder()
{
  if (pArena_ != nullptr)
    delete pArena_;

  for (size_t i = PDB_TSTAMP_INDEX; i < valVec_.size(); i++)
  {
    delete valVec_[i];
  }
}

PdbErr_t CompBlockBuilder::Init(const std::vector<FieldInfo>& fieldVec)
{
  if (!valVec_.empty())
    return PdbE_OBJECT_INITIALIZED;

  valVec_.push_back(nullptr);
  valVec_.push_back(new CompTsValBuilder());
  for (size_t i = (PDB_TSTAMP_INDEX + 1); i < fieldVec.size(); i++)
  {
    CompValBuilder* pValBuilder = nullptr;
    switch (fieldVec[i].GetFieldType())
    {
    case PDB_FIELD_TYPE::TYPE_BOOL:
      pValBuilder = new CompBoolValBuilder();
      break;
    case PDB_FIELD_TYPE::TYPE_REAL2:
    case PDB_FIELD_TYPE::TYPE_REAL3:
    case PDB_FIELD_TYPE::TYPE_REAL4:
    case PDB_FIELD_TYPE::TYPE_REAL6:
    case PDB_FIELD_TYPE::TYPE_INT64:
      pValBuilder = new CompBigIntValBuilder();
      break;
    case PDB_FIELD_TYPE::TYPE_DATETIME:
      pValBuilder = new CompDateTimeValBuilder();
      break;
    case PDB_FIELD_TYPE::TYPE_DOUBLE:
      pValBuilder = new CompDoubleValBuilder();
      break;
    case PDB_FIELD_TYPE::TYPE_STRING:
      pValBuilder = new CompStrBlobBuilder<PDB_FIELD_TYPE::TYPE_STRING>();
      break;
    case PDB_FIELD_TYPE::TYPE_BLOB:
      pValBuilder = new CompStrBlobBuilder<PDB_FIELD_TYPE::TYPE_BLOB>();
      break;
    default:
      return PdbE_INVALID_FIELD_TYPE;
    }
  
    valVec_.push_back(pValBuilder);
  }

  return PdbE_OK;
}

PdbErr_t CompBlockBuilder::Append(const DBVal* pVal, size_t fieldCnt)
{
  if (DBVAL_ELE_GET_DATETIME(pVal, PDB_TSTAMP_INDEX) <= edTs_)
    return PdbE_TSTAMP_DISORDER;

  PdbErr_t retVal = PdbE_OK;
  size_t totalLen = 0;
  DBVal tmpVal;


  edTs_ = DBVAL_ELE_GET_DATETIME(pVal, PDB_TSTAMP_INDEX);
  if (recCnt_ == 0)
  {
    bgTs_ = edTs_;
    devId_ = DBVAL_ELE_GET_INT64(pVal, PDB_DEVID_INDEX);
  }
  else if (devId_ != DBVAL_ELE_GET_INT64(pVal, PDB_DEVID_INDEX))
  {
    return PdbE_INVALID_PARAM;
  }
  for (size_t idx = PDB_TSTAMP_INDEX; idx < fieldCnt; idx++)
  {
    if (DBVAL_ELE_IS_BLOB(pVal, idx) || DBVAL_ELE_IS_STRING(pVal, idx))
    {
      tmpVal = pVal[idx];
      if (DBVAL_ELE_GET_LEN(pVal, idx) > 0)
      {
        if (pArena_ == nullptr)
          pArena_ = new Arena();

        if (pArena_ == nullptr)
          return PdbE_NOMEM;

        uint8_t* pTmpData = (uint8_t*)pArena_->Allocate(DBVAL_ELE_GET_LEN(pVal, idx));
        if (pTmpData == nullptr)
          return PdbE_NOMEM;

        memcpy(pTmpData, DBVAL_ELE_GET_BLOB(pVal, idx), DBVAL_ELE_GET_LEN(pVal, idx));
        tmpVal.val_.pData_ = pTmpData;
      }
      else
      {
        tmpVal.dataLen_ = 0;
      }

      retVal = valVec_[idx]->AppendVal(tmpVal);
    }
    else
    {
      retVal = valVec_[idx]->AppendVal(pVal[idx]);
    }

    if (retVal != PdbE_OK)
      return retVal;

    totalLen += valVec_[idx]->GetValLen();
  }

  blkLen_ = totalLen;

  recCnt_++;

  return PdbE_OK;
}

PdbErr_t CompBlockBuilder::Flush(uint8_t* pBuf, size_t* pDataLen, 
  size_t* pRecCnt, int64_t* pBgTs, int64_t* pEdTs)
{
  if (recCnt_ == 0)
    return PdbE_NODATA;

  CompPageHead* pPageHead = (CompPageHead*)pBuf;
  uint8_t* pTmp = pBuf + sizeof(CompPageHead);

  memset(pBuf, 0, sizeof(CompPageHead));
  uint8_t* pLimit = pBuf + *pDataLen;

  for (size_t idx = PDB_TSTAMP_INDEX; idx < valVec_.size(); idx++)
  {
    pTmp = valVec_[idx]->Flush(pTmp, pLimit);
    if (pTmp == nullptr)
      return PdbE_INVALID_PARAM;

    valVec_[idx]->Clear();
  }

  pPageHead->dataLen_ = static_cast<uint32_t>(pTmp - pBuf);
  pPageHead->devId_ = devId_;
  pPageHead->fieldCnt_ = static_cast<uint16_t>(valVec_.size());
  pPageHead->recCnt_ = static_cast<uint16_t>(recCnt_);
  pPageHead->pageCrc_ = StringTool::CRC32(pBuf, (pTmp - pBuf - 4), 4);

  *pDataLen = (pTmp - pBuf);
  *pRecCnt = recCnt_;
  *pBgTs = bgTs_;
  *pEdTs = edTs_;

  if (pArena_ != nullptr)
    delete pArena_;

  pArena_ = nullptr;
  devId_ = 0;
  bgTs_ = 0;
  edTs_ = 0;
  recCnt_ = 0;
  blkLen_ = 0;

  return PdbE_OK;
}