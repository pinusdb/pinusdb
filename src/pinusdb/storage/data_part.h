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
#include "query/snapshot_result_filter.h"
#include "storage/normal_part_idx.h"
#include "storage/normal_data_page.h"
#include "util/ref_util.h"
#include <atomic>

typedef struct _DataFileMeta
{
  char headStr_[16];            
  uint32_t pageSize_;
  uint32_t fieldCnt_;
  uint32_t partCode_;
  uint32_t tableType_;

  FieldInfoFormat fieldRec_[PDB_TABLE_MAX_FIELD_COUNT];
  char padding_[10460]; //640
  uint32_t crc_;
}DataFileMeta;

class DataPart : public RefObj
{
public:
  DataPart()
  {
    refCnt_ = 0;
    bgDayTs_ = 0;
    edDayTs_ = 0;
  }

  virtual ~DataPart() { }

  virtual void Close() = 0;
  virtual PdbErr_t RecoverDW(const char* pPageBuf) = 0;
  virtual PdbErr_t InsertRec(uint32_t metaCode, int64_t devId, int64_t tstamp, 
    bool replace, const uint8_t* pRec, size_t recLen) = 0;

  PdbErr_t QueryAsc(const std::list<int64_t>& devIdList, int64_t bgTs, int64_t edTs,
    const TableInfo* pTabInfo, IResultFilter* pResult, uint64_t timeOut);
  PdbErr_t QueryDesc(const std::list<int64_t>& devIdList, int64_t bgTs, int64_t edTs,
    const TableInfo* pTabInfo, IResultFilter* pResult, uint64_t timeOut);
  PdbErr_t QueryFirst(std::list<int64_t>& devIdList, int64_t bgTs, int64_t edTs,
    const TableInfo* pTabInfo, IResultFilter* pResult, uint64_t timeOut);
  PdbErr_t QueryLast(std::list<int64_t>& devIdList, int64_t bgTs, int64_t edTs,
    const TableInfo* pTabInfo, IResultFilter* pResult, uint64_t timeOut);
  PdbErr_t QuerySnapshot(std::list<int64_t>& devIdList, 
    const TableInfo* pTabInfo, ISnapshotResultFilter* pResult, uint64_t timeOut);

  virtual PdbErr_t UnMap() { return PdbE_OK; }

  virtual PdbErr_t DumpToCompPart(const char* pDataPath) { return PdbE_OK; }
  virtual bool SwitchToReadOnly() { return true; }
  virtual bool IsPartReadOnly() const { return true; }
  virtual bool IsNormalPart() const { return false; }
  virtual PdbErr_t SyncDirtyPages(bool syncAll, OSFile* pDwFile) { return PdbE_FILE_READONLY; }
  virtual PdbErr_t AbandonDirtyPages() { return PdbE_OK; }
  virtual size_t GetDirtyPageCnt() { return 0; }

  uint32_t GetPartCode() const { return static_cast<uint32_t>(bgDayTs_ / MillisPerDay); }
  std::string GetIdxPath() const { return idxPath_; }
  std::string GetDataPath() const { return dataPath_; }

protected:
  virtual PdbErr_t QueryDevAsc(int64_t devId, void* pQueryParam,
    IResultFilter* pResult, uint64_t timeOut, bool queryFirst, bool* pIsAdd) = 0;
  virtual PdbErr_t QueryDevDesc(int64_t devId, void* pQueryParam,
    IResultFilter* pResult, uint64_t timeOut, bool queryLast, bool* pIsAdd) = 0;
  virtual PdbErr_t QueryDevSnapshot(int64_t devId, void* pQueryParam,
    ISnapshotResultFilter* pResult, uint64_t timeOut, bool* pIsAdd) = 0;

  virtual void* InitQueryParam(const TableInfo* pQueryInfo, int64_t bgTs, int64_t edTs) = 0;
  virtual void ClearQueryParam(void* pQueryParam) = 0;

protected:
  std::string idxPath_;
  std::string dataPath_;

  int64_t bgDayTs_;
  int64_t edDayTs_;
};

