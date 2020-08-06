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
#include "port/mem_map_file.h"
#include "util/arena.h"
#include "table/table_info.h"
#include "query/iquery.h"

typedef struct _DevIdPos
{
  int64_t devId_;
  size_t pos_;
}DevIdPos;

class DevIDTable
{
public:
  DevIDTable();
  ~DevIDTable();

  static PdbErr_t Create(const char* pDevPath, const TableInfo* pTabInfo);

  PdbErr_t Open(const char* pPath, const char* pTabName, TableInfo* pTabInfo);
  PdbErr_t Close();
  PdbErr_t Alter(const TableInfo* pTabInfo);

  size_t GetDevCnt() const;
  PdbErr_t DevExist(int64_t devId);
  PdbErr_t AddDev(int64_t devId, PdbStr devName, PdbStr expand);
  PdbErr_t QueryDevId(const IQuery* pQuery, std::list<int64_t>& devIdList);
  PdbErr_t QueryDevId(const IQuery* pQuery, std::list<int64_t>& devIdList,
    size_t queryOffset, size_t queryRecord);
  PdbErr_t QueryDevInfo(const std::string& tabName, IQuery* pFilter);
  PdbErr_t DelDev(const std::string& tabName, const ConditionFilter* pCondition);
  void Flush();

private:
  PdbErr_t _DelDev(int64_t devId);

private:
  MemMapFile devIdFile_;

  //用于插入时判断设备是否存在
  std::mutex insertMutex_;
  std::unordered_set<int64_t> devIdSet_;

  //用于查询数据时筛选设备
  std::mutex queryIdMutex_;
  bool isSortId_;
  std::vector<int64_t> devIdVec_;

  //用于查询设备信息
  std::mutex fileMutex_;
  bool isSortInfo_;
  std::vector<DevIdPos> devIdPosVec_;

  //空闲的位置
  std::list<size_t> freePosList_;
};
