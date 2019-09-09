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

#include "pdb.h"
#include "storage/page_hdr.h"
#include <stdint.h>

class PDBTable;
class DataPart;

class PageRef
{
public:
  PageRef();
  ~PageRef();

  void Attach(PageHdr* pPage);
  PageHdr* GetPageHdr() { return pPageHdr_; }

private:
  PageHdr* pPageHdr_;
};

class TableRef
{
public:
  TableRef();
  ~TableRef();

  void Attach(PDBTable* pTab);

private:
  PDBTable* pTab_;
};

class DataPartRef
{
public:
  DataPartRef();
  ~DataPartRef();

  void Attach(DataPart* pPart);
  DataPart* GetDataPart() { return pPart_; }

private:
  DataPart* pPart_;
};