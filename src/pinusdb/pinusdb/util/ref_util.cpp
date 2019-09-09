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

#include "util/ref_util.h"
#include "db/page_pool.h"
#include "table/pdb_table.h"
#include "storage/data_part.h"

PageRef::PageRef()
{
  this->pPageHdr_ = nullptr;
}
PageRef::~PageRef()
{
  if (this->pPageHdr_ != nullptr)
  {
    PAGEHDR_MINUS_REF(pPageHdr_);
  }
}

void PageRef::Attach(PageHdr* pPage)
{
  if (this->pPageHdr_ != nullptr)
  {
    PAGEHDR_MINUS_REF(pPageHdr_);
  }
  this->pPageHdr_ = pPage;
  if (this->pPageHdr_ != nullptr)
  {
    PAGEHDR_ADD_REF(pPageHdr_);
  }
}

//////////////////////////////////////////////////////////////

TableRef::TableRef()
{
  this->pTab_ = nullptr;
}

TableRef::~TableRef()
{
  if (this->pTab_ != nullptr)
    pTab_->MinusRef();
}

void TableRef::Attach(PDBTable* pTab)
{
  if (this->pTab_ != nullptr)
  {
    pTab_->MinusRef();
  }

  this->pTab_ = pTab;
  if (this->pTab_ != nullptr)
  {
    this->pTab_->AddRef();
  }
}

DataPartRef::DataPartRef()
{
  this->pPart_ = nullptr;
}

DataPartRef::~DataPartRef()
{
  if (this->pPart_ != nullptr)
    this->pPart_->MinusRef();
}

void DataPartRef::Attach(DataPart* pPart)
{
  if (this->pPart_ != nullptr)
    this->pPart_->MinusRef();

  this->pPart_ = pPart;
  if (this->pPart_ != nullptr)
    this->pPart_->AddRef();
}

