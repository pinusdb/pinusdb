/*
* Copyright (c) 2020 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
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

typedef int PdbErr_t;

#define PDB_TABLE_NAME_LEN    48  //表名最长长度
#define PDB_FILED_NAME_LEN    48  //字段名长度
#define PDB_USER_NAME_LEN     48  //用户名最长长度

#define PDB_DEVID_NAME_LEN       96  // 设备名长度
#define PDB_DEVID_EXPAND_LEN     128 // 扩展属性长度

#define PDB_MIN_REAL_VALUE       (-1000000000)
#define PDB_MAX_REAL_VALUE       (+1000000000)

enum PDB_VALUE_TYPE
{
  VAL_NULL            = 0,
  VAL_BOOL            = 1,     // bool
  VAL_INT8            = 2,     // 1字节整型
  VAL_INT16           = 3,     // 2字节整型
  VAL_INT32           = 4,     // 4字节整型
  VAL_INT64           = 5,     // 8字节整型
  VAL_DATETIME        = 6,     // 时间戳 8字节
  VAL_FLOAT           = 7,     // 4字节 单精度浮点型
  VAL_DOUBLE          = 8,     // 8字节 双精度浮点型
  VAL_STRING          = 9,     // 字符串
  VAL_BLOB            = 10,    // 二进制
};

enum PDB_FIELD_TYPE
{
  TYPE_BOOL           = 1,  // bool
  TYPE_INT8           = 2,  // 1字节整型
  TYPE_INT16          = 3,  // 2字节整型
  TYPE_INT32          = 4,  // 4字节整型
  TYPE_INT64          = 5,  // 8字节整型
  TYPE_DATETIME       = 6,  // 时间戳 8字节
  TYPE_FLOAT          = 7,  // 4字节 单精度浮点型
  TYPE_DOUBLE         = 8,  // 8字节 双精度浮点型
  TYPE_STRING         = 9,  // 字符串
  TYPE_BLOB           = 10, // 二进制

  TYPE_REAL2          = 32, // double, 取值范围 [-999,999,999.99      ~  +999,999,999.99]
  TYPE_REAL3          = 33, // double, 取值范围 [-999,999,999.999     ~  +999,999,999.999]
  TYPE_REAL4          = 34, // double, 取值范围 [-999,999,999.9999    ~  +999,999,999.9999]
  TYPE_REAL6          = 35  // double, 取值范围 [-999,999,999.999999  ~  +999,999,999.999999]
};

#define PDB_TYPE_IS_VALID(type)    (((type) >= PDB_FIELD_TYPE::TYPE_BOOL && (type) <= PDB_FIELD_TYPE::TYPE_BLOB) || ((type) >= PDB_FIELD_TYPE::TYPE_REAL2  && (type) <= PDB_FIELD_TYPE::TYPE_REAL6))
#define PDB_TYPE_IS_REAL(type)     ((type) >= PDB_FIELD_TYPE::TYPE_REAL2 && (type) <= PDB_FIELD_TYPE::TYPE_REAL6)
#define PDB_TYPE_IS_NUMBER(type)   ((type) == PDB_FIELD_TYPE::TYPE_INT8 || (type) == PDB_FIELD_TYPE::TYPE_INT16 || (type) == PDB_FIELD_TYPE::TYPE_INT32 || (type) == PDB_FIELD_TYPE::TYPE_INT64)
#define PDB_TYPE_IS_FLOAT_OR_DOUBLE(type) ((type) == PDB_FIELD_TYPE::TYPE_FLOAT || (type) == PDB_FIELD_TYPE::TYPE_DOUBLE)


enum PDB_ROLE
{
  ROLE_READ_ONLY = 1,    //只读
  ROLE_WRITE_ONLY = 2,   //只写
  ROLE_READ_WRITE = 3,   //读写
  ROLE_ADMIN = 4,        //管理员
};

typedef struct _ColumnInfo
{
  char colName_[PDB_FILED_NAME_LEN];
  int colType_;
}ColumnInfo;
