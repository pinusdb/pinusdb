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

#include "storage/normal_part_idx.h"
#include "util/string_tool.h"
#include "util/log_util.h"
#include "util/date_time.h"
#include "util/coding.h"

#define NORMALIDX_FILE_TYPE_STR_LEN  16
#define NORMALIDX_FILE_TYPE_STR      "NORMAL IDX 1"

#define NORMALIDX_FILE_BLOCK_SIZE   (1024 * 1024)

typedef struct _NormalIdxMeta
{
  char fileType_[NORMALIDX_FILE_TYPE_STR_LEN];
  char partCode_[4];
  char padding_[8];
  char crc_[4];
}NormalIdxMeta;

typedef struct _NormalIdxItem
{
  char _devId_[8];
  char _idxTs_[8];
  char _pageNo_[4];
  char _padding_[8];
  char _crc_[4];
}NormalIdxItem;

#define NormalIdxItem_GetDevId(pIdx)                 Coding::FixedDecode64((pIdx)->_devId_)
#define NormalIdxItem_GetIdxTs(pIdx)                 Coding::FixedDecode64((pIdx)->_idxTs_)
#define NormalIdxItem_GetPageNo(pIdx)                Coding::FixedDecode32((pIdx)->_pageNo_)
#define NormalIdxItem_GetCrc(pIdx)                   Coding::FixedDecode32((pIdx)->_crc_)

#define NormalIdxItem_SetDevId(pIdx, devId)          Coding::FixedEncode64((pIdx)->_devId_, devId)
#define NormalIdxItem_SetIdxTs(pIdx, idxTs)          Coding::FixedEncode64((pIdx)->_idxTs_, idxTs)
#define NormalIdxItem_SetPageNo(pIdx, pageNo)        Coding::FixedEncode32((pIdx)->_pageNo_, pageNo)
#define NormalIdxItem_SetCrc(pIdx, crc)              Coding::FixedEncode32((pIdx)->_crc_, crc)

bool MemPageIdxSortComp(const MemPageIdx& idxA, const MemPageIdx& idxB)
{
  return idxA.idxTs_ < idxB.idxTs_;
}

size_t PageIdxBinarySearch(const std::vector<MemPageIdx>* pIdxVec, int64_t ts)
{
  if (pIdxVec->empty())
    return 0;

  if (pIdxVec->begin()->idxTs_ >= ts)
    return 0;

  if (pIdxVec->back().idxTs_ <= ts)
    return (pIdxVec->size() - 1);

  int lwr = 0;
  int upr = static_cast<int>(pIdxVec->size()) - 1;
  int idx = 0;

  while (lwr <= upr)
  {
    idx = (lwr + upr) / 2;
    if ((*pIdxVec)[idx].idxTs_ > ts)
      upr = idx - 1;
    else if ((*pIdxVec)[(size_t)idx + 1].idxTs_ > ts)
      break;
    else
      lwr = idx + 1;
  }

  return idx;
}

NormalPartIdx::NormalPartIdx()
{
  curPos_ = 0;
  bgDayTs_ = 0;
  edDayTs_ = 0;
  readOnly_ = true;
  maxPageNo_ = 0;
}

NormalPartIdx::~NormalPartIdx()
{
  for (auto devIt = idxMap_.begin(); devIt != idxMap_.end(); devIt++)
  {
    delete devIt->second;
  }
  idxMap_.clear();
  idxFile_.Close();
}

PdbErr_t NormalPartIdx::Create(const char* pPath, uint32_t partCode)
{
  PdbErr_t retVal = PdbE_OK;
  OSFile osFile;
  NormalIdxMeta idxMeta;

  if (partCode > 365 * 10000)
    return PdbE_INVALID_PARAM;

  retVal = osFile.OpenNew(pPath);
  if (retVal != PdbE_OK)
    return retVal;

  do {
    retVal = osFile.GrowTo(NORMALIDX_FILE_BLOCK_SIZE);
    if (retVal != PdbE_OK)
      break;

    memset(&idxMeta, 0, sizeof(NormalIdxMeta));
    strncpy(idxMeta.fileType_, NORMALIDX_FILE_TYPE_STR, sizeof(idxMeta.fileType_));
    Coding::FixedEncode32(idxMeta.partCode_, partCode);
    uint32_t crc = StringTool::CRC32(&idxMeta, (sizeof(NormalIdxMeta) - 4));
    Coding::FixedEncode32(idxMeta.crc_, crc);

    retVal = osFile.Write(&idxMeta, sizeof(NormalIdxMeta), 0);
  } while (false);

  osFile.Close();
  if (retVal != PdbE_OK)
  {
    FileTool::RemoveFile(pPath);
  }

  return retVal;
}

PdbErr_t NormalPartIdx::Open(const char* pPath, bool readOnly)
{
  PdbErr_t retVal = PdbE_OK;
  curPos_ = 0;
  bgDayTs_ = 0;
  edDayTs_ = 0;
  maxPageNo_ = 0;
  readOnly_ = readOnly;
  idxPath_ = pPath;

  std::unique_lock<std::mutex> idxLock(idxMutex_);
  std::unique_lock<std::mutex> fileLock(fileMutex_);

  retVal = idxFile_.OpenNormal(pPath, readOnly);
  if (retVal != PdbE_OK)
    return retVal;

  curPos_ = sizeof(NormalIdxItem);
  Arena tmpArena;
  char* pTmpBuf = tmpArena.Allocate(NORMALIDX_FILE_BLOCK_SIZE);
  if (pTmpBuf == nullptr)
    return PdbE_NOMEM;

  size_t fileSize = idxFile_.FileSize();
  if (fileSize % NORMALIDX_FILE_BLOCK_SIZE != 0)
    return PdbE_IDX_FILE_ERROR;

  retVal = idxFile_.Read(pTmpBuf, NORMALIDX_FILE_BLOCK_SIZE, 0);
  if (retVal != PdbE_OK)
    return retVal;

  //1.1 验证索引文件
  const NormalIdxMeta* pMeta = (const NormalIdxMeta*)pTmpBuf;
  uint32_t crc = Coding::FixedDecode32(pMeta->crc_);
  if (crc != StringTool::CRC32(pTmpBuf, (sizeof(NormalIdxMeta) - 4)))
  {
    LOG_ERROR("failed to open normal index file ({}), file meta crc error ", pPath);
    return PdbE_IDX_FILE_ERROR;
  }

  //1.2 验证文件版本
  if (strncmp(pMeta->fileType_, NORMALIDX_FILE_TYPE_STR, NORMALIDX_FILE_TYPE_STR_LEN) != 0)
  {
    LOG_ERROR("failed to open normal index file ({}), field list mismatch", pPath);
    return PdbE_IDX_FILE_ERROR;
  }

  //1.2 验证开始文件所属天
  uint32_t partCode = Coding::FixedDecode32(pMeta->partCode_);
  if (partCode < 0 || partCode > DateTime::MaxDay)
  {
    LOG_ERROR("failed to open normal index file ({}), datapart code ({}) error",
      pPath, pMeta->partCode_);
    return PdbE_IDX_FILE_ERROR;
  }

  bgDayTs_ = DateTime::MicrosecondPerDay * partCode;
  edDayTs_ = bgDayTs_ + DateTime::MicrosecondPerDay;

  //2. 读取数据内容
  size_t readPos = 0;
  const char* pTmpItem = pTmpBuf;
  MemPageIdx memPageIdx;
  while (true)
  {
    pTmpItem += sizeof(NormalIdxItem);
    if (pTmpItem >= (pTmpBuf + NORMALIDX_FILE_BLOCK_SIZE))
    {
      readPos += NORMALIDX_FILE_BLOCK_SIZE;
      if (readPos >= fileSize)
        break;

      retVal = idxFile_.Read(pTmpBuf, NORMALIDX_FILE_BLOCK_SIZE, readPos);
      if (retVal != PdbE_OK)
        return retVal;

      pTmpItem = pTmpBuf;
    }

    const NormalIdxItem* pIdxItem = (const NormalIdxItem*)pTmpItem;
    int64_t devId = NormalIdxItem_GetDevId(pIdxItem);
    int64_t idxTs = NormalIdxItem_GetIdxTs(pIdxItem);
    int32_t pageNo = NormalIdxItem_GetPageNo(pIdxItem);
    if (NormalIdxItem_GetCrc(pIdxItem) != 0 && devId > 0)
    {
      if (idxTs < bgDayTs_ || idxTs >= edDayTs_)
      {
        LOG_ERROR("normal index file ({}), position ({}) error", pPath, curPos_);
        return PdbE_IDX_FILE_ERROR;
      }

      std::vector<MemPageIdx>* pIdxVec = nullptr;
      auto idxIt = idxMap_.find(devId);
      if (idxIt == idxMap_.end())
      {
        pIdxVec = new std::vector<MemPageIdx>();
        pIdxVec->reserve(16);
        idxMap_.insert(std::pair<int64_t, std::vector<MemPageIdx>*>(devId, pIdxVec));
      }
      else
      {
        pIdxVec = idxIt->second;
      }

      memPageIdx.pageNo_ = pageNo;
      memPageIdx.idxTs_ = idxTs;
      if (pIdxVec->empty() || pIdxVec->back().idxTs_ < memPageIdx.idxTs_)
      {
        pIdxVec->push_back(memPageIdx);
      }
      else
      {
        pIdxVec->push_back(memPageIdx);
        std::sort(pIdxVec->begin(), pIdxVec->end(), MemPageIdxSortComp);
      }

      curPos_ += sizeof(NormalIdxItem);

      if (pageNo > maxPageNo_)
        maxPageNo_ = pageNo;
    }
    else
    {
      break;
    }
  }

  return PdbE_OK;
}

PdbErr_t NormalPartIdx::Close()
{
  std::unique_lock<std::mutex> idxLock(idxMutex_);
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  idxFile_.Close();
  curPos_ = 0;
  for (auto devIt = idxMap_.begin(); devIt != idxMap_.end(); devIt++)
  {
    delete devIt->second;
  }
  idxMap_.clear();

  return PdbE_OK;
}

PdbErr_t NormalPartIdx::AddIdx(int64_t devId, int64_t idxTs, int32_t pageNo)
{
  if (devId <= 0 || idxTs < bgDayTs_ || idxTs >= edDayTs_ || pageNo <= 0)
    return PdbE_INVALID_PARAM;

  MemPageIdx pageIdx;
  pageIdx.idxTs_ = idxTs;
  pageIdx.pageNo_ = pageNo;

  std::unique_lock<std::mutex> idxLock(idxMutex_);
  std::vector<MemPageIdx>* pIdxVec = nullptr;
  auto idxIt = idxMap_.find(devId);
  if (idxIt == idxMap_.end())
  {
    pIdxVec = new std::vector<MemPageIdx>();
    pIdxVec->reserve(16);
    idxMap_.insert(std::pair<int64_t, std::vector<MemPageIdx>*>(devId, pIdxVec));
  }
  else
  {
    pIdxVec = idxIt->second;
  }

  if (pIdxVec->empty() || pIdxVec->back().idxTs_ < pageIdx.idxTs_)
  {
    pIdxVec->push_back(pageIdx);
  }
  else
  {
    pIdxVec->push_back(pageIdx);
    std::sort(pIdxVec->begin(), pIdxVec->end(), MemPageIdxSortComp);
  }

  return PdbE_OK;
}

void NormalPartIdx::AppendIdx(std::string& idxBuf, int64_t devId, int64_t idxTs, int32_t pageNo)
{
  NormalIdxItem idxItem;
  memset(&idxItem, 0, sizeof(NormalIdxItem));
  NormalIdxItem_SetDevId(&idxItem, devId);
  NormalIdxItem_SetPageNo(&idxItem, pageNo);
  NormalIdxItem_SetIdxTs(&idxItem, idxTs);
  uint32_t crc = StringTool::CRC32(&idxItem, (sizeof(NormalIdxItem) - 4));
  NormalIdxItem_SetCrc(&idxItem, crc);

  idxBuf.append((const char*)&idxItem, sizeof(NormalIdxItem));
}

PdbErr_t NormalPartIdx::WriteIdx(const std::string& idxBuf)
{
  if (idxBuf.size() == 0)
    return PdbE_OK;

  if (idxBuf.size() % sizeof(NormalIdxItem) != 0)
    return PdbE_INVALID_PARAM;

  PdbErr_t retVal = PdbE_OK;
  std::unique_lock<std::mutex> fileLock(fileMutex_);
  size_t idxFileSize = idxFile_.FileSize();
  if ((curPos_ + idxBuf.size()) >= idxFileSize)
  {
    retVal = idxFile_.Grow(NORMALIDX_FILE_BLOCK_SIZE);
    if (retVal != PdbE_OK)
      return retVal;
  }
  retVal = idxFile_.Write(idxBuf.data(), idxBuf.size(), curPos_);
  if (retVal != PdbE_OK)
    return retVal;

  idxFile_.Sync();
  curPos_ += idxBuf.size();

  return retVal;
}

PdbErr_t NormalPartIdx::GetIndex(int64_t devId, int64_t ts, MemPageIdx* pIdx)
{
  if (pIdx == nullptr)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> idxLock(idxMutex_);
  auto devIt = idxMap_.find(devId);
  if (devIt != idxMap_.end())
  {
    if (devIt->second->empty())
      return PdbE_IDX_NOT_FOUND;

    std::vector<MemPageIdx>* pIdxVec = devIt->second;
    size_t pos = PageIdxBinarySearch(pIdxVec, ts);
    pIdx->pageNo_ = (*pIdxVec)[pos].pageNo_;
    pIdx->idxTs_ = (*pIdxVec)[pos].idxTs_;
    return PdbE_OK;
  }

  return PdbE_DEV_NOT_FOUND;
}

PdbErr_t NormalPartIdx::GetPrevIndex(int64_t devId, int64_t ts, MemPageIdx* pIdx)
{
  if (pIdx == nullptr)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> idxLock(idxMutex_);
  auto devIt = idxMap_.find(devId);
  if (devIt != idxMap_.end())
  {
    if (devIt->second->empty())
      return PdbE_IDX_NOT_FOUND;

    std::vector<MemPageIdx>* pIdxVec = devIt->second;
    size_t pos = PageIdxBinarySearch(pIdxVec, ts);
    if (pos > 0)
    {
      pIdx->pageNo_ = (*pIdxVec)[pos - 1].pageNo_;
      pIdx->idxTs_ = (*pIdxVec)[pos - 1].idxTs_;
      return PdbE_OK;
    }

    return PdbE_IDX_NOT_FOUND;
  }

  return PdbE_DEV_NOT_FOUND;
}

PdbErr_t NormalPartIdx::GetNextIndex(int64_t devId, int64_t ts, MemPageIdx* pIdx)
{
  if (pIdx == nullptr)
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> idxLock(idxMutex_);
  auto devIt = idxMap_.find(devId);
  if (devIt != idxMap_.end())
  {
    if (devIt->second->empty())
      return PdbE_IDX_NOT_FOUND;

    std::vector<MemPageIdx>* pIdxVec = devIt->second;
    size_t pos = PageIdxBinarySearch(pIdxVec, ts);
    if ((pos + 1) < pIdxVec->size())
    {
      pIdx->pageNo_ = (*pIdxVec)[pos + 1].pageNo_;
      pIdx->idxTs_ = (*pIdxVec)[pos + 1].idxTs_;
      return PdbE_OK;
    }

    return PdbE_IDX_NOT_FOUND;
  }

  return PdbE_DEV_NOT_FOUND;
}

void NormalPartIdx::GetAllDevId(std::vector<int64_t>& devIdVec)
{
  devIdVec.resize(idxMap_.size());
  size_t pos = 0;
  for (auto idxIt = idxMap_.begin(); idxIt != idxMap_.end(); idxIt++)
  {
    devIdVec[pos++] = idxIt->first;
  }

  std::sort(devIdVec.begin(), devIdVec.end());
}
