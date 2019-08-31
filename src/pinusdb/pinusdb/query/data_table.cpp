#include "query/data_table.h"
#include "pdb_error.h"
#include "util/string_tool.h"
#include "table/db_obj.h"

DataTable::DataTable()
{

}
DataTable::~DataTable()
{
  ClearData();
}

PdbErr_t DataTable::AddColumn(const std::string& colName, int32_t type)
{
  //已经有数据了，不能再加列
  if (dataVec_.size() > 0)
    return PdbE_INVALID_PARAM;

  return tabInfo_.AddField(colName.c_str(), type);
}

size_t DataTable::GetColumnCnt() const
{
  return tabInfo_.GetFieldCnt();
}

PdbErr_t DataTable::GetColumnInfo(size_t colPos, std::string& colName, int32_t* pColType) const
{
  PdbErr_t retVal = tabInfo_.GetFieldInfo(colPos, pColType);
  if (retVal != PdbE_OK)
    return retVal;

  colName = tabInfo_.GetFieldName(colPos);
  return PdbE_OK;
}

PdbErr_t DataTable::ClearData()
{
  for (auto dataIt = dataVec_.begin(); dataIt != dataVec_.end(); dataIt++)
  {
    delete *dataIt;
  }
  dataVec_.clear();

  return PdbE_OK;
}

PdbErr_t DataTable::AppendData(DBObj* pObj)
{
  if (pObj == nullptr)
    return PdbE_INVALID_PARAM;

  size_t fieldCnt = tabInfo_.GetFieldCnt();

  if (pObj->GetFieldCnt() != fieldCnt)
    return PdbE_VALUE_MISMATCH;

  int colType = 0;
  const DBVal* pVal = nullptr;

  for (int i = 0; i < fieldCnt; i++)
  {
    pVal = pObj->GetFieldValue(i);
    if (DBVAL_IS_NULL(pVal))
      continue;

    tabInfo_.GetFieldInfo(i, &colType);

    if (DBVAL_GET_TYPE(pVal) != colType)
      return PdbE_VALUE_MISMATCH;
  }

  dataVec_.push_back(pObj);
  return PdbE_OK;
}

Arena* DataTable::GetArena()
{
  return &arena_;
}

size_t DataTable::GetRecordCnt() const
{
  return dataVec_.size();
}

PdbErr_t DataTable::GetSerializeLen(size_t* pDataLen)
{
  if (pDataLen == nullptr)
    return PdbE_INVALID_PARAM;

  PdbErr_t retVal = PdbE_OK;

  size_t recLen = 0;
  size_t totalLen = 0;

  retVal = HeadSerialize(nullptr, &recLen);
  if (retVal != PdbE_OK)
    return retVal;

  totalLen += recLen;

  for (auto dataIt = dataVec_.begin(); dataIt != dataVec_.end(); dataIt++)
  {
    retVal = (*dataIt)->GetTransLen(&recLen);
    if (retVal != PdbE_OK)
      break;

    totalLen += recLen;
  }

  if (retVal == PdbE_OK)
  {
    *pDataLen = totalLen;
  }

  return retVal;
}
PdbErr_t DataTable::Serialize(uint8_t* pData, size_t* pBodyLen)
{
  if (pData == nullptr)
    return PdbE_INVALID_PARAM;

  PdbErr_t retVal = PdbE_OK;

  size_t recLen = 0;
  size_t totalLen = 0;

  retVal = HeadSerialize(pData, &recLen);
  if (retVal != PdbE_OK)
    return retVal;

  totalLen += recLen;

  for (auto dataIt = dataVec_.begin(); dataIt != dataVec_.end(); dataIt++)
  {
    retVal = (*dataIt)->SerializeToTrans((pData + totalLen), &recLen);
    if (retVal != PdbE_OK)
      break;

    totalLen += recLen;
  }

  if (pBodyLen != nullptr)
    *pBodyLen = totalLen;

  return retVal;
}

PdbErr_t DataTable::HeadSerialize(uint8_t* pData, size_t* pDataLen)
{
  size_t serializeLen = 0;
  size_t fieldCnt = tabInfo_.GetFieldCnt();
  int32_t fieldType = 0;

  Arena arena;
  DBObj headObj(&arena);

  for (size_t i = 0; i < fieldCnt; i++)
  {
    std::string transName = "";

    tabInfo_.GetFieldInfo(i, &fieldType);
    const char* pFieldName = tabInfo_.GetFieldName(i);

    switch (fieldType)
    {
    case PDB_FIELD_TYPE::TYPE_BOOL:
      transName = "bool;";
      break;
    case PDB_FIELD_TYPE::TYPE_INT64:
      transName = "bigint;";
      break;
    case PDB_FIELD_TYPE::TYPE_DATETIME:
      transName = "datetime;";
      break;
    case PDB_FIELD_TYPE::TYPE_DOUBLE:
      transName = "double;";
      break;
    case PDB_FIELD_TYPE::TYPE_STRING:
      transName = "string;";
      break;
    case PDB_FIELD_TYPE::TYPE_BLOB:
      transName = "blob;";
      break;
    default:
      return PdbE_RECORD_FAIL;
    }

    transName += pFieldName;

    headObj.AppendStrVal(transName.c_str(), transName.size());
  }

  if (pData != nullptr)
    return headObj.SerializeToTrans(pData, pDataLen);
  else
    return headObj.GetTransLen(pDataLen);
}
