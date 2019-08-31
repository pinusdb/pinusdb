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