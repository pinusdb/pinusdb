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
#include "storage/data_part.h"
#include "storage/comp_format.h"
#include "port/mem_map_file.h"

class CompDataPart : public DataPart
{
public:
  CompDataPart();
  virtual ~CompDataPart();

  PdbErr_t Open(int32_t partCode, const char* pDataPath);

  virtual void Close();

  virtual PdbErr_t UnMap();

protected:
  PdbErr_t QueryDevAsc(int64_t devId, const DataPartQueryParam& queryParam,
    IQuery* pQuery, uint64_t timeOut, bool queryFirst, bool* pIsAdd) override;
  PdbErr_t QueryDevDesc(int64_t devId, const DataPartQueryParam& queryParam,
    IQuery* pQuery, uint64_t timeOut, bool queryLast, bool* pIsAdd) override;
  PdbErr_t QueryDevSnapshot(int64_t devId, const DataPartQueryParam& queryParam,
    IQuery* pQuery, uint64_t timeOut, bool* pIsAdd) override;

  template<bool IsAsc, bool IsSnapshot>
  PdbErr_t QueryDevData(int64_t devId, const DataPartQueryParam& queryParam,
    IQuery* pQuery, uint64_t timeOut, bool querySingle, bool* pIsAdd);

  PdbErr_t InitMemMap();

  PdbErr_t GetDevIdxs(int64_t devId, int64_t bgts, int64_t edTs, std::vector<size_t>& idxVec);

  template<bool IsAsc>
  PdbErr_t TraversalDataPage(size_t idxPos,
    const DataPartQueryParam& queryParam, IQuery* pQuery, bool* pIsAdd);

  const std::vector<FieldInfo>& GetFieldInfoVec() const override;
  const std::vector<size_t>& GetFieldPosVec() const override;
  bool SupportPreWhere() const { return true; }

  template<bool IsAsc>
  PdbErr_t DecodeBoolVals(DBVal* pValsBg, size_t valCnt, const char* pBuf, const char* pLimit);
  template<bool IsAsc, int FieldType>
  PdbErr_t DecodeIntVals(DBVal* pValsBg, size_t valCnt, const char* pBuf, const char* pLimit);
  template<bool IsAsc>
  PdbErr_t DecodeFloatVals(DBVal* pValsBg, size_t valCnt, const char* pBuf, const char* pLimit);
  template<bool IsAsc>
  PdbErr_t DecodeDoubleVals(DBVal* pValsBg, size_t valCnt, const char* pBuf, const char* pLimit);
  template<bool IsAsc, int FieldType>
  PdbErr_t DecodeBlockVals(DBVal* pValsBg, size_t valCnt, const char* pBuf, const char* pLimit, Arena& arena);


  typedef struct _TsIdxItem
  {
    uint32_t bgPos_;
    uint32_t idxCnt_;
  }TsIdxItem;

private:
  std::mutex initMutex_;
  uint64_t lastQueryTime_;
  MemMapFile dataMemMap_;
  const char* pData_;
  size_t fieldIdxCnt_;
  std::vector<size_t> idxPosVec_;
  std::unordered_map<int64_t, TsIdxItem> tsIdxMap_;
  std::vector<FieldInfo> fieldInfoVec_;
  std::vector<size_t> fieldPosVec_;
};




