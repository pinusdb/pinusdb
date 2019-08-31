#pragma once
#include "internal.h"

class MemMapFile
{
public:
  MemMapFile();
  ~MemMapFile();

  bool IsOpened();
  PdbErr_t Open(const char* pFilePath, bool readOnly);
  uint8_t* GetBaseAddr() { return static_cast<uint8_t*>(pBase_); }
  size_t MemMapSize() const { return fileSize_; }
  PdbErr_t Sync();
  void Close();

private:
  HANDLE fileHandle_;
  HANDLE mapHandle_;
  LPVOID pBase_;
  size_t fileSize_;
  bool readOnly_;
};
