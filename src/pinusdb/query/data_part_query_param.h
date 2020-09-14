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
#include <vector>

typedef struct _FieldQueryMapping
{
  int fieldType_;
  size_t queryFieldPos_;
  size_t storeFieldPos_;
}FieldQueryMapping;

class DataPartQueryParam
{
public:
  DataPartQueryParam()
  {
    bgTs_ = 0;
    edTs_ = 0;
    queryFieldCnt_ = 0;
  }

  ~DataPartQueryParam() = default;

  DataPartQueryParam(const DataPartQueryParam&) = delete;
  void operator=(const DataPartQueryParam&) = delete;

  void InitTstampRange(int64_t bgTs, int64_t edTs)
  {
    this->bgTs_ = bgTs;
    this->edTs_ = edTs;
  }

  void InitQueryFieldCnt(size_t queryFieldCnt)
  {
    this->queryFieldCnt_ = queryFieldCnt;
  }

  void AddQueryField(int fieldType, size_t queryFieldPos, size_t storeFieldPos)
  {
    FieldQueryMapping queryField;
    queryField.fieldType_ = fieldType;
    queryField.queryFieldPos_ = queryFieldPos;
    queryField.storeFieldPos_ = storeFieldPos;
    queryFieldVec_.push_back(queryField);
  }

  void AddPreWhereField(int fieldType, size_t queryFieldPos, size_t storeFieldPos)
  {
    FieldQueryMapping queryField;
    queryField.fieldType_ = fieldType;
    queryField.queryFieldPos_ = queryFieldPos;
    queryField.storeFieldPos_ = storeFieldPos;
    queryFieldVec_.push_back(queryField);
  }

  int64_t GetBgTs() const { return bgTs_; }
  int64_t GetEdTs() const { return edTs_; }
  size_t GetQueryFieldCnt() const { return queryFieldCnt_; }
  const std::vector<FieldQueryMapping>& GetQueryFieldVec() const { return queryFieldVec_; }
  const std::vector<FieldQueryMapping>& GetPreWhereFieldVec() const { return preWhereFieldVec_; }

private:
  int64_t bgTs_;
  int64_t edTs_;
  size_t queryFieldCnt_;
  std::vector<FieldQueryMapping> queryFieldVec_;
  std::vector<FieldQueryMapping> preWhereFieldVec_;
};

