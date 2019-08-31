#include "pdb_datatable.h"
#include "pdb_error.h"

PDBDataTable::PDBDataTable()
{

}
PDBDataTable::~PDBDataTable()
{
  for (auto dataIt = dataVec_.begin(); dataIt != dataVec_.end(); dataIt++)
  {
    delete *dataIt;
  }
}

PdbErr_t PDBDataTable::AddColumn(const char* pColName, size_t colNameLen, uint32_t type)
{
  if (pColName == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  if (!PDB_TYPE_IS_VALID(type))
  {
    return PdbE_INVALID_PARAM;
  }

  if (dataVec_.size() != 0)
  {
    return PdbE_INVALID_PARAM;
  }

  std::string colName(pColName, colNameLen);
  PdbErr_t retVal = tabInfo_.AddField(colName.c_str(), type);
  if (retVal != PdbE_OK)
    return retVal;

  return PdbE_OK;
}

size_t PDBDataTable::GetColumnCount() const
{
  return tabInfo_.GetFieldCnt();
}
PdbErr_t PDBDataTable::GetColumnInfo(size_t idx, ColumnInfo* pColInfo) const
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  const char * pFieldName = nullptr;

  retVal = tabInfo_.GetFieldInfo(idx, &fieldType);
  if (retVal != PdbE_OK)
    return retVal;

  pFieldName = tabInfo_.GetFieldName(idx);

  if (pColInfo != nullptr)
  {
    strcpy_s(pColInfo->colName_, pFieldName);
    pColInfo->colType_ = fieldType;
  }

  return retVal;
}

PdbErr_t PDBDataTable::GetColumnPos(const char* pName, size_t* pPos) const
{
  if (pName == nullptr || pPos == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  return tabInfo_.GetFieldInfo(pName, pPos, nullptr);
}

size_t PDBDataTable::GetRows() const
{
  return dataVec_.size();
}
PdbErr_t PDBDataTable::AddRow(DBObj* pObj)
{
  if (pObj == nullptr)
    return PdbE_INVALID_PARAM;

  size_t fieldCnt = tabInfo_.GetFieldCnt();

  if (pObj->GetFieldCnt() != fieldCnt)
    return PdbE_INVALID_PARAM;

  const DBVal* pVal = nullptr;

  DBObj* pNewObj = new DBObj(&arena_);
  if (pNewObj == nullptr)
    return PdbE_NOMEM;

  char* pTmpBuf = nullptr;
  
  for (int i = 0; i < fieldCnt; i++)
  {
    pVal = pObj->GetFieldValue(i);

    int32_t fieldType = 0;
    tabInfo_.GetFieldInfo(i, &fieldType);

    if ((!DBVAL_IS_NULL(pVal)) && DBVAL_GET_TYPE(pVal) != fieldType)
    {
      delete pNewObj;
      return PdbE_INVALID_PARAM;
    }

    if (DBVAL_IS_NULL(pVal))
    {
      pNewObj->AppendNullVal();
    }
    else
    {
      PdbErr_t retVal = pNewObj->AppendVal(pVal);
      if (retVal != PdbE_OK)
        return retVal;
    }
  }

  dataVec_.push_back(pNewObj);

  return PdbE_OK;
}

const DBObj* PDBDataTable::GetData(size_t idx)
{
  if (idx < 0 || idx >= dataVec_.size())
  {
    return nullptr;
  }

  return dataVec_[idx];
}

