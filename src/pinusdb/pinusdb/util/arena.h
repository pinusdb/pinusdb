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

#include <vector>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <mutex>
#include <atomic>

class Arena
{
public:
  Arena();
  ~Arena();

  char* Allocate(size_t bytes);
  char* AllocateAligned(size_t bytes);

  size_t MemoryUsage() const {
    return reinterpret_cast<uintptr_t>(memoryUsage_.load(std::memory_order_relaxed));
  }

private:
  char* AllocateFallback(size_t bytes);
  char* AllocateNewBlock(size_t blockBytes);

  char* allocPtr_;
  size_t allocBytesRemaining_;

  std::vector<char*> blocks_;

  std::atomic<void*> memoryUsage_;

  // No copying allowed
  Arena(const Arena&);
  void operator=(const Arena&);
};

inline char* Arena::Allocate(size_t bytes)
{
  assert(bytes > 0);
  if (bytes <= allocBytesRemaining_)
  {
    char* result = allocPtr_;
    allocPtr_ += bytes;
    allocBytesRemaining_ -= bytes;
    return result;
  }
  return AllocateFallback(bytes);
}

