#include "util/arena.h"
#include <assert.h>

static const int kBlockSize = 8192;

Arena::Arena() : memoryUsage_(0)
{
  allocPtr_ = nullptr;
  allocBytesRemaining_ = 0;
}

Arena::~Arena()
{
  for (size_t i = 0; i < blocks_.size(); i++)
  {
    delete[] blocks_[i];
  }
}

char* Arena::AllocateFallback(size_t bytes)
{
  if (bytes > kBlockSize / 8) {
    char* result = AllocateNewBlock(bytes);
    return result;
  }

  allocPtr_ = AllocateNewBlock(kBlockSize);
  allocBytesRemaining_ = kBlockSize;

  char* result = allocPtr_;
  allocPtr_ += bytes;
  allocBytesRemaining_ -= bytes;
  return result;
}

char* Arena::AllocateAligned(size_t bytes)
{
  const int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;
  assert((align & (align - 1)) == 0);
  size_t currentMod = reinterpret_cast<uintptr_t>(allocPtr_) & (align - 1);
  size_t slop = (currentMod == 0 ? 0 : align - currentMod);
  size_t needed = bytes + slop;
  char* result;
  if (needed <= allocBytesRemaining_)
  {
    result = allocPtr_ + slop;
    allocPtr_ += needed;
    allocBytesRemaining_ -= needed;
  }
  else
  {
    // AllocateFallback always returned aligned memory
    result = AllocateFallback(bytes);
  }
  assert((reinterpret_cast<uintptr_t>(result) & (align - 1)) == 0);
  return result;
}

char* Arena::AllocateNewBlock(size_t blockBytes)
{
  char* result = new char[blockBytes];
  blocks_.push_back(result);
  memoryUsage_.store(
    reinterpret_cast<void*>(MemoryUsage() + blockBytes + sizeof(char*)));
  return result;
}