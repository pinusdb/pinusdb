#pragma once
#include "storage/data_part.h"
#include "storage/comp_format.h"
#include "port/mem_map_file.h"

class CompDataPart : public DataPart
{
public:
  CompDataPart();
  virtual ~CompDataPart();

  PdbErr_t Open(int32_t partCode, const TableInfo* pTabInfo,
    const char* pDataPath);

  virtual void Close();
  virtual PdbErr_t RecoverDW(const char* pPageBuf);
  virtual PdbErr_t InsertRec(int64_t devId, int64_t tstamp,
    bool replace, const uint8_t* pRec, size_t recLen);

  virtual PdbErr_t UnMap();

protected:
  virtual PdbErr_t QueryDevAsc(int64_t devId, void* pQueryParam,
    IResultFilter* pResult, uint64_t timeOut, bool queryFirst, bool* pIsAdd);
  virtual PdbErr_t QueryDevDesc(int64_t devId, void* pQueryParam,
    IResultFilter* pResult, uint64_t timeOut, bool queryLast, bool* pIsAdd);

  PdbErr_t InitMemMap();

  PdbErr_t GetIdx(int64_t devId, int64_t ts, CompDevId* pCompDevId, int* pCurIdx);

  virtual void* InitQueryParam(int* pTypes, size_t valCnt, int64_t bgTs, int64_t edTs);
  virtual void ClearQueryParam(void* pQueryParam);

  class CompDataIter
  {
  public:
    CompDataIter();
    ~CompDataIter();

    PdbErr_t Init(int* pTypes, size_t fieldCnt, int64_t bgTs, int64_t edTs);
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

    PdbErr_t SetDevIdVals(int64_t devId);
    PdbErr_t DecodeTstampVals(const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeBoolVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeBigIntVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeDateTimeVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeDoubleVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeStringVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeBlobVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit);
    PdbErr_t DecodeRealVals(int fieldIdx, const uint8_t* pBuf, const uint8_t* pLimit, double multiple);

  private:
    Arena arena_;
    size_t fieldCnt_;
    int* pTypes_;
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
  int32_t devCnt_;
};




