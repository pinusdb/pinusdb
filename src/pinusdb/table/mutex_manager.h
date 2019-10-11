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
#include <mutex>
#include <vector>
#include <stdint.h>

class MutexManager
{
public:
  MutexManager()
  {
    ppDevMutex_ = new std::mutex*[kDevMutexCnt / kBlkCnt];
    ppPageMutex_ = new std::mutex*[kPageMutexCnt / kBlkCnt];

    for (size_t i = 0; i < (kDevMutexCnt / kBlkCnt); i++)
    {
      ppDevMutex_[i] = new std::mutex[kBlkCnt];
    }

    for (size_t i = 0; i < (kPageMutexCnt / kBlkCnt); i++)
    {
      ppPageMutex_[i] = new std::mutex[kBlkCnt];
    }
  }

  ~MutexManager()
  {
    if (ppDevMutex_ != nullptr)
    {
      for (size_t i = 0; i < (kDevMutexCnt / kBlkCnt); i++)
      {
        delete[] ppDevMutex_[i];
      }

      delete[] ppDevMutex_;
    }

    if (ppPageMutex_ != nullptr)
    {
      for (size_t i = 0; i < (kPageMutexCnt / kBlkCnt); i++)
      {
        delete[] ppPageMutex_[i];
      }

      delete[] ppPageMutex_;
    }
  }

  std::mutex* GetDevMutex(int64_t devId)
  {
    uint64_t tmpPos = (((uint64_t)devId) % kDevMutexCnt);
    std::mutex* pMutexBlk = ppDevMutex_[tmpPos / kBlkCnt];
    return pMutexBlk + (tmpPos % kBlkCnt);
  }

  std::mutex* GetPageMutex(int32_t pageNo)
  {
    uint32_t tmpPos = (((uint32_t)pageNo) % kPageMutexCnt);
    std::mutex* pMutexBlk = ppPageMutex_[tmpPos / kBlkCnt];
    return pMutexBlk + (tmpPos % kBlkCnt);
  }

private:
  enum {
    kDevMutexCnt = 8192,
    kPageMutexCnt = 32768,
    kBlkCnt = 128,
  };

  std::mutex** ppDevMutex_;
  std::mutex** ppPageMutex_;
};

