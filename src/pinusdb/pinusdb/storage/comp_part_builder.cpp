#include "storage/comp_part_builder.h"
#include "storage/data_part.h"
#include "storage/comp_data_part.h"
#include "zlib.h"

CompPartBuilder::CompPartBuilder()
{
  dataOffset_ = 0;
  dayBgTs_ = 0;
  pCompBuf_ = nullptr;
  pRawBuf_ = nullptr;
}

CompPartBuilder::~CompPartBuilder()
{
}

PdbErr_t CompPartBuilder::Create(uint32_t partCode, const char* pDataPath,
  const std::vector<FieldInfo>& fieldVec)
{
  PdbErr_t retVal = PdbE_OK;
  dayBgTs_ = MillisPerDay * partCode;
  dataPath_ = pDataPath;
  tmpDataPath_ = dataPath_ + ".tmp";
  devIdxList_.clear();

  if (pCompBuf_ == nullptr)
  {
    pCompBuf_ = (uint8_t*)arena_.Allocate(NORMAL_PAGE_SIZE + PDB_KB_BYTES(16));
    pRawBuf_ = (uint8_t*)arena_.Allocate(NORMAL_PAGE_SIZE + PDB_KB_BYTES(16));

    if (pCompBuf_ == nullptr || pRawBuf_ == nullptr)
      return PdbE_NOMEM;
  }

  if (FileTool::FileExists(tmpDataPath_.c_str()))
    FileTool::RemoveFile(tmpDataPath_.c_str());

  retVal = dataFile_.OpenNew(tmpDataPath_.c_str());
  if (retVal != PdbE_OK)
    return retVal;

  DataFileMeta fileMeta;
  memset(&fileMeta, 0, sizeof(DataFileMeta));
  strncpy(fileMeta.headStr_, COMPRESS_DATA_FILE_HEAD_STR, sizeof(fileMeta.headStr_));
  fileMeta.fieldCnt_ = static_cast<uint32_t>(fieldVec.size());
  fileMeta.partCode_ = partCode;
  fileMeta.tableType_ = PDB_PART_TYPE_COMPRESS_VAL;

  for (size_t idx = 0; idx < fieldVec.size(); idx++)
  {
    strncpy(fileMeta.fieldRec_[idx].fieldName_, fieldVec[idx].GetFieldName(), PDB_FILED_NAME_LEN);
    fileMeta.fieldRec_[idx].fieldType_ = fieldVec[idx].GetFieldType();
  }

  fileMeta.crc_ = StringTool::CRC32(&fileMeta, (sizeof(DataFileMeta) - 4));
  retVal = blkBuilder_.Init(fieldVec);
  if (retVal != PdbE_OK)
    return retVal;

  retVal = dataFile_.Write(&fileMeta, sizeof(DataFileMeta), 0);
  if (retVal != PdbE_OK)
    return retVal;

  dataOffset_ = sizeof(DataFileMeta);
  return PdbE_OK;

}

PdbErr_t CompPartBuilder::Append(const DBVal* pVal, size_t fieldCnt)
{
  PdbErr_t retVal = blkBuilder_.Append(pVal, fieldCnt);
  if (retVal != PdbE_OK)
    return retVal;

  if (blkBuilder_.GetBlkLen() >= NORMAL_PAGE_SIZE)
  {
    return Flush();
  }

  return PdbE_OK;
}

PdbErr_t CompPartBuilder::Flush()
{
  PdbErr_t retVal = PdbE_OK;
  CompBlkIdx blkIdx;
  size_t recCnt = 0;
  int64_t bgTs = 0;
  int64_t edTs = 0;

  int64_t devId = blkBuilder_.GetDevId();
  size_t rawLen = NORMAL_PAGE_SIZE + PDB_KB_BYTES(16);
  retVal = blkBuilder_.Flush(pRawBuf_, &rawLen, &recCnt, &bgTs, &edTs);
  if (retVal == PdbE_NODATA)
    return PdbE_OK;

  if (retVal != PdbE_OK)
    return retVal;

  uLongf compLen = NORMAL_PAGE_SIZE + PDB_KB_BYTES(16) - sizeof(CompBlockHead);
  CompBlockHead* pCompHead = (CompBlockHead*)pCompBuf_;

  int compRet = compress((Bytef*)(pCompBuf_ + sizeof(CompBlockHead)), &compLen,
    (Bytef*)pRawBuf_, (uLong)rawLen);
  if (compRet != Z_OK)
    return PdbE_COMPRESS_ERROR;

  pCompHead->magic_ = HIS_BLOCK_DATA_MAGIC;
  pCompHead->dataLen_ = static_cast<uint32_t>(compLen);

  retVal = dataFile_.Write(pCompBuf_, (compLen + sizeof(CompBlockHead)), dataOffset_);
  if (retVal != PdbE_OK)
    return retVal;

  if (devIdxList_.size() == 0 || devIdxList_.back().first != devId)
  {
    DevIdxType devIdPair(devId, std::vector<CompBlkIdx>());
    devIdxList_.push_back(devIdPair);
  }

  blkIdx.blkPos_ = dataOffset_;
  blkIdx.blkLen_ = static_cast<int32_t>(compLen + sizeof(CompBlockHead));
  blkIdx.bgTsForDay_ = static_cast<int32_t>(bgTs - dayBgTs_);
  devIdxList_.back().second.push_back(blkIdx);

  dataOffset_ += (compLen + sizeof(CompBlockHead));
  return PdbE_OK;
}

PdbErr_t CompPartBuilder::Finish()
{
  PdbErr_t retVal = PdbE_OK;

  do {
    retVal = Flush();
    if (retVal != PdbE_OK)
      break;

    retVal = WriteIdxBlk();
    if (retVal != PdbE_OK)
      break;
  } while (false);

  dataFile_.Sync();
  dataFile_.Close();

  if (retVal != PdbE_OK)
    FileTool::RemoveFile(tmpDataPath_.c_str());
  else
    FileTool::Rename(tmpDataPath_.c_str(), dataPath_.c_str());

  return retVal;
}

PdbErr_t CompPartBuilder::Abandon()
{
  dataFile_.Close();
  FileTool::RemoveFile(tmpDataPath_.c_str());
  return PdbE_OK;
}

PdbErr_t CompPartBuilder::WriteIdxBlk()
{
  PdbErr_t retVal = PdbE_OK;
  const int align = PDB_KB_BYTES(8);
  size_t pos = 0;
  CompDataFooter footer;
  std::vector<CompDevId> devIdxVec;
  devIdxVec.resize(devIdxList_.size());

  memset(&footer, 0, sizeof(CompDataFooter));
  footer.blkDataLen_ = dataOffset_;

  //Ð´ÈëÌî³ä¿é
  int curMod = (dataOffset_ + sizeof(CompBlockHead)) & (align - 1);
  int slop = (curMod == 0 ? 0 : align - curMod);
  memset(pRawBuf_, 0, (slop + sizeof(CompBlockHead)));
  ((CompBlockHead*)pRawBuf_)->magic_ = HIS_BLOCK_PAD_MAGIC;
  ((CompBlockHead*)pRawBuf_)->dataLen_ = slop;
  retVal = dataFile_.Write(pRawBuf_, (slop + sizeof(CompBlockHead)), dataOffset_);
  if (retVal != PdbE_OK)
    return retVal;
  dataOffset_ += (slop + sizeof(CompBlockHead));

  for (auto devIt = devIdxList_.begin(); devIt != devIdxList_.end(); devIt++)
  {
    const CompBlkIdx* pBlkIdxs = devIt->second.data();
    size_t idxDataLen = sizeof(CompBlkIdx) * devIt->second.size();
    devIdxVec[pos].devId_ = devIt->first;
    devIdxVec[pos].bgPos_ = dataOffset_;
    devIdxVec[pos].blkIdxCnt_ = static_cast<int32_t>(devIt->second.size());
    devIdxVec[pos].allBlkIdxCrc_ = StringTool::CRC32(pBlkIdxs, idxDataLen);

    retVal = dataFile_.Write(pBlkIdxs, idxDataLen, dataOffset_);
    if (retVal != PdbE_OK)
      return retVal;

    dataOffset_ += idxDataLen;
    pos++;
  }

  const CompDevId* pDevIds = devIdxVec.data();
  size_t devDataLen = sizeof(CompDevId) * devIdxVec.size();
  retVal = dataFile_.Write(pDevIds, devDataLen, dataOffset_);
  if (retVal != PdbE_OK)
    return retVal;

  footer.devIdsPos_ = dataOffset_;
  footer.devCnt_ = devIdxVec.size();
  footer.devCrc_ = StringTool::CRC32(pDevIds, devDataLen);

  footer.footCrc_ = StringTool::CRC32(&footer, (sizeof(CompDataFooter) - 4));

  dataOffset_ += devDataLen;
  retVal = dataFile_.Write(&footer, sizeof(CompDataFooter), dataOffset_);
  if (retVal != PdbE_OK)
    return retVal;

  dataOffset_ += sizeof(CompDataFooter);

  return PdbE_OK;
}
