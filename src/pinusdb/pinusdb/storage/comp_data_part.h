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
  virtual PdbErr_t RecoverDW(const char* pPageBuf);
  virtual PdbErr_t InsertRec(uint32_t metaCode, int64_t devId, int64_t tstamp,
    bool replace, const uint8_t* pRec, size_t recLen);

  virtual PdbErr_t UnMap();

protected:
  virtual PdbErr_t QueryDevAsc(int64_t devId, void* pQueryParam,
    IResultFilter* pResult, uint64_t timeOut, bool queryFirst, bool* pIsAdd);
  virtual PdbErr_t QueryDevDesc(int64_t devId, void* pQueryParam,
    IResultFilter* pResult, uint64_t timeOut, bool queryLast, bool* pIsAdd);
  virtual PdbErr_t QueryDevSnapshot(int64_t devId, void* pQueryParam,
    ISnapshotResultFilter* pResult, uint64_t timeOut, bool* pIsAdd);

  PdbErr_t InitMemMap();

  PdbErr_t GetIdx(int64_t devId, int64_t ts, CompDevId* pCompDevId, int* pCurIdx);

  virtual void* InitQueryParam(const TableInfo* pQueryInfo, int64_t bgTs, int64_t edTs);
  virtual void ClearQueryParam(void* pQueryParam);

  class CompDataIter
  {
  public:
    CompDataIter();
    ~CompDataIter();

    PdbErr_t Init(const std::vector<FieldInfo>& fieldVec,
      int* pFieldPos, size_t queryFieldCnt, int64_t bgTs, int64_t edTs);
    PdbErr_t Load(const uint8_t* pBuf, size_t bufLen);
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

    PdbErr_t DecodeTstampVals(DBVal* pValBg, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeBoolVals(DBVal* pValBg, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeBigIntVals(DBVal* pValBg, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeDateTimeVals(DBVal* pValBg, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeDoubleVals(DBVal* pValBg, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeStringVals(DBVal* pValBg, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeBlobVals(DBVal* pValBg, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeRealVals(DBVal* pValBg, const uint8_t* pBuf, const uint8_t* pLimit, double multiple);

  private:
    Arena arena_;
    size_t mateCnt_;  //匹配上的字段数量
    size_t fieldCnt_;
    int* pTypes_;
    int* pFieldPos_;
    DBVal* pQueryVals_;
    uint8_t* pRawBuf_;
    int64_t bgTs_;
    int64_t edTs_;

    int32_t recCnt_;
    int32_t curIdx_;
    size_t totalValCnt_;
    DBVal* pAllVals_;
  };

private:
  std::mutex initMutex_;
  uint64_t lastQueryTime_;
  MemMapFile dataMemMap_;
  const uint8_t* pData_;
  const CompDevId* pDevId_;
  std::vector<FieldInfo> fieldVec_;
  int32_t devCnt_;
};




