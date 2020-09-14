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
#include "port/os_file.h"
#include "util/arena.h"
#include "util/ker_list.h"
#include "storage/normal_part_idx.h"
#include "storage/normal_data_page.h"
#include "storage/comp_part_builder.h"
#include "util/ref_util.h"
#include "storage/data_part.h"
#include <unordered_map>
#include <unordered_set>

class NormalDataPart : public DataPart
{
public:
  NormalDataPart();
  virtual ~NormalDataPart();

  static PdbErr_t Create(const char* pIdxPath, const char* pDataPath,
    const TableInfo* pTabInfo, uint32_t partCode);

  PdbErr_t Open(uint8_t tabCode, uint32_t partCode,
    const char* pIdxPath, const char* pDataPath);

  virtual void Close();
  virtual PdbErr_t RecoverDW(const char* pPageBuf);
  virtual PdbErr_t InsertRec(uint32_t metaCode, int64_t devId, int64_t tstamp,
    bool replace, const char* pRec, size_t recLen);

  virtual PdbErr_t DumpToCompPart(const char* pDataPath);
  virtual bool SwitchToReadOnly();
  virtual bool IsPartReadOnly() const { return readOnly_; }
  virtual bool IsNormalPart() const { return true; }
  virtual uint32_t GetMetaCode() const { return partMetaCode_; }
  virtual PdbErr_t SyncDirtyPages(bool syncAll, OSFile* pDwFile);
  virtual PdbErr_t AbandonDirtyPages();
  virtual size_t GetDirtyPageCnt() { return dirtyList_.size(); }

protected:
  virtual PdbErr_t QueryDevAsc(int64_t devId, const DataPartQueryParam& queryParam,
    IQuery* pQuery, uint64_t timeOut, bool queryFirst, bool* pIsAdd);
  virtual PdbErr_t QueryDevDesc(int64_t devId, const DataPartQueryParam& queryParam,
    IQuery* pQuery, uint64_t timeOut, bool queryLast, bool* pIsAdd);
  virtual PdbErr_t QueryDevSnapshot(int64_t devId, const DataPartQueryParam& queryParam,
    IQuery* pQuery, uint64_t timeOut, bool* pIsAdd);

  template<bool IsAsc, bool IsSnapshot>
  PdbErr_t QueryDevData(int64_t devId, const DataPartQueryParam& queryParam,
    IQuery* pQuery, uint64_t timeOut, bool querySingle, bool* pIsAdd);

  template<bool QuerySingle, bool IsAsc, bool IsSnapshot>
  PdbErr_t TraversalDataPage(const NormalDataPage* pDataPage, int64_t devId,
    const DataPartQueryParam& queryParam, IQuery* pQuery, bool* pIsAdd);

  PdbErr_t DumpToCompPartId(int64_t devId, CompPartBuilder* pPartBuilder);

  PdbErr_t GetPage(int32_t pageNo, PageRef* pPageRef);
  PdbErr_t AllocPage(PageRef* pPageRef);
  PdbErr_t WritePages(const std::vector<PageHdr*>& hdrVec, OSFile* pDwFile);

  const std::vector<FieldInfo>& GetFieldInfoVec() const override { return fieldInfoVec_; }
  const std::vector<size_t>& GetFieldPosVec() const override { return fieldPosVec_; }
  
private:
  std::mutex allocPageMutex_;
  std::mutex dirtyMutex_;
  std::list<PageHdr*> dirtyList_;
  std::vector<FieldInfo> fieldInfoVec_;
  std::vector<size_t> fieldPosVec_;
  uint32_t partMetaCode_;
  bool readOnly_;

  OSFile dataFile_;
  NormalPartIdx normalIdx_;
  uint64_t pageCodeMask_;
  int32_t nextPageNo_;
};


