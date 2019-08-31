#pragma once

typedef int PdbErr_t;

#ifdef _WIN32

#define PDBAPI extern "C" __declspec(dllexport)

#define PDBAPI_CALLRULE _stdcall

#endif

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
  VAL_INT64           = 2,     // 8字节 bigint
  VAL_DATETIME        = 3,    // 时间戳 8字节
  VAL_DOUBLE          = 4,     // 8字节 双精度浮点型
  VAL_STRING          = 5,     // 字符串
  VAL_BLOB            = 6,     // 二进制
};

enum PDB_FIELD_TYPE
{
  TYPE_BOOL             = 1,     // bool
  TYPE_INT64            = 2,     // 8字节
  TYPE_DATETIME         = 3,     // 8字节
  TYPE_DOUBLE           = 4,     // 8字节 双精度浮点型
  TYPE_STRING           = 5,     // 字符串
  TYPE_BLOB             = 6,     // 二进制

  TYPE_REAL2            = 32,     // double, 取值范围 [-999,999,999.99      ~  +999,999,999.99]
  TYPE_REAL3            = 33,     // double, 取值范围 [-999,999,999.999     ~  +999,999,999.999]
  TYPE_REAL4            = 34,    // double, 取值范围 [-999,999,999.9999    ~  +999,999,999.9999]
  TYPE_REAL6            = 35,    // double, 取值范围 [-999,999,999.999999  ~  +999,999,999.999999]
};

#define PDB_TYPE_IS_VALID(type)    (((type) >= PDB_FIELD_TYPE::TYPE_BOOL && (type) <= PDB_FIELD_TYPE::TYPE_BLOB) || ((type) >= PDB_FIELD_TYPE::TYPE_REAL2  && (type) <= PDB_FIELD_TYPE::TYPE_REAL6))
#define PDB_TYPE_IS_REAL(type)     ((type) >= PDB_FIELD_TYPE::TYPE_REAL2 && (type) <= PDB_FIELD_TYPE::TYPE_REAL6)


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


