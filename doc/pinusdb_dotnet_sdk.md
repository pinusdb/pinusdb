# 松果时序数据库（PinusDB）用户手册 V3.0
2020-11-20 , version 3.0
## 1.前言
**概述**  
本文档介绍松果时序数据库(以下又称PinusDB)3.0版本的.Net SDK使用。

**读者对象**  
本文档适用于使用松果时序数据库的开发工程师。

**联系我们**  
如果您有任何疑问或建议，请提交Issue或发送邮件到 zhangqhn@foxmail.com 

## 2.快速开始  
在项目中引用PDB.DotNetSDK.dll程序集即可。PDB.DotNetSDK.dll 支持 .Net Framework和.Net Core。  
```c#
//连接字符串
string connStr = "server=127.0.0.1;port=8105;username=sa;password=pinusdb";
//实例化连接对象
PDBConnection conn = new PDBConnection(connStr);
//打开连接
conn.Open();
//创建执行对象
PDBCommand cmd = conn.CreateCommand();
//执行查询Sql
DataTable dt = cmd.ExecuteQuery("select * from sys_table");
//关闭数据库连接
conn.Close();
```

## 3.PDBConnection类  
一个PDBConnection类的实例表示一个与松果时序数据库的连接。  
**构造函数**  
PDBConnection(string connStr)  
参数connStr：使用指定的数据库连接串初始化连接对象，一个典型的连接字符串如下  
"server=127.0.0.1;port=8105;username=sa;password=pinusdb"  
实际使用时替换相应内容即可。  

**方法**
+ Open : 打开与松果时序数据库的连接。
+ Close : 关闭与松果时序数据库的连接。  
+ CreateCommand : 使用当前的连接对象创建一个命令执行对象。  
+ IsValid : 连接是否有效。

## 4.PDBCommand类  
表示松果时序数据库执行SQL语句的对象。  
**构造函数**  
PDBCommand(PDBConnection conn)  
参数conn：初始化PDBCommand类的实例。  

**属性**  
SuccessCount : 执行数据插入时，插入成功的条数。  
InsertResult : 执行数据插入时，每条数据的执行结果。  

**方法**  

+ ExecuteNonQuery(string sql)  
执行没有返回值的SQL，例如：创建用户，修改用户权限，创建表，删除表，附加表，附件文件等等。  

+ ExecuteNonQuery(string sql, params PDBParameter[] parms)  
使用参数执行没有返回值的SQL，变量名必须以@开头，可以包含字母、数字。  

+ ExecuteInsert(string sql)  
执行插入语句，当执行多条插入语句时，其中某条插入失败时，后面的不会继续执行。    
一次调用只能对一个表执行插入，必须指定所有的列名。典型的插入语句如下：
注意：插入数据前，请先创建创建表，在表中创建对象，具体请参考用户手册：pinusdb_user_manual.md 
```sql
--单条插入,插入实际的数据前需要先创建设备
INSERT INTO tab01(devid, tstamp, val01, val02)
VALUES(1, now(), true, 101)

--多条插入
INSERT INTO tab01(devid, tstamp, val01, val02)
VALUES(1, now(), true, 102),(2, now(), true, 103)
```
+ ExecuteInsert(string sql, params PDBParameter[] parms)  
带参数执行一条插入语句，变量名必须以@开头，可以包含字母、数字  

+ ExecuteInsert(DataTable dataTable)  
将DataTable插入到松果时序数据库，表名和列名由DataTable中的表名和列名指定，列类型必须和指定表的列类型匹配。
```C#
static void TestInsertTable()
{
  //测试表结构
  //CREATE TABLE testTab(devid bigint, tstamp datetime, val01 double)
  //已创建设备, 1,2
  string connStr = "server=127.0.0.1;port=8105;username=sa;password=pinxxx";
  //注意此处，表名与松果时序数据库中的表名对应
  DataTable dt = new DataTable("testTab");
  dt.Columns.Add(new DataColumn("devid", typeof(long)));
  dt.Columns.Add(new DataColumn("tstamp", typeof(DateTime)));
  dt.Columns.Add(new DataColumn("val01", typeof(double)));

  DataRow row1 = dt.NewRow();
  row1["devid"] = 1;
  row1["tstamp"] = DateTime.Now;
  row1["val01"] = 1.1;
  dt.Rows.Add(row1);
  DataRow row2 = dt.NewRow();
  row2["devid"] = 3;
  row2["tstamp"] = DateTime.Now;
  row2["val01"] = 2.2;
  dt.Rows.Add(row2);

  using (PDBConnection conn = new PDBConnection(connStr))
  {
    conn.Open();
    PDBCommand cmd = conn.CreateCommand();
    PDBErrorCode retCode = cmd.ExecuteInsert(dt);
    if (retCode == PDBErrorCode.PdbE_OK)
    {
      Console.WriteLine("执行插入成功，成功插入{0}条数据", cmd.SuccessCount);
    }
    else
    {
      Console.WriteLine("执行插入，成功插入{0}条数据", cmd.SuccessCount);
      for(int idx = 0; idx < cmd.InsertResult.Count(); idx++)
      {
        if (cmd.InsertResult[idx] != PDBErrorCode.PdbE_OK)
        {
          Console.WriteLine("第{0}条数据，插入失败，错误信息:{1}",
            idx, PDBErrorMsg.GetErrorMsg(cmd.InsertResult[idx]));
        }
      }
    }
  }
}
```

+ ExecuteInsert(string tabName, DataTable dataTable)  
将DataTable插入到松果时序数据库，表名由参数tabName指定，列名由DataTable中的列名指定。其他与上个方法相同。  


+ ExecuteQuery(string sql)  
执行查询SQL  

+ ExecuteQuery(string sql, params PDBParameter[] parms)  
使用参数执行查询，变量名必须以@开头，可以包含字母、数字。  

## 5. PDBParameter类  
表示执行SQL语句的参数。  
**构造函数**  
PDBParameter(string parameterName, PDBType parameterType)  
参数 parameterName: 参数名，以@开头，可以包含字母、数字。  
参数 parameterType: 参数类型  

**属性**  
ParameterName  参数名  
Value  参数值  

一个简单的示例：
```c#
static void TestQuery()
{
  string connStr = "server=127.0.0.1;port=8105;username=sa;password=pinxxx";
  using (PDBConnection conn = new PDBConnection(connStr))
  {
    conn.Open();
    PDBCommand cmd = conn.CreateCommand();
    string sql = "SELECT * FROM sys_column WHERE tabname = @tabname";
    PDBParameter[] queryParams = new PDBParameter[1];
    queryParams[0] = new PDBParameter("@tabname", PDBType.String);
    queryParams[0].Value = "testTab";
    DataTable dt = cmd.ExecuteQuery(sql, queryParams);
  }
}
```

## 6. PDBErrorMsg类  
用于获取错误描述信息，静态类。  
**静态方法**  
GetErrorMsg(PDBErrorCode errCode) : 根据错误码返回错误描述信息。  

## 7. PDBType枚举  
枚举类型，松果时序数据库支持的类型。  
成员：  
+ Bool : 布尔类型，对应.Net bool类型  
+ BigInt : 8字节整型，对应.Net Long类型  
+ DateTime : 时间类型，精确到毫秒，对应.Net DateTime类型  
+ Double : 双精度浮点类型，对应.Net double类型  
+ String : 字符串类型，UTF8编码，对应.Net string类型  
+ Blob : 二进制类型，对应.Net byte[]类型  

备注：松果时序数据库中的real系列类型，实际操作时当作double类型处理。  

## 8. 实例
### 8.1. 使用SQL插入数据  
注意：数据写入时，tstamp的值必须处于数据写入窗口内，数据写入窗口见用户手册名词解释部分。  
```c#
//运行前，请确保已创建表及添加设备
//CREATE TABLE tab01(devid bigint, tstamp datetime, val01 bigint, val02 double)
//INSERT INTO sys_dev(tabname,devid) VALUES('tab01', 101),('tab01', 102)
void InsertForSQL()
{
  string connStr = "server=127.0.0.1;port=8105;username=sa;password=pinusdb";
  using(PDBConnection conn = new PDBConnection(connStr))
  {
    conn.Open();
    PDBCommand cmd = conn.CreateCommand();
    PDBErrorCode retCode = cmd.ExecuteInsert("insert into tab01(devid, tstamp, val01, val02) values(101,now(),1,2.1),(102, now(),5,21)");
    if (retCode != PDBErrorCode.PdbE_OK)
    {
      Console.WriteLine("成功条数:" + cmd.SuccessCount + " 错误信息:" + PDBErrorMsg.GetErrorMsg(retCode));
    }
    else
      Console.WriteLine("插入成功!");
  }
}
```

### 8.2. 查询数据  
执行查询时，可以使用limit子句限制查询的数量，一次请求最多可以查询10000条数据，若不使用limit指定查询的数量，则默认查询1000条数据。  
```c#
using (PDBConnection conn = new PDBConnection(connStr))
{
  conn.Open();
  PDBCommand cmd = conn.CreateCommand();
  DataTable data = cmd.ExecuteQuery("select * from tab01 where devid = 101");
}

using (PDBConnection conn = new PDBConnection(connStr))
{
  conn.Open();
  PDBCommand cmd = conn.CreateCommand();
  PDBParameter[] queryParam = { new PDBParameter("@devid", PDBType.BigInt) };
  queryParam[0].Value = 101;
  DataTable data = cmd.ExecuteQuery("select * from tab01 where devid = @devid");
}
```
