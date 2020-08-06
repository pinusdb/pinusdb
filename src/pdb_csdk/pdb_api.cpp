#include "pdb_api.h"
#include "pdb_client.h"
#include "pdb_error_msg.h"
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include "db_obj.h"

std::mutex handleMutex_;
int32_t maxHandle_ = 1;
std::unordered_map<int32_t, DBClient*> handleMap_;

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_connect(
  const char* pHostName,
  int32_t port,
  int32_t* pHandle
)
{
  PdbErr_t retVal = PdbE_OK;

  if (pHostName == nullptr || pHandle == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  DBClient* pClient = new DBClient();
  if (pClient == nullptr)
  {
    return PdbE_NOMEM;
  }

  retVal = pClient->Connect(pHostName, port);
  if (retVal != PdbE_OK)
  {
    delete pClient;
    return retVal;
  }

  std::unique_lock<std::mutex> handleLock(handleMutex_);
  *pHandle = maxHandle_++;
  std::pair<int32_t, DBClient*> handlePair((*pHandle), pClient);
  handleMap_.insert(handlePair);

  return PdbE_OK;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_disconnect(
  int32_t handle
)
{
  std::unique_lock<std::mutex> handleLock(handleMutex_);
  auto handleIt = handleMap_.find(handle);
  if (handleIt != handleMap_.end())
  {
    handleIt->second->Disconnect();
    delete handleIt->second;
    handleMap_.erase(handleIt);

    return PdbE_OK;
  }

  return PdbE_INVALID_HANDLE;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_login(
  int32_t handle,
  const char* pUser,
  const char* pPassword
)
{
  if (pUser == nullptr || pPassword == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  DBClient* pClient = nullptr;

  {
    std::unique_lock<std::mutex> handleLock(handleMutex_);
    auto handleIter = handleMap_.find(handle);
    if (handleIter != handleMap_.end())
    {
      pClient = handleIter->second;
    }
  }

  if (pClient == nullptr)
  {
    return PdbE_INVALID_HANDLE;
  }

  PdbErr_t retVal = PdbE_OK;

  retVal = pClient->Login(pUser, pPassword);

  return retVal;
}

PDBAPI
const char*
PDBAPI_CALLRULE
pdb_get_error_msg(
  int32_t errorCode
)
{
  return PdbErrorMsg::GetInstance()->GetMsgInfo(errorCode);
}


PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_execute_insert(
  int32_t handle,
  const char* pSql,
  size_t* pSucessCnt,
  size_t* pPosition
)
{
  if (pSql == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  DBClient* pClient = nullptr;

  {
    std::unique_lock<std::mutex> handleLock(handleMutex_);
    auto handleIter = handleMap_.find(handle);
    if (handleIter != handleMap_.end())
    {
      pClient = handleIter->second;
    }
  }

  if (pClient == nullptr)
  {
    return PdbE_INVALID_HANDLE;
  }

  return pClient->ExecuteInsert(pSql, pSucessCnt, pPosition);
}


PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_execute_query(
  int32_t handle,
  const char* pSql,
  void** ppTable
)
{
  if (pSql == nullptr || ppTable == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  DBClient* pClient = nullptr;

  {
    std::unique_lock<std::mutex> handleLock(handleMutex_);
    auto handleIter = handleMap_.find(handle);
    if (handleIter != handleMap_.end())
    {
      pClient = handleIter->second;
    }
  }

  if (pClient == nullptr)
  {
    return PdbE_INVALID_HANDLE;
  }

  PdbErr_t retVal = PdbE_OK;

  PDBDataTable* pTmpTable = new PDBDataTable();
  if (pTmpTable == nullptr)
  {
    return PdbE_NOMEM;
  }

  retVal = pClient->ExecuteQuery(pSql, pTmpTable);
  if (retVal != PdbE_OK)
  {
    delete pTmpTable;
    return retVal;
  }

  *ppTable = pTmpTable;

  return PdbE_OK;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_execute_non_query(
  int32_t handle,
  const char* pSql
)
{
  if (pSql == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  DBClient* pClient = nullptr;

  {
    std::unique_lock<std::mutex> handleLock(handleMutex_);
    auto handleIter = handleMap_.find(handle);
    if (handleIter != handleMap_.end())
    {
      pClient = handleIter->second;
    }
  }

  if (pClient == nullptr)
  {
    return PdbE_INVALID_HANDLE;
  }

  return pClient->ExecuteNonQuery(pSql);
}


PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_free(
  void* pTable
)
{
  if (pTable == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  delete ((PDBDataTable*)pTable);

  return PdbE_OK;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_column_count(
  void* pTable,
  size_t* pCnt
)
{
  if (pTable == nullptr || pCnt == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  *pCnt = ((PDBDataTable*)pTable)->GetColumnCount();

  return PdbE_OK;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_column_info(
  void* pTable,
  size_t columnIdx,
  ColumnInfo* pColInfo
)
{
  if (pTable == nullptr || columnIdx < 0 || pColInfo == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  return ((PDBDataTable*)pTable)->GetColumnInfo(columnIdx, pColInfo);
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_row_count(
  void* pTable,
  size_t* pCnt
)
{
  if (pTable == nullptr || pCnt == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  *pCnt = ((PDBDataTable*)pTable)->GetRows();
  return PdbE_OK;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_val_is_null_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  bool* pIsNull
  )
{
  if (pTable == nullptr || pIsNull == nullptr)
    return PdbE_INVALID_PARAM;

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  const DBObj* pDataObj = pTmp->GetData(rowIdx);
  if (pDataObj == nullptr)
    return PdbE_INVALID_PARAM;

  const DBVal* pDataVal = pDataObj->GetFieldValue(colIdx);
  if (pDataVal == nullptr)
    return PdbE_RECORD_FAIL;

  *pIsNull = DBVAL_IS_NULL(pDataVal);

  return PdbE_OK;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_val_is_null_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  bool* pIsNull
  )
{
  if (pTable == nullptr || pColumnName == nullptr || pIsNull == nullptr)
    return PdbE_INVALID_PARAM;

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  PdbErr_t retVal = PdbE_OK;

  size_t colIdx = 0;
  retVal = pTmp->GetColumnPos(pColumnName, &colIdx);
  if (retVal != PdbE_OK)
    return retVal;

  return pdb_table_val_is_null_by_colidx(pTable, rowIdx, colIdx, pIsNull);
}


PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_bool_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  bool* pVal
)
{
  if (pTable == nullptr || pVal == nullptr)
    return PdbE_INVALID_PARAM;

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  const DBObj* pDataObj = pTmp->GetData(rowIdx);
  if (pDataObj == nullptr)
    return PdbE_INVALID_PARAM;

  const DBVal* pDataVal = pDataObj->GetFieldValue(colIdx);
  if (pDataVal == nullptr)
    return PdbE_RECORD_FAIL;

  if (DBVAL_IS_NULL(pDataVal))
    return PdbE_NULL_VALUE;

  if (DBVAL_IS_BOOL(pDataVal))
  {
    *pVal = DBVAL_GET_BOOL(pDataVal);
    return PdbE_OK;
  }

  return PdbE_VALUE_MISMATCH;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_bool_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  bool* pVal
)
{
  if (pTable == nullptr || pColumnName == nullptr || pVal == nullptr)
    return PdbE_INVALID_PARAM;

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  PdbErr_t retVal = PdbE_OK;

  size_t colIdx = 0;
  retVal = pTmp->GetColumnPos(pColumnName, &colIdx);
  if (retVal != PdbE_OK)
    return retVal;

  return pdb_table_get_bool_by_colidx(pTable, rowIdx, colIdx, pVal);
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_bigint_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  int64_t* pVal
)
{
  if (pTable == nullptr || pVal == nullptr)
    return PdbE_INVALID_PARAM;

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  const DBObj* pDataObj = pTmp->GetData(rowIdx);
  if (pDataObj == nullptr)
    return PdbE_INVALID_PARAM;

  const DBVal* pDataVal = pDataObj->GetFieldValue(colIdx);
  if (pDataVal == nullptr)
    return PdbE_RECORD_FAIL;

  if (DBVAL_IS_NULL(pDataVal))
  {
    return PdbE_NULL_VALUE;
  }

  if (DBVAL_IS_INT64(pDataVal))
  {
    *pVal = DBVAL_GET_INT64(pDataVal);
    return PdbE_OK;
  }

  return PdbE_VALUE_MISMATCH;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_bigint_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  int64_t* pVal
)
{
  if (pTable == nullptr || pColumnName == nullptr || pVal == nullptr)
    return PdbE_INVALID_PARAM;

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  PdbErr_t retVal = PdbE_OK;

  size_t colIdx = 0;
  retVal = pTmp->GetColumnPos(pColumnName, &colIdx);
  if (retVal != PdbE_OK)
    return retVal;

  return pdb_table_get_bigint_by_colidx(pTable, rowIdx, colIdx, pVal);
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_double_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  double* pVal
)
{
  if (pTable == nullptr || pVal == nullptr)
    return PdbE_INVALID_PARAM;

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  const DBObj* pDataObj = pTmp->GetData(rowIdx);
  if (pDataObj == nullptr)
    return PdbE_INVALID_PARAM;

  const DBVal* pDataVal = pDataObj->GetFieldValue(colIdx);
  if (pDataVal == nullptr)
    return PdbE_RECORD_FAIL;

  if (DBVAL_IS_NULL(pDataVal))
  {
    return PdbE_NULL_VALUE;
  }

  if (DBVAL_IS_DOUBLE(pDataVal))
  {
    *pVal = DBVAL_GET_DOUBLE(pDataVal);
    return PdbE_OK;
  }

  return PdbE_VALUE_MISMATCH;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_double_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  double* pVal
)
{
  if (pTable == nullptr || pColumnName == nullptr || pVal == nullptr)
    return PdbE_INVALID_PARAM;

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  PdbErr_t retVal = PdbE_OK;

  size_t colIdx = 0;
  retVal = pTmp->GetColumnPos(pColumnName, &colIdx);
  if (retVal != PdbE_OK)
    return retVal;

  return pdb_table_get_double_by_colidx(pTable, rowIdx, colIdx, pVal);
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_string_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  const char** ppStrVal,
  size_t* pStrLen
)
{
  if (pTable == nullptr || ppStrVal == nullptr || pStrLen == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  const DBObj* pDataObj = pTmp->GetData(rowIdx);
  if (pDataObj == nullptr)
    return PdbE_INVALID_PARAM;

  const DBVal* pDataVal = pDataObj->GetFieldValue(colIdx);
  if (pDataVal == nullptr)
    return PdbE_RECORD_FAIL;

  if (DBVAL_IS_NULL(pDataVal))
  {
    return PdbE_NULL_VALUE;
  }

  if (DBVAL_IS_STRING(pDataVal))
  {
    *ppStrVal = DBVAL_GET_STRING(pDataVal);
    *pStrLen = DBVAL_GET_LEN(pDataVal);
    return PdbE_OK;
  }

  return PdbE_VALUE_MISMATCH;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_string_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  const char** ppStrVal,
  size_t* pStrLen
)
{
  if (pTable == nullptr || pColumnName == nullptr 
    || ppStrVal == nullptr || pStrLen == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  PdbErr_t retVal = PdbE_OK;

  size_t colIdx = 0;
  retVal = pTmp->GetColumnPos(pColumnName, &colIdx);
  if (retVal != PdbE_OK)
    return retVal;

  return pdb_table_get_string_by_colidx(pTable, rowIdx, colIdx, ppStrVal, pStrLen);
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_blob_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  const uint8_t** ppBlobVal,
  size_t* pBlobLen
)
{
  if (pTable == nullptr  || ppBlobVal == nullptr || pBlobLen == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  const DBObj* pDataObj = pTmp->GetData(rowIdx);
  if (pDataObj == nullptr)
    return PdbE_INVALID_PARAM;

  const DBVal* pDataVal = pDataObj->GetFieldValue(colIdx);
  if (pDataVal == nullptr)
    return PdbE_RECORD_FAIL;

  if (DBVAL_IS_NULL(pDataVal))
  {
    return PdbE_NULL_VALUE;
  }

  if (DBVAL_IS_BLOB(pDataVal))
  {
    *ppBlobVal = DBVAL_GET_BLOB(pDataVal);
    *pBlobLen = DBVAL_GET_LEN(pDataVal);
    return PdbE_OK;
  }

  return PdbE_VALUE_MISMATCH;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_blob_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  const uint8_t** ppBlobVal,
  size_t* pBlobLen
)
{
  if (pTable == nullptr || pColumnName == nullptr 
    || ppBlobVal == nullptr || pBlobLen == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  PdbErr_t retVal = PdbE_OK;

  size_t colIdx = 0;
  retVal = pTmp->GetColumnPos(pColumnName, &colIdx);
  if (retVal != PdbE_OK)
    return retVal;

  return pdb_table_get_blob_by_colidx(pTable, rowIdx, colIdx, ppBlobVal, pBlobLen);
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_datetime_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  int64_t* pVal
  )
{
  if (pTable == nullptr || pVal == nullptr)
    return PdbE_INVALID_PARAM;

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  const DBObj* pDataObj = pTmp->GetData(rowIdx);
  if (pDataObj == nullptr)
    return PdbE_INVALID_PARAM;

  const DBVal* pDataVal = pDataObj->GetFieldValue(colIdx);
  if (pDataVal == nullptr)
    return PdbE_RECORD_FAIL;

  if (DBVAL_IS_NULL(pDataVal))
  {
    return PdbE_NULL_VALUE;
  }

  if (DBVAL_IS_DATETIME(pDataVal))
  {
    *pVal = DBVAL_GET_DATETIME(pDataVal);
    return PdbE_OK;
  }

  return PdbE_VALUE_MISMATCH;
}

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_datetime_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  int64_t* pVal
  )
{
  if (pTable == nullptr || pColumnName == nullptr || pVal == nullptr)
    return PdbE_INVALID_PARAM;

  PDBDataTable* pTmp = (PDBDataTable*)pTable;

  PdbErr_t retVal = PdbE_OK;
  
  size_t colIdx = 0;
  retVal = pTmp->GetColumnPos(pColumnName, &colIdx);
  if (retVal != PdbE_OK)
    return retVal;

  return pdb_table_get_datetime_by_colidx(pTable, rowIdx, colIdx, pVal);
}
