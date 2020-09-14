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

#include <stdint.h>
#include "internal.h"

#define BIT_MAP_SET(p, i)   (((uint8_t*)(p))[(i) / 8] |= (1 << (7 - ((i) & 7))))
#define BIT_MAP_IS_SET(p, i)    (((uint8_t*)(p))[(i) / 8] & (1 << (7 - ((i) & 7))))

typedef struct _DBVal
{
  int32_t dataType_;
  int32_t dataLen_;
  union val{
    bool    boolVal_;
    int8_t  int8Val_;
    int16_t int16Val_;
    int32_t int32Val_;
    int64_t int64Val_;
    float  floatVal_;
    double doubleVal_;
    const uint8_t* pData_;
    uint8_t bytes_[8];
  }val_;
}DBVal;

#define DBVAL_REAL2_MULTIPLE   100
#define DBVAL_REAL3_MULTIPLE   1000
#define DBVAL_REAL4_MULTIPLE   10000
#define DBVAL_REAL6_MULTIPLE   1000000

#define DBVAL_SET_NULL(pVal)        \
do {  \
  (pVal)->dataType_ = PDB_VALUE_TYPE::VAL_NULL; \
  (pVal)->dataLen_ = 0; \
  (pVal)->val_.int64Val_ = 0; \
} while(false)

#define DBVAL_SET_BOOL(pVal, val)   \
do { \
  (pVal)->dataType_ = PDB_VALUE_TYPE::VAL_BOOL; \
  (pVal)->dataLen_ = 1; \
  (pVal)->val_.boolVal_ = (val); \
} while(false)

#define DBVAL_SET_INT8(pVal, val)    \
do { \
  (pVal)->dataType_ = PDB_VALUE_TYPE::VAL_INT8; \
  (pVal)->dataLen_ = 1; \
  (pVal)->val_.int8Val_ = (val); \
} while(false)

#define DBVAL_SET_INT16(pVal, val)    \
do { \
  (pVal)->dataType_ = PDB_VALUE_TYPE::VAL_INT16; \
  (pVal)->dataLen_ = 2;  \
  (pVal)->val_.int16Val_ = (val); \
} while(false)

#define DBVAL_SET_INT32(pVal, val)     \
do { \
  (pVal)->dataType_ = PDB_VALUE_TYPE::VAL_INT32; \
  (pVal)->dataLen_ = 4; \
  (pVal)->val_.int32Val_ = (val); \
} while(false)

#define DBVAL_SET_INT64(pVal, val)     \
do { \
  (pVal)->dataType_ = PDB_VALUE_TYPE::VAL_INT64; \
  (pVal)->dataLen_ = 8; \
  (pVal)->val_.int64Val_ = (val); \
} while(false)

#define DBVAL_SET_DATETIME(pVal, val)   \
do { \
  (pVal)->dataType_ = PDB_VALUE_TYPE::VAL_DATETIME;  \
  (pVal)->dataLen_ = 8;  \
  (pVal)->val_.int64Val_ = (val);  \
} while(false)

#define DBVAL_SET_FLOAT(pVal, val)  \
do { \
  (pVal)->dataType_ = PDB_VALUE_TYPE::VAL_FLOAT; \
  (pVal)->dataLen_ = 4; \
  (pVal)->val_.floatVal_ = (val); \
} while(false)

#define DBVAL_SET_DOUBLE(pVal, val)  \
do { \
  (pVal)->dataType_ = PDB_VALUE_TYPE::VAL_DOUBLE; \
  (pVal)->dataLen_ = 8; \
  (pVal)->val_.doubleVal_ = (val); \
} while(false)

#define DBVAL_SET_STRING(pVal, pStr, len) \
do { \
  (pVal)->dataType_ = PDB_VALUE_TYPE::VAL_STRING; \
  (pVal)->dataLen_ = (int32_t)len; \
  (pVal)->val_.pData_ = (const uint8_t*)pStr; \
} while(false)

#define DBVAL_SET_BLOB(pVal, pBlob, len) \
do { \
  (pVal)->dataType_ = PDB_VALUE_TYPE::VAL_BLOB; \
  (pVal)->dataLen_ = (int32_t)(len); \
  (pVal)->val_.pData_ = (const uint8_t*)(pBlob); \
} while(false)

#define DBVAL_SET_BLOCK_VALUE(pVal, valType, pBlock, len) \
do { \
  (pVal)->dataType_ = valType;  \
  (pVal)->dataLen_ = (int32_t)(len);  \
  (pVal)->val_.pData_ = (const uint8_t*)(pBlock);  \
} while(false)

///////////////////////////////////////////////////////////

#define DBVAL_ELE_SET_NULL(pVal, idx)  \
do {  \
  (pVal)[idx].dataType_ = PDB_VALUE_TYPE::VAL_NULL; \
  (pVal)[idx].dataLen_ = 0; \
  (pVal)[idx].val_.int64Val_ = 0; \
} while (false)

#define DBVAL_ELE_SET_BOOL(pVal, idx, val) \
do {  \
  (pVal)[idx].dataType_ = PDB_VALUE_TYPE::VAL_BOOL; \
  (pVal)[idx].dataLen_ = 1; \
  (pVal)[idx].val_.boolVal_ = (val); \
} while (false)

#define DBVAL_ELE_SET_INT8(pVal, idx, val)      \
do { \
  (pVal)[idx].dataType_ = PDB_VALUE_TYPE::VAL_INT8; \
  (pVal)[idx].dataLen_ = 1; \
  (pVal)[idx].val_.int8Val_ = (val); \
} while(false)

#define DBVAL_ELE_SET_INT16(pVal, idx, val)     \
do { \
  (pVal)[idx].dataType_ = PDB_VALUE_TYPE::VAL_INT16; \
  (pVal)[idx].dataLen_ = 2;  \
  (pVal)[idx].val_.int16Val_ = (val); \
} while(false)

#define DBVAL_ELE_SET_INT32(pVal, idx, val)     \
do { \
  (pVal)[idx].dataType_ = PDB_VALUE_TYPE::VAL_INT32; \
  (pVal)[idx].dataLen_ = 4; \
  (pVal)[idx].val_.int32Val_ = (val); \
} while(false)

#define DBVAL_ELE_SET_INT64(pVal, idx, val)     \
do { \
  (pVal)[idx].dataType_ = PDB_VALUE_TYPE::VAL_INT64; \
  (pVal)[idx].dataLen_ = 8; \
  (pVal)[idx].val_.int64Val_ = (val); \
} while(false)

#define DBVAL_ELE_SET_DATETIME(pVal, idx, val)  \
do { \
  (pVal)[idx].dataType_ = PDB_VALUE_TYPE::VAL_DATETIME;  \
  (pVal)[idx].dataLen_ = 8;  \
  (pVal)[idx].val_.int64Val_ = (val);  \
} while(false)

#define DBVAL_ELE_SET_FLOAT(pVal, idx, val)     \
do { \
  (pVal)[idx].dataType_ = PDB_VALUE_TYPE::VAL_FLOAT; \
  (pVal)[idx].dataLen_ = 4; \
  (pVal)[idx].val_.floatVal_ = (val); \
} while(false)

#define DBVAL_ELE_SET_DOUBLE(pVal, idx, val)    \
do { \
  (pVal)[idx].dataType_ = PDB_VALUE_TYPE::VAL_DOUBLE; \
  (pVal)[idx].dataLen_ = 8; \
  (pVal)[idx].val_.doubleVal_ = (val); \
} while(false)


#define DBVAL_ELE_SET_STRING(pVal, idx, pStr, len)  \
do { \
  (pVal)[idx].dataType_ = PDB_VALUE_TYPE::VAL_STRING; \
  (pVal)[idx].dataLen_ = (int32_t)len; \
  (pVal)[idx].val_.pData_ = (const uint8_t*)pStr; \
} while(false)


#define DBVAL_ELE_SET_BLOB(pVal, idx, pBlob, len)   \
do { \
  (pVal)[idx].dataType_ = PDB_VALUE_TYPE::VAL_BLOB; \
  (pVal)[idx].dataLen_ = (int32_t)(len); \
  (pVal)[idx].val_.pData_ = (const uint8_t*)(pBlob); \
} while(false)


///////////////////////////////////////////////////////////

#define DBVAL_IS_NULL(pVal)       (PDB_VALUE_TYPE::VAL_NULL == (pVal)->dataType_)
#define DBVAL_IS_BOOL(pVal)       (PDB_VALUE_TYPE::VAL_BOOL == (pVal)->dataType_)
#define DBVAL_IS_INT8(pVal)       (PDB_VALUE_TYPE::VAL_INT8 == (pVal)->dataType_)
#define DBVAL_IS_INT16(pVal)      (PDB_VALUE_TYPE::VAL_INT16 == (pVal)->dataType_)
#define DBVAL_IS_INT32(pVal)      (PDB_VALUE_TYPE::VAL_INT32 == (pVal)->dataType_)
#define DBVAL_IS_INT64(pVal)      (PDB_VALUE_TYPE::VAL_INT64 == (pVal)->dataType_)
#define DBVAL_IS_DATETIME(pVal)   (PDB_VALUE_TYPE::VAL_DATETIME == (pVal)->dataType_)
#define DBVAL_IS_FLOAT(pVal)      (PDB_VALUE_TYPE::VAL_FLOAT == (pVal)->dataType_)
#define DBVAL_IS_DOUBLE(pVal)     (PDB_VALUE_TYPE::VAL_DOUBLE == (pVal)->dataType_)
#define DBVAL_IS_STRING(pVal)     (PDB_VALUE_TYPE::VAL_STRING == (pVal)->dataType_)
#define DBVAL_IS_BLOB(pVal)       (PDB_VALUE_TYPE::VAL_BLOB == (pVal)->dataType_)

#define DBVAL_ELE_IS_NULL(pVal, idx)       (PDB_VALUE_TYPE::VAL_NULL == (pVal)[idx].dataType_)
#define DBVAL_ELE_IS_BOOL(pVal, idx)       (PDB_VALUE_TYPE::VAL_BOOL == (pVal)[idx].dataType_)
#define DBVAL_ELE_IS_INT8(pVal, idx)       (PDB_VALUE_TYPE::VAL_INT8 == (pVal)[idx].dataType_)
#define DBVAL_ELE_IS_INT16(pVal, idx)      (PDB_VALUE_TYPE::VAL_INT16 == (pVal)[idx].dataType_)
#define DBVAL_ELE_IS_INT32(pVal, idx)      (PDB_VALUE_TYPE::VAL_INT32 == (pVal)[idx].dataType_)
#define DBVAL_ELE_IS_INT64(pVal, idx)      (PDB_VALUE_TYPE::VAL_INT64 == (pVal)[idx].dataType_)
#define DBVAL_ELE_IS_DATETIME(pVal, idx)   (PDB_VALUE_TYPE::VAL_DATETIME == (pVal)[idx].dataType_)
#define DBVAL_ELE_IS_FLOAT(pVal, idx)      (PDB_VALUE_TYPE::VAL_FLOAT == (pVal)[idx].dataType_)
#define DBVAL_ELE_IS_DOUBLE(pVal, idx)     (PDB_VALUE_TYPE::VAL_DOUBLE == (pVal)[idx].dataType_)
#define DBVAL_ELE_IS_STRING(pVal, idx)     (PDB_VALUE_TYPE::VAL_STRING == (pVal)[idx].dataType_)
#define DBVAL_ELE_IS_BLOB(pVal, idx)       (PDB_VALUE_TYPE::VAL_BLOB == (pVal)[idx].dataType_)

#define DBVAL_ELE_IS_TYPE(pVal, idx, type) ((type) == (pVal)[idx].dataType_)

  //////////////////////////////////////////////////////////

#define DBVAL_GET_TYPE(pVal)         ((pVal)->dataType_)
#define DBVAL_GET_BOOL(pVal)         ((pVal)->val_.boolVal_)
#define DBVAL_GET_INT8(pVal)         ((pVal)->val_.int8Val_)
#define DBVAL_GET_INT16(pVal)        ((pVal)->val_.int16Val_)
#define DBVAL_GET_INT32(pVal)        ((pVal)->val_.int32Val_)
#define DBVAL_GET_INT64(pVal)        ((pVal)->val_.int64Val_)
#define DBVAL_GET_DATETIME(pVal)     ((pVal)->val_.int64Val_)
#define DBVAL_GET_FLOAT(pVal)        ((pVal)->val_.floatVal_)
#define DBVAL_GET_DOUBLE(pVal)       ((pVal)->val_.doubleVal_)
#define DBVAL_GET_STRING(pVal)       ((const char*)(pVal)->val_.pData_)
#define DBVAL_GET_BLOB(pVal)         ((pVal)->val_.pData_)
#define DBVAL_GET_LEN(pVal)          ((pVal)->dataLen_)
#define DBVAL_GET_BYTES(pVal)        ((pVal)->val_.bytes_)

#define DBVAL_ELE_GET_TYPE(pVal, idx)       ((pVal)[idx].dataType_)
#define DBVAL_ELE_GET_BOOL(pVal, idx)       ((pVal)[idx].val_.boolVal_)
#define DBVAL_ELE_GET_INT8(pVal, idx)       ((pVal)[idx].val_.int8Val_)
#define DBVAL_ELE_GET_INT16(pVal, idx)      ((pVal)[idx].val_.int16Val_)
#define DBVAL_ELE_GET_INT32(pVal, idx)      ((pVal)[idx].val_.int32Val_)
#define DBVAL_ELE_GET_INT64(pVal, idx)      ((pVal)[idx].val_.int64Val_)
#define DBVAL_ELE_GET_DATETIME(pVal, idx)   ((pVal)[idx].val_.int64Val_)
#define DBVAL_ELE_GET_FLOAT(pVal, idx)      ((pVal)[idx].val_.floatVal_)
#define DBVAL_ELE_GET_DOUBLE(pVal, idx)     ((pVal)[idx].val_.doubleVal_)
#define DBVAL_ELE_GET_UINT64(pVal, idx)     ((pVal)[idx].val_.u64Val_)
#define DBVAL_ELE_GET_STRING(pVal, idx)     ((const char*)((pVal)[idx].val_.pData_))
#define DBVAL_ELE_GET_BLOB(pVal, idx)       ((pVal)[idx].val_.pData_)
#define DBVAL_ELE_GET_LEN(pVal, idx)        ((pVal)[idx].dataLen_)

#define DBVAL_ELE_GET_BYTES(pVal, idx)        ((pVal)[idx].val_.bytes_)