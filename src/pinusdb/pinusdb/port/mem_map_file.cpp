#include "port/mem_map_file.h"

MemMapFile::MemMapFile()
{
  fileHandle_ = INVALID_HANDLE_VALUE;
  mapHandle_ = NULL;
  pBase_ = NULL;
  fileSize_ = 0;
  readOnly_ = true;
}

MemMapFile::~MemMapFile()
{
  Close();
}

bool MemMapFile::IsOpened()
{
  return fileHandle_ != INVALID_HANDLE_VALUE;
}

PdbErr_t MemMapFile::Open(const char* pFilePath, bool readOnly)
{
  PdbErr_t retVal = PdbE_OK;
  LARGE_INTEGER tmpSize;

  readOnly_ = readOnly;
  if (fileHandle_ != INVALID_HANDLE_VALUE)
    return PdbE_OBJECT_INITIALIZED;

  do {

    fileHandle_ = CreateFile(pFilePath,
      readOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
      0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == fileHandle_)
    {
      retVal = PdbE_IOERR;
      break;
    }

    if (GetFileSizeEx(fileHandle_, &tmpSize) == FALSE)
    {
      retVal = PdbE_IOERR;
      break;
    }

    fileSize_ = tmpSize.QuadPart;
    mapHandle_ = CreateFileMapping(fileHandle_, NULL,
      readOnly ? PAGE_READONLY : PAGE_READWRITE,
      0, 0, NULL);
    if (NULL == mapHandle_)
    {
      retVal = PdbE_IOERR;
      break;
    }

    pBase_ = MapViewOfFile(mapHandle_,
      readOnly ? FILE_MAP_READ : (FILE_MAP_READ | FILE_MAP_WRITE),
      0, 0, 0);
    if (NULL == pBase_)
    {
      retVal = PdbE_IOERR;
      break;
    }
  } while (false);

  if (retVal != PdbE_OK)
  {
    Close();
  }

  return retVal;
}

PdbErr_t MemMapFile::Sync()
{
  if (!readOnly_)
    FlushViewOfFile(pBase_, fileSize_);

  return PdbE_OK;
}

void MemMapFile::Close()
{
  if (pBase_ != NULL)
  {
    if (!readOnly_)
      FlushViewOfFile(pBase_, fileSize_);

    UnmapViewOfFile(pBase_);
    pBase_ = NULL;
  }

  if (mapHandle_ != NULL)
  {
    CloseHandle(mapHandle_);
    mapHandle_ = NULL;
  }

  if (fileHandle_ != INVALID_HANDLE_VALUE)
  {
    CloseHandle(fileHandle_);
    fileHandle_ = INVALID_HANDLE_VALUE;
  }

  fileSize_ = 0;
  readOnly_ = true;
}

