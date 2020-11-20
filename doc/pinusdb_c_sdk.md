# 松果时序数据库（PinusDB）用户手册 V3.0
2020-11-20 , version 3.0
## 1.前言
**概述**  
本文档介绍松果时序数据库(以下又称PinusDB)3.0版本的C/C++ SDK使用。

**读者对象**  
本文档适用于使用松果时序数据库的开发工程师。

**联系我们**  
如果您有任何疑问或建议，请提交Issue或发送邮件到 zhangqhn@foxmail.com 

## 2.快速入门
在项目中引用pdb_csdk.lib并包含pdb_api.h文件，将pdb_csdk.dll拷贝到执行程序的目录下即可（注意：松果时序数据库提供的c sdk为64位版本，若需要32位版本请自行下载源码编译即可）。

一个简单的示例：
```cpp
#include <time.h>
#include <stdio.h>
#include "pdb_api.h"
 
int main(int argc, char* argv[])
{
  PdbErr_t retVal = PdbE_OK;
  int handle = 0;
  size_t sucessCnt = 0;
  char timeBuf[32];
  char sqlBuf[1024];
 
  retVal = pdb_connect("127.0.0.1", 8105, &handle);
  if (retVal != PdbE_OK)
  {
    std::cout << "连接数据库失败:" << pdb_get_error_msg(retVal) << std::endl;
    return 0;
  }
 
  do {
    retVal = pdb_login(handle, "sa", "pinusdb");
    if (retVal != PdbE_OK)
    {
      std::cout << "登录失败:" << pdb_get_error_msg(retVal) << std::endl;
      break;
    }
 
    //创建表
    retVal = pdb_execute_non_query(handle,
      "create table tab01(\
         devid bigint,\
         tstamp datetime,\
         val01 bigint,\
         val02 float)");
 
    if (retVal != PdbE_OK)
    {
      std::cout << "创建表失败:" << pdb_get_error_msg(retVal) << std::endl;
      break;
    }
 
    //创建设备
    retVal = pdb_execute_insert(handle,
      "insert into sys_dev(tabname, devid, devname)\
       values('tab01', 1, 'dev_01'), ('tab01', 2, 'dev_02')", &sucessCnt, nullptr);
    if (retVal != PdbE_OK)
    {
      std::cout << "创建设备失败:" << pdb_get_error_msg(retVal) << std::endl;
      break;
    }
 
    time_t t;
    tm tm;
    time(&t);
    localtime_s(&tm, &t);
sprintf_s(timeBuf,"%d-%d-%d %d:%d:%d", (1900 + tm.tm_year), (1 + tm.tm_mon), tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
 
    sprintf_s(sqlBuf,
      "insert into tab01(devid, tstamp, val01, val02) \
      values(1, '%s', 1023, 57.35),(2, '%s', 10, 32.337)",
      timeBuf, timeBuf);
 
    retVal = pdb_execute_insert(handle, sqlBuf, &sucessCnt, nullptr);
    if (retVal != PdbE_OK)
    {
      std::cout << "插入数据失败:" << pdb_get_error_msg(retVal) << std::endl;
      break;
    }
 
  } while (false);
 
  pdb_disconnect(handle);
  return 0;
}
```

## 3. pdb_connect  
连接松果时序数据库。  
**函数原型**  
```c
PdbErr_t
pdb_connect(
  const char* pHostName,
  int32_t port,
  int32_t* pHandle
);
```
**参数**  
+ pHostName : [输入参数] 松果时序数据库服务启动的IP地址。  
+ port : [输入参数] 松果时序数据库服务启动的端口号。   
+ pHandle : [输出参数] 执行成功时，设置为连接句柄，做为后续操作的参数。使用完后需要调用pdb_disconnect关闭。   

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应错误码。  

## 4. pdb_disconnect  
断开松果时序数据库。  
**函数原型**
```c
PdbErr_t
pdb_disconnect(
  int32_t handle
);
```
**参数**  
+ handle : [输入参数] 要断开的句柄。 

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应错误码。

## 5. pdb_login  
登录松果时序数据库。  
**函数原型**  
```c
PdbErr_t
pdb_login(
  int32_t handle,
  const char* pUser,
  const char* pPassword
);
```
**参数**  
+ handle : [输入参数] 松果时序数据库数据库连接句柄。
+ pUser : [输入参数] 用户名。  
+ pPassword : [输入参数] 密码。  

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应错误码。  

## 6. pdb_get_error_msg  
根据错误码获取错误描述信息。  
**函数原型**
```c
const char*
pdb_get_error_msg(
  int32_t errorCode
);
```
**参数**  
+ errorCode : [输入参数] 错误码。  

**返回值**  
返回错误码的描述信息。

## 7. pdb_execute_insert  
插入数据到松果时序数据库。  
**函数原型**  
```c
PdbErr_t
pdb_execute_insert(
  int32_t handle, 
  const char* pSql,
  size_t* pSucessCnt,
  size_t* pPosition
);
```
**参数**  
+ handle : [输入参数] 连接句柄。  
+ pSql : [输入参数] 执行插入的sql语句，一次调用最多插入1000条数据。
+ pSuccessCnt : [输出参数] 执行成功的条数。  
+ pPosition : [输出参数] 错误SQL的位置(暂未使用)。

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应错误码。  

## 8. pdb_execute_query  
查询松果时序数据库中的数据。  
**函数原型**  
```c
PdbErr_t
pdb_execute_query(
  int32_t handle, 
  const char* pSql, 
  void** ppTable
);
```
**参数**  
+ handle : [输入参数] 连接句柄。  
+ pSql : [输入参数] sql查询语句。  
+ ppTable : [输出参数] 查询结果表，若函数返回PdbE_OK，在执行完数据的读取后需要调用pdb_table_free释放结果表。  
**返回值**  
执行成功返回PdbE_OK，执行失败返回对应错误码。  

## 9. pdb_execute_non_query  
执行非查询语句（修改密码，添加用户等）。  
**函数原型**  
```c
PdbErr_t
pdb_execute_non_query(
  int32_t handle, 
  const char* pSql
);
```
**参数**  
+ handle : [输入参数] 连接句柄。  
+ pSql : [输入参数] 执行的sql语句。  

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应错误码。  

## 10. pdb_table_free  
释放由pdb_execute_query返回的结果表。  
**函数原型**  
```c
PdbErr_t
pdb_table_free(
  void* pTable
);
```
**参数**  
+ pTable : [输入参数] 要释放的结果表。  

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应的错误码。  

## 11. pdb_table_get_column_count  
获取表中列的数量。  
**函数原型**  
```c
PdbErr_t
pdb_table_get_column_count(
  void* pTable, 
  size_t* pCnt
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ pCnt : [输入参数] 表中列的数量。  

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应错误码。  

## 12. pdb_table_get_column_info  
获取表中列的信息。  
**函数原型**  
```c
PdbErr_t
pdb_table_get_column_info(
  void* pTable,
  size_t columnIdx,
  ColumnInfo* pColInfo
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ columnIdx : [输入参数] 列下标，从0开始。  
+ pColInfo : [输出参数] 执行成功，存储列信息。  

**返回值**  
执行成功返回PdbE_OK， 执行失败返回对应的错误码。  

## 13. pdb_table_get_row_count  
获取表中行的数量。  
**函数原型**  
```c
PdbErr_t
pdb_table_get_row_count(
  void* pTable,
  size_t* pCnt
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ pCnt : [输出参数] 执行成功，存储表的行数。  
**返回值**  
执行成功返回PdbE_OK，执行失败返回对应的错误码。  

## 14. pdb_table_val_is_null_by_colidx  
根据行下标及列下标判断字段值是否为null。  
**函数原型**  
```c
PdbErr_t
pdb_table_val_is_null_by_colidx(
  void* pTable, 
  size_t rowIdx,
  size_t colIdx,
  bool* pIsNull
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ colIdx : [输入参数] 列下标，从0开始。  
+ pIsNull : [输出参数] 当值为null时，设置为true；当值非null时，设置为false。  
**返回值** 
执行成功返回PdbE_OK， 执行失败返回对应错误码。  

## 15. pdb_table_val_is_null_by_colname  
根据行下标及列名判断字段值是否为null。  
**函数原型**  
```c
PdbErr_t
pdb_table_val_is_null_by_colname(
  void* pTable, 
  size_t rowIdx, 
  const char* pColumnName,
  bool* pIsNull
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ pColumnName : [输入参数] 列名。  
+ pIsNull : [输出参数] 当值为null时，设置为true；当值非null时，设置为false。 
**返回值** 
执行成功返回PdbE_OK， 执行失败返回对应错误码。  

## 16. pdb_table_get_bool_by_colidx  
根据行下标及列下标获取bool类型的值。  
**函数原型**  
```c
PdbErr_t
pdb_table_get_bool_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  bool* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ colIdx : [输入参数] 列下标，从0开始。  
+ pVal : [输出参数] 执行成功时，返回获取到的值。  

**返回值**  
执行成功返回PdbE_OK， 执行失败返回对应的错误码。

## 17. pdb_table_get_bool_by_colname  
根据行下标及列名获取bool类型的值。  
**函数原型**  
```c
PdbErr_t
pdb_table_get_bool_by_colidx(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  bool* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ pColumnName : [输入参数] 列名。  
+ pVal : [输出参数] 执行成功时，返回获取到的值。  

**返回值**  
执行成功返回PdbE_OK， 执行失败返回对应的错误码。

## 18. pdb_table_get_tinyint_by_colidx  
根据行下标及列下标获取int8_t类型的值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_tinyint_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  int8_t* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ colIdx : [输入参数] 列下标，从0开始。  
+ pVal : [输出参数] 执行成功时，返回获取到的值。

**返回值**  
执行成功返回PdbE_OK， 执行失败返回对应的错误码。

## 19. pdb_table_get_tinyint_by_colname  
根据行下标和列名获取获取int8_t类型的值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_tinyint_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  int8_t* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ pColumnName : [输入参数] 列名。  
+ pVal : [输出参数] 执行成功时，返回获取到的值。

**返回值**  
执行成功返回PdbE_OK， 执行失败返回对应的错误码。

## 20. pdb_table_get_smallint_by_colidx  
根据行下标及列下标获取int16_t类型的值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_smallint_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  int16_t* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ colIdx : [输入参数] 列下标，从0开始。  
+ pVal : [输出参数] 执行成功时，返回获取到的值。

**返回值**  
执行成功返回PdbE_OK， 执行失败返回对应的错误码。

## 21. pdb_table_get_smallint_by_colname  
根据行下标和列名获取获取int8_t类型的值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_smallint_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  int16_t* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ pColumnName : [输入参数] 列名。  
+ pVal : [输出参数] 执行成功时，返回获取到的值。

**返回值**  
执行成功返回PdbE_OK， 执行失败返回对应的错误码。

## 22. pdb_table_get_int_by_colidx  
根据行下标及列下标获取int16_t类型的值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_int_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  int32_t* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ colIdx : [输入参数] 列下标，从0开始。  
+ pVal : [输出参数] 执行成功时，返回获取到的值。

**返回值**  
执行成功返回PdbE_OK， 执行失败返回对应的错误码。

## 23. pdb_table_get_int_by_colname  
根据行下标和列名获取获取int32_t类型的值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_int_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  int32_t* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ pColumnName : [输入参数] 列名。  
+ pVal : [输出参数] 执行成功时，返回获取到的值。

**返回值**  
执行成功返回PdbE_OK， 执行失败返回对应的错误码。


## 24. pdb_table_get_bigint_by_colidx  
根据行下标及列下标从表中获取bigint类型的值。

**函数原型**  
```c
PdbErr_t
pdb_table_get_bigint_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  int64_t* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ colIdx : [输入参数] 列下标，从0开始。  
+ pVal : [输出参数] 执行成功时，返回获取到的值。

**返回值**  
执行成功返回PdbE_OK， 执行失败返回对应的错误码。

## 25. pdb_table_get_bigint_by_colidx  
根据行下标及列名从表中获取bigint类型的值。

**函数原型**  
```c
PdbErr_t
pdb_table_get_bigint_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  int64_t* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ pColumnName : [输入参数] 列名。  
+ pVal : [输出参数] 执行成功时，返回获取到的值。

**返回值**  
执行成功返回PdbE_OK， 执行失败返回对应的错误码。

## 26. pdb_table_get_datetime_by_colidx
根据行下标及列下标从表中获取DateTime类型的值。  

**函数原型**
```c
PdbErr_t
pdb_table_get_blob_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  int64_t* pDateTimeVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ colIdx : [输入参数] 列下标，从0开始。  
+ pDateTimeVal : [输出参数] 执行成功时，返回获取到的值。  

**返回值**   
执行成功返回PdbE_OK，执行失败返回对应错误码。

## 27. pdb_table_get_datetime_by_colname
根据行下标及列下标从表中获取DateTime类型的值。  

**函数原型**
```c
PdbErr_t
pdb_table_get_blob_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  int64_t* pDateTimeVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ pColumnName : [输入参数] 列名。  
+ pDateTimeVal : [输出参数] 执行成功时，返回获取到的值。  

**返回值**   
执行成功返回PdbE_OK，执行失败返回对应错误码。

## 28. pdb_table_get_double_by_colidx  
根据列下标从表中获取double类型的值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_double_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  double* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ colIdx : [输入参数] 列下标，从0开始。
+ pVal : [输出参数] 执行成功时，返回获取到的值。  

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应的错误码。

## 29. pdb_table_get_double_by_colname  
根据列名从表中获取duoble类型的值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_double_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  double* pVal
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标， 从0开始。  
+ pColumnName : [输入参数] 列名。  
+ pVal : [输出参数] 执行成功时，返回获取到的值。  

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应的错误码。

## 30. pdb_table_get_string_by_colidx  
根据行下标及列下标获取string类型值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_string_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  const char** ppStrVal,
  size_t* pStrLen
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ colIdx : [输入参数] 列下标，从0开始。  
+ ppStrVal : [输出参数] 执行成功时，指向获取到的字符串。  
+ pStrLen : [输出参数] 执行成功时，输出字符串的长度。  

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应的错误码。

## 31. pdb_table_get_string_by_colname  
根据行下标及列名获取string类型值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_string_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  const char** ppStrVal,
  size_t* pStrLen
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ pColumnName : [输入参数] 列名。  
+ ppStrVal : [输出参数] 执行成功时，指向获取到的字符串。  
+ pStrLen : [输出参数] 执行成功时，输出字符串的长度。  

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应的错误码。

## 32. pdb_table_get_blob_by_colidx  
根据行下标及列下标获取blob类型值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_blob_by_colidx(
  void* pTable,
  size_t rowIdx,
  size_t colIdx,
  const uint8_t** ppBlobVal,
  size_t* pBlobLen
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ colIdx : [输入参数] 列下标，从0开始。  
+ ppBlobVal : [输出参数] 执行成功时，指向获取到的blob值。
+ pBlobLen : [输出参数] 执行成功时，输出blob值的长度。

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应的错误码。  

## 33. pdb_table_get_blob_by_colname  
根据行下标及列名获取blob类型值。  

**函数原型**  
```c
PdbErr_t
pdb_table_get_blob_by_colname(
  void* pTable,
  size_t rowIdx,
  const char* pColumnName,
  const uint8_t** ppBlobVal,
  size_t* pBlobLen
);
```
**参数**  
+ pTable : [输入参数] 表。  
+ rowIdx : [输入参数] 行下标，从0开始。  
+ pColumnName : [输入参数] 列名。  
+ ppBlobVal : [输出参数] 执行成功时，指向获取到的blob值。
+ pBlobLen : [输出参数] 执行成功时，输出blob值的长度。

**返回值**  
执行成功返回PdbE_OK，执行失败返回对应的错误码。  


