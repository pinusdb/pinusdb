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

#include "expr/column_item.h"

ColumnItem::ColumnItem(const Token* pToken, int32_t type)
{
  this->name_ = std::string(pToken->str_, pToken->len_);
  this->type_ = type;
}

ColumnItem::~ColumnItem()
{

}

const char* ColumnItem::GetName() const
{
  return this->name_.c_str();
}

int32_t ColumnItem::GetNameLen() const
{
  return (int32_t)this->name_.size();
}

int32_t ColumnItem::GetType() const
{
  return this->type_;
}

ColumnItem* ColumnItem::MakeColumnItem(const Token* pToken, int32_t type)
{
  if (pToken == nullptr)
    return nullptr;

  return new ColumnItem(pToken, type);
}

void ColumnItem::FreeColumnItem(ColumnItem* pColItem)
{
  delete pColItem;
}

ColumnList::ColumnList()
{

}
ColumnList::~ColumnList()
{
  for (auto colIt = colItemVec_.begin(); colIt != colItemVec_.end(); colIt++)
  {
    delete *colIt;
  }
  colItemVec_.clear();
}

ColumnList* ColumnList::AddColumnItem(ColumnItem* pColItem)
{
  this->colItemVec_.push_back(pColItem);
  return this;
}
const std::vector<ColumnItem*>& ColumnList::GetColumnList() const
{
  return this->colItemVec_;
}

ColumnList* ColumnList::AppendColumnItem(ColumnList* pColList, ColumnItem* pColItem)
{
  if (pColList == nullptr)
  {
    pColList = new ColumnList();
  }

  if (pColItem != nullptr)
    pColList->AddColumnItem(pColItem);

  return pColList;
}
void ColumnList::FreeColumnList(ColumnList* pColList)
{
  if (pColList != nullptr)
    delete pColList;
}

