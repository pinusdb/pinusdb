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

#include "storage/comp_part_builder.h"
#include "storage/data_part.h"
#include "storage/comp_data_part.h"
#include "zlib.h"

CompPartBuilder::CompPartBuilder()
{
  devId_ = -1;
  bgTstamp_ = 0;
  dataOffset_ = 0;
  pArena_ = nullptr;
  flushSize_ = 0;
}

CompPartBuilder::~CompPartBuilder()
{
  if (pArena_ != nullptr)
    delete pArena_;

  for (size_t idx = 0; idx < valBuilderVec_.size(); idx++)
  {
    delete valBuilderVec_[idx];
  }
  valBuilderVec_.clear();
}

PdbErr_t CompPartBuilder::Create(uint32_t partCode, const char* pDataPath,
  const std::vector<FieldInfo>& fieldVec)
{
  Arena arena;
  PdbErr_t retVal = PdbE_OK;
  dataPath_ = pDataPath;
  tmpDataPath_ = dataPath_ + ".tmp";
  tmpIdxPath_ = dataPath_ + ".idx.tmp";

  for (size_t idx = 0; idx < valBuilderVec_.size(); idx++)
  {
    delete valBuilderVec_[idx];
  }
  valBuilderVec_.clear();
  typeVec_.clear();

  DataFileMeta* pMeta = (DataFileMeta*)arena.AllocateAligned(sizeof(DataFileMeta));

  if (FileTool::FileExists(tmpDataPath_.c_str()))
    FileTool::RemoveFile(tmpDataPath_.c_str());

  if (FileTool::FileExists(tmpIdxPath_.c_str()))
    FileTool::RemoveFile(tmpIdxPath_.c_str());

  retVal = idxFile_.OpenNew(tmpIdxPath_.c_str());
  if (retVal != PdbE_OK)
    return retVal;

  retVal = dataFile_.OpenNew(tmpDataPath_.c_str());
  if (retVal != PdbE_OK)
    return retVal;

  memset(pMeta, 0, sizeof(DataFileMeta));
  strncpy(pMeta->headStr_, COMPRESS_DATA_FILE_HEAD_STR, sizeof(pMeta->headStr_));
  Coding::FixedEncode32(pMeta->pageSize_, 0);
  Coding::FixedEncode32(pMeta->fieldCnt_, static_cast<uint32_t>(fieldVec.size()));
  Coding::FixedEncode32(pMeta->partCode_, partCode);
  Coding::FixedEncode32(pMeta->tableType_, PDB_PART_TYPE_COMPRESS_VAL);

  for (size_t idx = 0; idx < fieldVec.size(); idx++)
  {
    int32_t fieldType = fieldVec[idx].GetFieldType();
    typeVec_.push_back(fieldType);
    strncpy(pMeta->fieldRec_[idx].fieldName_, fieldVec[idx].GetFieldName(), PDB_FILED_NAME_LEN);
    Coding::FixedEncode32(pMeta->fieldRec_[idx].fieldType_, fieldType);
    switch (fieldType)
    {
    case PDB_FIELD_TYPE::TYPE_BOOL:
      valBuilderVec_.push_back(new CompBoolValBuilder());
      break;
    case PDB_FIELD_TYPE::TYPE_INT8:
      valBuilderVec_.push_back(new CompIntValBuilder<PDB_FIELD_TYPE::TYPE_INT8>());
      break;
    case PDB_FIELD_TYPE::TYPE_INT16:
      valBuilderVec_.push_back(new CompIntValBuilder<PDB_FIELD_TYPE::TYPE_INT16>());
      break;
    case PDB_FIELD_TYPE::TYPE_INT32:
      valBuilderVec_.push_back(new CompIntValBuilder<PDB_FIELD_TYPE::TYPE_INT32>());
      break;
    case PDB_FIELD_TYPE::TYPE_REAL2:
    case PDB_FIELD_TYPE::TYPE_REAL3:
    case PDB_FIELD_TYPE::TYPE_REAL4:
    case PDB_FIELD_TYPE::TYPE_REAL6:
    case PDB_FIELD_TYPE::TYPE_INT64:
      valBuilderVec_.push_back(new CompIntValBuilder<PDB_FIELD_TYPE::TYPE_INT64>());
      break;
    case PDB_FIELD_TYPE::TYPE_DATETIME:
      valBuilderVec_.push_back(new CompIntValBuilder<PDB_FIELD_TYPE::TYPE_DATETIME>());
      break;
    case PDB_FIELD_TYPE::TYPE_FLOAT:
      valBuilderVec_.push_back(new CompFloatValBuilder());
      break;
    case PDB_FIELD_TYPE::TYPE_DOUBLE:
      valBuilderVec_.push_back(new CompDoubleValBuilder());
      break;
    case PDB_FIELD_TYPE::TYPE_STRING:
      valBuilderVec_.push_back(new CompStrBlobBuilder<PDB_FIELD_TYPE::TYPE_STRING>());
      break;
    case PDB_FIELD_TYPE::TYPE_BLOB:
      valBuilderVec_.push_back(new CompStrBlobBuilder<PDB_FIELD_TYPE::TYPE_BLOB>());
      break;
    }
  }

  uint32_t crc = StringTool::CRC32(pMeta, (sizeof(DataFileMeta) - 4));
  Coding::FixedEncode32(pMeta->crc_, crc);
  retVal = dataFile_.Write(pMeta, sizeof(DataFileMeta), 0);
  if (retVal != PdbE_OK)
    return retVal;

  dataOffset_ = sizeof(DataFileMeta);
  
  size_t idxVecSize = 1024;
  if (fieldVec.size() < 32)
    idxVecSize = 8192;
  else if (fieldVec.size() < 64)
    idxVecSize = 4096;
  else if (fieldVec.size() < 128)
    idxVecSize = 2048;

  tsIdxVec_.reserve(idxVecSize);
  cmpIdxVec_.reserve(idxVecSize * (fieldVec.size() - 2));

  devId_ = -1;
  flushSize_ = 0;
  return PdbE_OK;
}

PdbErr_t CompPartBuilder::Append(const DBVal* pVal, size_t fieldCnt)
{
  PdbErr_t retVal = PdbE_OK;
  int64_t devId = DBVAL_ELE_GET_INT64(pVal, PDB_DEVID_INDEX);
  if (devId != devId_)
  {
    if (devId_ > 0)
    {
      Flush();
    }

    devId_ = devId;
    bgTstamp_ = DBVAL_ELE_GET_DATETIME(pVal, PDB_TSTAMP_INDEX);
  }

  bool flushFlag = false;
  for (size_t idx = PDB_TSTAMP_INDEX; idx < fieldCnt; idx++)
  {
    retVal = valBuilderVec_[idx]->AppendVal(pVal[idx]);
    if (retVal != PdbE_OK)
      return retVal;

    if (valBuilderVec_[idx]->GetValLen() >= PDB_KB_BYTES(490))
      flushFlag = true;
  }

  if (flushFlag || valBuilderVec_[PDB_TSTAMP_INDEX]->GetValCnt() >= CompBlockRecMaxCnt)
  {
    Flush();
  }

  return PdbE_OK;
}

PdbErr_t CompPartBuilder::Finish()
{
  PdbErr_t retVal = PdbE_OK;

  do {
    retVal = Flush();
    if (retVal != PdbE_OK)
      break;

    retVal = Sync(true);
    if (retVal != PdbE_OK)
      break;

    retVal = WriteIdxBlk();
    if (retVal != PdbE_OK)
      break;
  } while (false);

  dataFile_.Sync();
  dataFile_.Close();

  idxFile_.Close();
  FileTool::RemoveFile(tmpIdxPath_.c_str());

  if (retVal != PdbE_OK)
    FileTool::RemoveFile(tmpDataPath_.c_str());
  else
    FileTool::Rename(tmpDataPath_.c_str(), dataPath_.c_str());

  return retVal;
}

PdbErr_t CompPartBuilder::Abandon()
{
  dataFile_.Close();
  idxFile_.Close();
  FileTool::RemoveFile(tmpDataPath_.c_str());
  FileTool::RemoveFile(tmpIdxPath_.c_str());
  return PdbE_OK;
}

PdbErr_t CompPartBuilder::Flush()
{
  PdbErr_t retVal = PdbE_OK;
  std::string tmpBuf;
  tmpBuf.reserve(PDB_KB_BYTES(512));

  uint32_t crc32 = 0;
  int compRet = 0;
  size_t rawSize = 0;
  uLongf compSize = 0;
  char* pRawBuf = nullptr;
  char* pCmpBuf = &tmpBuf[0];

  if (devId_ > 0)
  {
    for (size_t idx = PDB_TSTAMP_INDEX; idx < valBuilderVec_.size(); idx++)
    {
      valBuilderVec_[idx]->GetData(&pRawBuf, &rawSize);

      compSize = tmpBuf.capacity() - sizeof(CmpBlockHead);
      compRet = compress((Bytef*)(pCmpBuf + sizeof(CmpBlockHead)), &compSize,
        (Bytef*)pRawBuf, (uLong)rawSize);
      if (compRet != Z_OK)
        return PdbE_COMPRESS_ERROR;

      CmpBlockHead* pCmpHead = (CmpBlockHead*)pCmpBuf;
      Coding::FixedEncode16(pCmpHead->fieldType_, static_cast<uint16_t>(typeVec_[idx]));
      Coding::FixedEncode16(pCmpHead->fieldPos_, static_cast<uint16_t>(idx));
      Coding::FixedEncode32(pCmpHead->recCnt_, static_cast<uint32_t>(valBuilderVec_[idx]->GetValCnt()));
      Coding::FixedEncode32(pCmpHead->dataLen_, static_cast<uint32_t>(compSize));
      crc32 = StringTool::CRC32((pCmpBuf + sizeof(CmpBlockHead)), compSize);
      Coding::FixedEncode32(pCmpHead->dataCrc_, crc32);
      crc32 = StringTool::CRC32(pCmpBuf, (sizeof(CmpBlockHead) - sizeof(CmpBlockHead::crc_)));
      Coding::FixedEncode32(pCmpHead->crc_, crc32);

      if (idx == PDB_TSTAMP_INDEX)
      {
        retVal = dataFile_.Write(pCmpBuf, (sizeof(CmpBlockHead) + compSize), dataOffset_);
        if (retVal != PdbE_OK)
          return retVal;

        TsBlkIdx tsIdx;
        Coding::FixedEncode64(tsIdx.devId_, devId_);
        Coding::FixedEncode64(tsIdx.bgTs_, bgTstamp_);
        Coding::FixedEncode64(tsIdx.blkPos_, dataOffset_);
        Coding::FixedEncode32(tsIdx.recCnt_, static_cast<uint32_t>(valBuilderVec_[PDB_TSTAMP_INDEX]->GetValCnt()));
        crc32 = StringTool::CRC32(&tsIdx, (sizeof(TsBlkIdx) - sizeof(TsBlkIdx::crc_)));
        Coding::FixedEncode32(tsIdx.crc_, crc32);
        tsIdxVec_.push_back(tsIdx);

        dataOffset_ += (sizeof(CmpBlockHead) + compSize);
      }
      else
      {
        if (pArena_ == nullptr)
        {
          pArena_ = new Arena();
        }

        CmpBlkIdx blkIdx;
        char* pTmp = pArena_->Allocate(sizeof(CmpBlockHead) + compSize);
        Coding::FixedEncode64(blkIdx.blkPos_, reinterpret_cast<intptr_t>(pTmp));
        Coding::FixedEncode32(blkIdx.blkLen_, static_cast<uint32_t>(sizeof(CmpBlockHead) + compSize));
        Coding::FixedEncode32(blkIdx.crc_, static_cast<uint32_t>(idx));
        memcpy(pTmp, pCmpBuf, (sizeof(CmpBlockHead) + compSize));
        cmpIdxVec_.push_back(blkIdx);
      }
    
      valBuilderVec_[idx]->Reset();
    }
  }

  devId_ = -1;
  if (pArena_->MemoryUsage() > PDB_MB_BYTES(256)
    || tsIdxVec_.size() == tsIdxVec_.capacity())
  {
    retVal = Sync();
    if (retVal != PdbE_OK)
      return retVal;
  }

  return PdbE_OK;
}

PdbErr_t CompPartBuilder::Sync(bool syncAll /* = false*/)
{
  PdbErr_t retVal = PdbE_OK;
  if (tsIdxVec_.size() == 0)
    return PdbE_OK;

  for (size_t fieldIdx = PDB_TSTAMP_INDEX + 1; fieldIdx < valBuilderVec_.size(); fieldIdx++)
  {
    size_t idx = flushSize_ + fieldIdx - (PDB_TSTAMP_INDEX + 1);
    while (idx < cmpIdxVec_.size())
    {
      CmpBlkIdx* pBlkIdx = cmpIdxVec_.data() + idx;

      uint64_t pos = Coding::FixedDecode64(pBlkIdx->blkPos_);
      const char* pData = reinterpret_cast<const char*>(pos);
      Coding::FixedEncode64(pBlkIdx->blkPos_, dataOffset_);
      size_t blkLen = Coding::FixedDecode32(pBlkIdx->blkLen_);
      uint32_t crc = StringTool::CRC32(pBlkIdx, (sizeof(CmpBlkIdx) - sizeof(CmpBlkIdx::crc_)));
      Coding::FixedEncode32(pBlkIdx->crc_, crc);

      retVal = dataFile_.Write(pData, blkLen, dataOffset_);
      if (retVal != PdbE_OK)
      {
        return retVal;
      }

      dataOffset_ += blkLen;
      idx += (valBuilderVec_.size() - 2);
    }
  }

  if (tsIdxVec_.size() == tsIdxVec_.capacity() || syncAll)
  {
    size_t idxFileSize = idxFile_.FileSize();
    retVal = idxFile_.Write(tsIdxVec_.data(), (tsIdxVec_.size() * sizeof(TsBlkIdx)), idxFileSize);
    if (retVal != PdbE_OK)
      return retVal;

    idxFileSize += tsIdxVec_.size() * sizeof(TsBlkIdx);
    std::string tmp;
    tmp.reserve(sizeof(CmpBlkIdx) * tsIdxVec_.size());

    for (size_t fieldIdx = PDB_TSTAMP_INDEX + 1; fieldIdx < valBuilderVec_.size(); fieldIdx++)
    {
      CmpBlkIdx* pBlkIdx = (CmpBlkIdx*)tmp.data();
      size_t idx = fieldIdx - (PDB_TSTAMP_INDEX + 1);
      while (idx < cmpIdxVec_.size())
      {
        *pBlkIdx = cmpIdxVec_[idx];
        pBlkIdx++;
        idx += (valBuilderVec_.size() - 2);
      }
      retVal = idxFile_.Write(tmp.data(), (sizeof(CmpBlkIdx) * tsIdxVec_.size()), idxFileSize);
      if (retVal != PdbE_OK)
        return retVal;

      idxFileSize += (sizeof(CmpBlkIdx) * tsIdxVec_.size());
    }

    idxFile_.Sync();

    idxNumVec_.push_back(tsIdxVec_.size());
    tsIdxVec_.clear();
    cmpIdxVec_.clear();
  }

  flushSize_ = cmpIdxVec_.size();
  delete pArena_;
  pArena_ = nullptr;
  return PdbE_OK;
}

PdbErr_t CompPartBuilder::WriteIdxBlk()
{
  CmpFooter footer;
  PdbErr_t retVal = PdbE_OK;
  uint32_t crc = 0;
  std::string buf;
  buf.reserve(sizeof(TsBlkIdx) * tsIdxVec_.capacity());
  char* pBuf = &buf[0];

  CmpBlockHead* pBlkHead = (CmpBlockHead*)pBuf;
  size_t alignPad = 1024 - ((dataOffset_ + sizeof(CmpBlockHead)) % 1024);
  memset(pBuf, 0, (sizeof(CmpBlockHead) + alignPad));
  Coding::FixedEncode32(pBlkHead->dataLen_, static_cast<uint32_t>(alignPad));
  crc = StringTool::CRC32(pBlkHead, (sizeof(CmpBlockHead) - sizeof(CmpBlockHead::crc_)));
  Coding::FixedEncode32(pBlkHead->crc_, crc);
  retVal = dataFile_.Write(pBuf, (sizeof(CmpBlockHead) + alignPad), dataOffset_);
  if (retVal != PdbE_OK)
    return retVal;

  dataOffset_ += (sizeof(CmpBlockHead) + alignPad);

  Coding::FixedEncode32(footer.magic_, CmpFooterMagic);
  Coding::FixedEncode64(footer.tsIdxPos_, dataOffset_);
  Coding::FixedEncode32(footer.pad32_, 0);
  size_t tsIdxCnt = 0;
  size_t partPos = 0;
  for (size_t idx = 0; idx < idxNumVec_.size(); idx++)
  {
    retVal = idxFile_.Read(pBuf, sizeof(TsBlkIdx) * idxNumVec_[idx], partPos);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = dataFile_.Write(pBuf, sizeof(TsBlkIdx) * idxNumVec_[idx], dataOffset_);
    if (retVal != PdbE_OK)
      return retVal;

    dataOffset_ += sizeof(TsBlkIdx) * idxNumVec_[idx];
    partPos += (sizeof(TsBlkIdx) + sizeof(CmpBlkIdx) * (valBuilderVec_.size() - 2)) * idxNumVec_[idx];
    tsIdxCnt += idxNumVec_[idx];
  }
  Coding::FixedEncode32(footer.tsIdxCnt_, static_cast<uint32_t>(tsIdxCnt));
  crc = StringTool::CRC32(&footer, (sizeof(CmpFooter) - sizeof(CmpFooter::crc_)));
  Coding::FixedEncode32(footer.crc_, crc);

  for (size_t fieldIdx = 0; fieldIdx < (valBuilderVec_.size() - 2); fieldIdx++)
  {
    partPos = 0;
    for (size_t idx = 0; idx < idxNumVec_.size(); idx++)
    {
      size_t offsetPos = (sizeof(TsBlkIdx) + sizeof(CmpBlkIdx) * fieldIdx) * idxNumVec_[idx];
      retVal = idxFile_.Read(pBuf, sizeof(CmpBlkIdx) * idxNumVec_[idx], (partPos + offsetPos));
      if (retVal != PdbE_OK)
        return retVal;

      retVal = dataFile_.Write(pBuf, sizeof(CmpBlkIdx) * idxNumVec_[idx], dataOffset_);
      if (retVal != PdbE_OK)
        return retVal;

      dataOffset_ += sizeof(CmpBlkIdx) * idxNumVec_[idx];
      partPos += (sizeof(TsBlkIdx) + sizeof(CmpBlkIdx) * (valBuilderVec_.size() - 2)) * idxNumVec_[idx];
    }
  }

  return dataFile_.Write(&footer, sizeof(CmpFooter), dataOffset_);
}
