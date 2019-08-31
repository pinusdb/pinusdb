#pragma once

#include "pdb.h"
#include "pdb_error.h"
#include <stdint.h>

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_connect(
  const char* pHostName,
  int32_t port,
  int32_t* pHandle
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_disconnect(
  int32_t handle
);


PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_login(
  int32_t handle,
  const char* pUser,
  const char* pPassword
);

PDBAPI
const char*
PDBAPI_CALLRULE
pdb_get_error_msg(
  int32_t errorCode
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_execute_insert(
  int32_t handle,
  const char* pSql,
  size_t* pSucessCnt,
  size_t* pPosition
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_execute_query(
  int32_t handle, 
  const char* pSql, 
  void** ppTable
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_execute_non_query(
  int32_t handle, 
  const char* pSql
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_free(
  void* pTable
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_column_count(
  void* pTable, 
  size_t* pCnt
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_column_info(
  void* pTable,
  size_t columnIdx,
  ColumnInfo* pColInfo
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_row_count(
  void* pTable,
  size_t* pCnt
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_val_is_null_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  bool* pIsNull
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_val_is_null_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  bool* pIsNull
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_bool_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  bool* pVal
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_bool_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  bool* pVal
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_bigint_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  int64_t* pVal
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_bigint_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  int64_t* pVal
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_double_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  double* pVal
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_double_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  double* pVal
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_string_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  const char** ppStrVal,
  size_t* pStrLen
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_string_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  const char** ppStrVal,
  size_t* pStrLen
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_blob_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  const uint8_t** ppBlobVal,
  size_t* pBlobLen
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_blob_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  const uint8_t** ppBlobVal,
  size_t* pBlobLen
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_datetime_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  int64_t* pDateTimeVal
);

PDBAPI
PdbErr_t
PDBAPI_CALLRULE
pdb_table_get_datetime_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  int64_t* pDateTimeVal
);
