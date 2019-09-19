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

RefUtil::RefUtil()
{
  this->pObj_ = nullptr;
}

RefUtil::~RefUtil()
{
  if (this->pObj_ != nullptr)
    this->pObj_->MinusRef();
}

void RefUtil::Attach(RefObj* pObj)
{
  if (this->pObj_ != nullptr)
  {
    pObj_->MinusRef();
  }

  this->pObj_ = pObj;
  if (this->pObj_ != nullptr)
  {
    this->pObj_->AddRef();
  }
}
