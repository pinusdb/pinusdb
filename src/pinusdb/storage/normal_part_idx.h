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
#include "util/date_time.h"
#include <unordered_map>

typedef struct _MemPageIdx
{
  uint32_t tstamp_;
  int32_t pageNo_;
}MemPageIdx;

typedef struct _NormalPageIdx
{
  int64_t devId_;
  int64_t idxTs_;
  int32_t pageNo_;
  char    pad12[12];
}NormalPageIdx;

class NormalPartIdx
{
public:
  NormalPartIdx();
  ~NormalPartIdx();

  static PdbErr_t Create(const char* pPath, uint32_t partCode);

  PdbErr_t Open(const char* pPath, bool readOnly);
  PdbErr_t Close();

  PdbErr_t AddIdx(int64_t devId, int64_t idxTs, int32_t pageNo);
  PdbErr_t WriteIdx(const std::vector<NormalPageIdx>& idxVec);

  PdbErr_t GetIndex(int64_t devId, int64_t ts, NormalPageIdx* pIdx);
  PdbErr_t GetPrevIndex(int64_t devId, int64_t ts, NormalPageIdx* pIdx);
  PdbErr_t GetNextIndex(int64_t devId, int64_t ts, NormalPageIdx* pIdx);

  uint32_t GetPartCode() const { return static_cast<uint32_t>(bgDayTs_ / MillisPerDay); }
  int32_t GetMaxPageNo() const { return maxPageNo_; }

  void GetAllDevId(std::vector<int64_t>& devIdVec);

private:
  OSFile idxFile_;
  std::string idxPath_;
  size_t curPos_;        //下一个写入的索引文件位置
  int64_t bgDayTs_;
  int64_t edDayTs_;

  bool readOnly_;
  int32_t maxPageNo_;     //当前最大使用的数据页

  std::mutex idxMutex_;
  std::mutex fileMutex_;
  std::unordered_map<int64_t, std::vector<MemPageIdx>*> idxMap_;
};


