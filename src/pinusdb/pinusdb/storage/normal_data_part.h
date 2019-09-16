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
#include "query/result_filter.h"
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
    const TableInfo* pTabInfo, int32_t partCode);

  PdbErr_t Open(uint8_t tabCode, int32_t partCode, const TableInfo* pTabInfo,
    const char* pIdxPath, const char* pDataPath);

  virtual void Close();
  virtual PdbErr_t RecoverDW(const char* pPageBuf);
  virtual PdbErr_t InsertRec(int64_t devId, int64_t tstamp,
    bool replace, const uint8_t* pRec, size_t recLen);

  virtual PdbErr_t DumpToCompPart(const char* pDataPath);
  virtual bool SwitchToReadOnly();
  virtual bool IsPartReadOnly() const { return readOnly_; }
  virtual bool IsNormalPart() const { return true; }
  virtual PdbErr_t SyncDirtyPages(bool syncAll, OSFile* pDwFile);
  virtual PdbErr_t AbandonDirtyPages();
  virtual size_t GetDirtyPageCnt() { return dirtyList_.size(); }

protected:
  virtual PdbErr_t QueryDevAsc(int64_t devId, void* pQueryParam,
    IResultFilter* pResult, uint64_t timeOut, bool queryFirst, bool* pIsAdd);
  virtual PdbErr_t QueryDevDesc(int64_t devId, void* pQueryParam,
    IResultFilter* pResult, uint64_t timeOut, bool queryLast, bool* pIsAdd);
  virtual PdbErr_t QueryDevSnapshot(int64_t devId, void* pQueryParam,
    ISnapshotResultFilter* pResult, uint64_t timeOut, bool* pIsAdd);

  PdbErr_t GetPage(int32_t pageNo, PageRef* pPageRef);
  PdbErr_t AllocPage(PageRef* pPageRef);
  PdbErr_t WritePages(const std::vector<PageHdr*>& hdrVec, OSFile* pDwFile);

  virtual void* InitQueryParam(int* pTypes, size_t fieldCnt, int64_t bgTs, int64_t edTs);
  virtual void ClearQueryParam(void* pQueryParam);

  class PageDataIter
  {
  public:
    PageDataIter();
    ~PageDataIter();

    PdbErr_t Init(int* pTypes, size_t fieldCnt, int64_t bgTs, int64_t edTs);
    PdbErr_t InitForDump(const std::vector<FieldInfo>& fieldVec_);
    PdbErr_t Load(PageHdr* pHdr);
    bool Valid() const;
    PdbErr_t SeekTo(int64_t tstamp);
    PdbErr_t SeekToFirst();
    PdbErr_t SeekToLast();
    PdbErr_t Next();
    PdbErr_t Prev();

    size_t GetFieldCnt() const { return fieldCnt_; }
    int64_t GetBgTs() const { return bgTs_; }
    int64_t GetEdTs() const { return edTs_; }
    DBVal* GetRecord();

  private:
    Arena arena_;
    size_t fieldCnt_;
    int* pTypes_;
    DBVal* pVals_;
    int64_t bgTs_;
    int64_t edTs_;

    PageHdr* pHdr_;
    NormalDataPage normalPage_;
    int64_t devId_;
    size_t curIdx_;
  };

  PdbErr_t DumpToCompPartId(int64_t devId, PageDataIter* pDataIter, CompPartBuilder* pPartBuilder);

private:
  std::mutex allocPageMutex_;
  std::mutex dirtyMutex_;
  std::list<PageHdr*> dirtyList_;
  std::vector<FieldInfo> fieldVec_;
  bool readOnly_;

  OSFile dataFile_;
  NormalPartIdx normalIdx_;
  uint64_t pageCodeMask_;
  int32_t nextPageNo_;
};


