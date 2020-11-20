# 松果时序数据库（PinusDB）JDBC V3.0 使用手册
2020-11-20 , version 3.0
## 1.前言
**概述**  
本文档介绍松果时序数据库(以下又称PinusDB)3.0版本的JDBC SDK使用。

**读者对象**  
本文档适用于使用松果时序数据库的开发工程师。

**联系我们**  
如果您有任何疑问或建议，请提交Issue或发送邮件到 zhangqhn@foxmail.com 

## 2.快速开始  
在项目中导入jar包 "pinusdb_jdbc_3.x.x.jar" 即可。  
```java
public class Test {
public static void printResultSet(ResultSet rs) throws SQLException {
    ResultSetMetaData rsMeta = rs.getMetaData();
    int colCnt = rsMeta.getColumnCount();
    for(int i = 1; i <= colCnt; i++) {
        System.out.print("--");
        System.out.print(rsMeta.getColumnName(i));
	    System.out.print(",");
	    System.out.print(rsMeta.getColumnTypeName(i));
	    System.out.print("--|");
    }
    System.out.println();
    while(rs.next()) {
	for(int i = 1; i <= colCnt; i++) {
	    System.out.print(rs.getObject(i).toString());
	    System.out.print("  |  ");
	}
	System.out.println();
    }
}
public static void Query(String connStr, String querySql) {
    try {
        Connection conn = DriverManager.getConnection(connStr);
	    Statement statement = conn.createStatement();
	    ResultSet rs = statement.executeQuery(querySql);
	    printResultSet(rs);
	    rs.close();
	    conn.close();
    } catch (SQLException ex) {
	    System.out.println("Exception:");
	    System.out.println(ex.getMessage());
    }
}
public static void main(String[] args) throws ClassNotFoundException {
    Class.forName("cn.pinusdb.jdbc.PDBDriver");
    String connStr = "jdbc:pinusdb://127.0.0.1:8105?user=sa&password=pinusdb";
    Query(connStr, "select * from sys_table");
}
}
```

## 3 PDBDriver类  
加载注册驱动程序。  
注意：松果时序数据库JDBC并没有实现所有的接口，请注意阅读相应文档。

**实现的方法**  
acceptsURL(String url)  查询驱动程序是否可以打开给定的URL连接。  
connect(String url, Properties info)  创建一个给定的URL的数据连接。  
getMajorVersion()  获取驱动程序的主版本号  
getMinorVersion()  获取驱动程序的次版本号  

### 3.1 acceptsURL  
查询驱动程序是否可以打开给定的URL连接。  
```java
boolean acceptsURL(String url) throws SQLException  
```
**参数：**  
    url  松果时序数据库的连接URL。   
    如："jdbc:pinusdb://127.0.0.1:8105 ?user=sa&password=pinusdb"  
**返回值：**  
    如果此驱动程序支持给定的 URL，则返回 true；否则，返回 false。
  
### 3.2 connect  
创建一个松果时序数据库的连接。  
```java
Connection connect(String url,  
  Properties info)  
  throws SQLException  
```
**参数：**  
  url  表示松果时序数据库的连接串。  
  info  作为连接参数的任意字符串键值对列表，可以包括"user"和"password"属性。  
**返回值：**  
  创建的连接对象。
  
### 3.3 getMajorVersion
获取驱动程序的主版本号。
```java
int getMajorVersion()
```  
**返回值：**   
  驱动程序的主版本号。

### 3.4 getMinorVersion  
  获取驱动程序的次版本号。
```java
int getMinorVersion()
```
**返回值：**  
驱动程序的次版本号。

## 4 PDBConnection类  
JDBC与松果时序数据库的连接对象。  

**实现的方法**
close  释放此连接对象的资源。  
createStatement  创建一个Statement对象用来执行对数据库操作。  
prepareStatement  创建一个PreparedStatement对象用来执行对数据库操作。  
isValid 判断连接是否有效。  


### 4.1 close  
关闭数据库连接并释放资源。
```java
void close() throws SQLException
```

### 4.2 createStatement  
创建一个Statement对象用来执行对数据库的操作。
```java
Statement createStatement() throws SQLException
```
返回值：  
Statemnt对象。

### 4.3 prepareStatement  
创建一个PreparedStatement对象用来将参数化的SQL语句发送到数据库执行。  
```java
PreparedStatement prepareStatement(String sql)
throws SQLException
```
参数：  
sql  包含一个或多个'?' 参数占位符的SQL语句。  
返回值：  
PreparedStatement 对象。

### 4.4 isValid  
判断连接是否有效  
```sql
boolean isValid(int timeout) throws SQLException
```
参数：  
 timeout : 暂未使用，为了兼容性请传递一个大于等于0的参数。

返回值：  
 连接有效返回true，无效返回false 

## 5 PDBStatement类  
用于执行静态SQL语句并返回结果的对象。  

**实现的方法：**  
close  释放对象占用的资源。  
execute  执行给定的SQL语句，在松果时序数据库中最多能返回一个结果。  
executeQuery  执行给定的SQL语句，返回一个ResultSet对象。  
executeUpdate  执行给定的SQL语句，可以是insert语句或不反回结果的语句。  
getConnection  获取当前对象的Connection对象。  
getResultSet  获取当前查询结果集。  
getUpdateCount  返回更新计数的结果，如果结果为ResultSet或没有结果，则返回-1。  



### 5.1 close  
释放对象占用的资源。
```java
void close() throws SQLException
```
抛出：   
SQLException - 如果发生数据库访问错误

### 5.2 execute
执行给定的SQL语句，在松果时序数据库中最多能返回一个结果。  
松果时序数据库只支持UTF8编码。
```java
boolean execute(String sql) throws SQLException
```
参数：  
 sql   要执行的SQL语句。  

返回值：  
  如果执行结果为ResultSet对象，则返回true。  
  如果执行结果为更新计数或没有执行结果，则返回false。

抛出：  
SQLException - 如果发生数据库访问错误，或者在已关闭的 Statement 上调用此方法，或者未能正确执行发送到数据库的命令之一。

### 5.3 executeQuery  
执行给定的 SQL 语句，该语句返回单个 ResultSet 对象。  
PinusDB只支持UTF8编码，驱动中已实现sql转UTF8。  
```java
ResultSet executeQuery(String sql) throws SQLException
```
参数：   
sql - 要发送给数据库的 SQL 查询语句。  
返回：  
包含给定查询所生成数据的 ResultSet 对象；  
抛出：  
SQLException - 如果发生数据库访问错误，在已关闭的 Statement 上调用此方法，或者给定 SQL 语句生成单个 ResultSet 对象之外的任何其他内容。

### 5.4 executeUpdate  
执行给定 SQL 语句，该语句可能为 INSERT语句，或者不返回任何内容的 SQL 语句（如 SQL DDL 语句）。
```java
int executeUpdate(String sql) throws SQLException
```
参数：   
sql - SQL 数据操作语言（Data Manipulation Language，DML）语句，如 INSERT；或者不返回任何内容的 SQL 语句，如 DDL 语句。 
返回：  
(1) 对于 SQL 数据操作语言 (DML) 语句，返回行计数   
(2) 对于什么都不返回的 SQL 语句，返回 0   
抛出：  
SQLException - 如果发生数据库访问错误，在已关闭的 Statement 上调用此方法，或者给定的 SQL 语句生成 ResultSet 对象。

### 5.5 getConnection  
获取生成此 Statement 对象的 Connection 对象。 
```java
Connection getConnection() throws SQLException
```
返回：  
此语句生成的连接。  
抛出：  
SQLException - 如果发生数据库访问错误，或者在已关闭的 Statement 上调用此方法

### 5.6 getResultSet  
以 ResultSet 对象的形式获取当前结果。每个结果只应调用一次此方法。 
```java
ResultSet getResultSet() throws SQLException
```
返回：  
以 ResultSet 对象的形式返回当前结果；如果结果是更新计数或没有更多的结果，则返回 null  
抛出：  
SQLException - 如果发生数据库访问错误，或者在已关闭的 Statement 上调用此方法

### 5.7 getUpdateCount  
以更新计数的形式获取当前结果；如果结果为 ResultSet 对象或没有更多结果，则返回 -1。每个结果只应调用一次此方法。
```java
int getUpdateCount() throws SQLException
```
返回：   
以更新计数的形式返回当前结果；如果当前结果为 ResultSet 对象或没有更多结果，则返回 -1   
抛出：   
SQLException - 如果发生数据库访问错误，或者在已关闭的 Statement 上调用此方法

## 6 PDBPreStatement类  
表示预编译的 SQL 语句的对象。  
SQL 语句被预编译并存储在 PreparedStatement 对象中。然后可以使用此对象多次高效地执行该语句。   
**实现的方法**  
addBatch  将一组参数添加到此 PreparedStatement 对象的批处理命令中。  
clearParameters  清除当前参数值。  
execute  在此 PreparedStatement 对象中执行 SQL 语句，该语句可以是任何种类的 SQL 语句。  
executeQuery  在此 PreparedStatement 对象中执行 SQL 查询，并返回该查询生成的 ResultSet 对象。  
executeUpdate  在此 PreparedStatement 对象中执行 SQL 语句。  
executeBatch  执行批量插入。  
setBoolean  将指定参数设置为 boolean 值。  
setBytes  将指定参数设置为 blob 值。  
setDouble  将指定参数设置为 double 值。  
setFloat  将指定参数设置为 float 值。  
setLong  将指定参数设置为long 值。  
setObject  使用给定对象设置指定参数的值。  
setString  将指定参数设置为 String 值。  
setTimestamp  将指定参数设置为 datetime 值。  

### 6.1 addBatch
```java
void addBatch() throws SQLException
```
将一组参数添加到此 PreparedStatement 对象的批处理命令中。  
注意：本方法仅支持批量执行插入，并且每个值都必须使用参数，并使用setXXX方法指定参数值。  
例如：以下SQL支持批量执行:  
```sql
insert into tab01(devid,tstamp,val01,val02) values(?,?,?,?)
```
以下SQL不支持批量执行:
```sql
insert into tab01(devid,tstamp,val01,val02) values(10,?,?,?)
insert into tab01(devid,tstamp,val01,val02) values(?,?,?,?),(?,?,?,?)
```
抛出：  
SQLException - 如果发生数据库访问错误，或者在关闭的 PreparedStatement 上调用此方法

### 6.2 clearParameters  
清除当前参数值。 
```java
void clearParameters()  throws SQLException
```
通常参数值对语句的重复使用仍然有效。设置一个参数值会自动清除其以前的值。不过，在某些情况下，直接释放当前参数值使用的资源也是很有用的；这可以通过调用 clearParameters 方法实现。  
抛出： 
SQLException - 如果发生数据库访问错误，或者在关闭的 PreparedStatement 上调用此方法。

### 6.3 execute  
```java
boolean execute() throws SQLException
```
在此 PreparedStatement 对象中执行 SQL 语句，该语句可以是任何种
类的 SQL 语句。但不支持返回多个结果集的语句。execute 方法返回一个 boolean 值，指示结果集的形式。必须调用getResultSet 或 getUpdateCount 方法获取该结果。  
建议执行查询时选择 executeQuery 方法。  
执行批量插入时选择 executeBatch 方法。  
执行其他语句时选择 executeUpdate 方法。  

返回：  
如果第一个结果是 ResultSet 对象，则返回 true；如果第一个结果是更新计数或者没有结果，则返回 false 

抛出： 
SQLException - 如果发生数据库访问错误；在关闭的 PreparedStatement 上调用此方法，或者为此方法提供了参数

### 6.4 executeQuery  
```java
ResultSet executeQuery() throws SQLException
```
在此 PreparedStatement 对象中执行 SQL 查询，并返回该查询生成的 ResultSet 对象。   
返回：  
包含该查询生成的数据的 ResultSet 对象；  
抛出：  
SQLException - 如果发生数据库访问错误，在关闭的 PreparedStatement 上调用此方法。  

### 6.5 executeUpdate
```java
int executeUpdate() throws SQLException
```
在此 PreparedStatement 对象中执行 SQL 语句，该语句必须是一个 SQL 数据操作语言（Data Manipulation Language，DML）语句，比如 INSERT语句；或者是无返回内容的 SQL 语句，比如 DDL 语句。   
返回：   
(1) SQL 数据操作语言 (DML) 语句的行数。  
(2) 对于无返回内容的 SQL 语句，返回 0 。  

抛出：  
QLException - 如果发生数据库访问错误，在关闭的 PreparedStatement 上调用此方法，或者 SQL 语句返回一个 ResultSet 对象

### 6.6 executeBatch
```java
int[] executeBatch() throws SQLException
```
在此PreparedStatement 对象上执行批量插入语句。  
返回：  
每条语句执行的结果。 可调用getUpdateCount 方法获取成功插入的条数。  
抛出：  
SQLException - 如果发生数据库访问错误，在关闭的 PreparedStatement 上调用此方法，或者 不是插入语句。  

### 6.7 setBoolean
```java
void setBoolean(int parameterIndex,boolean x)
    throws SQLException
```
将指定参数设置为 boolean 值。  
参数：  
parameterIndex - 第一个参数是 1，第二个参数是 2，……   
x - 参数值  

抛出：  
SQLException - 如果 parameterIndex 不对应于 SQL 语句中的参数标记；

### 6.8 setBytes
```java
void setBytes(int parameterIndex, byte[] x)
    throws SQLException
```
将指定参数设置为 blob 值。   
参数：  
parameterIndex - 第一个参数是 1，第二个参数是 2，……   
x - 参数值 

抛出：  
SQLException - 如果 parameterIndex 不对应于 SQL 语句中的参数标记；

### 6.9 setDouble
```java
void setDouble(int parameterIndex, double x)
    throws SQLException
```
将指定参数设置为 double 值

参数：  
parameterIndex - 第一个参数是 1，第二个参数是 2，……   
x - 参数值   

抛出：   
SQLException - 如果 parameterIndex 不对应于 SQL 语句中的参数标记；

### 6.10 setFloat
```java
void setFloat(int parameterIndex, float x)
    throws SQLException
```
将指定参数设置为 Float值。  
参数：  
parameterIndex - 第一个参数是 1，第二个参数是 2，……   
x - 参数值 

抛出：   
SQLException - 如果 parameterIndex 不对应于 SQL 语句中的参数标记；如果发生数据库访问错误，或者在关闭的 PreparedStatement 上调用此方法。

### 6.11 setLong
```java
void setLong(int parameterIndex, long x)
    throws SQLException
```
将指定参数设置为 long 值。  

参数：  
parameterIndex - 第一个参数是 1，第二个参数是 2，……   
x - 参数值 

抛出：  
SQLException - 如果 parameterIndex 不对应于 SQL 语句中的参数标记；如果发生数据库访问错误，或者在关闭的 PreparedStatement 上调用此方法。

### 6.12 setObject
```java
void setObject(int parameterIndex, Object x)
    throws SQLException
```
使用给定对象设置指定参数的值。第二个参数必须是 Object 类型；所以，应该对内置类型使用 java.lang 的等效对象。 

参数：   
parameterIndex - 第一个参数是 1，第二个参数是 2，……   
x - 包含输入参数值的对象 仅支持 boolean, long, Timestamp, float, double, String, byte[] 类型值。  
抛出：  
SQLException - 如果 parameterIndex 不对应于 SQL 语句中的参数标记；或者给定对象的类型不支持。

### 6.13 setString  
```java
void setString(int parameterIndex, String x)
    throws SQLException
```
将指定参数设置为 String 值。  
PinusDB只支持UTF8编码，驱动中已实现参数转UTF8。  

参数：  
parameterIndex - 第一个参数是 1，第二个参数是 2，……   
x - 参数值 

抛出：   
SQLException - 如果 parameterIndex 不对应于 SQL 语句中的参数标记；

### 6.14 setTimestamp
```java
void setTimestamp(int parameterIndex, Timestamp x)
    throws SQLException
```
将指定参数设置为 datetime 值。  

参数：  
parameterIndex - 第一个参数是 1，第二个参数是 2，……   
x - 参数值  
抛出：   
SQLException - 如果 parameterIndex 不对应于 SQL 语句中的参数标记；

## 7 PDBResultSet类  
表示数据库结果集的数据表，通过执行查询数据库的语句生成。  
**实现的方法**  
absolute  将指针移动到此 ResultSet 对象的给定行编号。  
afterLast  将指针移动到此 ResultSet 对象的末尾，正好位于最后一行之后。  
beforeFirst  将指针移动到此 ResultSet 对象的开头，正好位于第一行之前。  
close  释放此 ResultSet 对象的数据库和 JDBC 资源。  
findColumn  将给定的 ResultSet 列标签映射到其 ResultSet 列索引。  
first  将指针移动到此 ResultSet 对象的第一行。  
getBoolean  获取 boolean 值。  
getBytes  获取byte[]值，对应表中blob列。  
getFloat  获取float值，对应表中float列。  
getDouble  获取double值，对应表中double列。  
getByte  获取byte值，对应表中的tinyint列。  
getShort  获取short值，对应表中的smallint列。  
getInt  获取int值，对应表中的int列。  
getLong  获取long值，对应表中的bigint列。  
getMetaData  获取此 ResultSet 对象的列的名称、类型和属性。  
getObject  以 Object 的形式获取值。若结果为String类型，则以UTF8编码。  
getRow  获取当前行编号。   
getString  获取String值。获取结果为UTF8编码。  
getTimestamp  获取Timestamp值。对应Datetime列。  
isAfterLast  获取指针是否位于此 ResultSet 对象的最后一行之后。  
isBeforeFirst  获取指针是否位于此 ResultSet 对象的第一行之前。  
isFirst  获取指针是否位于此 ResultSet 对象的第一行。  
isLast  获取指针是否位于此 ResultSet 对象的最后一行。  
last  将指针移动到此 ResultSet 对象的最后一行。  
next  将指针从当前位置向前移一行。  
previous  将指针移动到此 ResultSet 对象的上一行。  
relative  按相对行数（或正或负）移动指针。  
wasNull  报告最后一个读取的列是否是 SQL NULL。  

### 7.1 absolute
```java
boolean absolute(int row) throws SQLException
```
将光标移动到PDBResult 结果集给定的行编号。  
如果行编号为正，则将光标移动到相对于结果集开头的给定行编号。第一行为行 1，第二行为行 2，依此类推。   
如果给定行编号为负，则将光标移动到相对于结果集末尾的绝对行位置。例如，调用方法 absolute(-1) 将光标置于最后一行；调用方法 absolute(-2) 将光标移动到倒数第二行，依此类推。   
试图将光标置于结果集的第一行/最后一行之外将导致光标位于第一行之前或最后一行之后。   
注：调用 absolute(1) 等效于调用 first()。调用 absolute(-1) 等效于调用 last()。  

参数：   
row - 光标应该移动到的行的编号。正的编号指示从结果集开头开始计数的行编号；负的编号指示从结果集末尾开始计数的行编号 

返回： 
如果光标移动到此 PDBResult对象的位置处，则返回 true；如果光标在第一行的前面或最后一行的后面，则返回 false   

抛出：  
SQLException - 如果发生数据库访问错误

### 7.2 afterLast
```java
void afterLast()throws SQLException
```
将光标移动到此 PinusDBResult对象的末尾，正好位于最后一行之后。如果结果集中不包含任何行，则此方法无效。  

抛出：  
SQLException - 如果发生数据库访问错误

### 7.3 beforeFirst
```java
void beforeFirst() throws SQLException
```
将光标移动到此 PinusDBResult对象的开头，正好位于第一行之前。如果结果集中不包含任何行，则此方法无效。   

抛出：   
SQLException - 如果发生数据库访问错误

### 7.4 close
```java
void close() throws SQLException
```
立即释放此 PinusDBResult对象的数据库资源。  

抛出：  
SQLException - 如果发生数据库访问错误

### 7.5 findColumn
```java
int findColumn(String columnLabel)throws SQLException
```
将给定的 PDBResult列标签映射到其 PDBResult列索引。 

参数：   
columnLabel - 列名称 

返回：   
给定列名称的列索引 

抛出：   
SQLException - 如果 PDBResult对象不包含标记为 columnLabel 的列，或发生数据库访问错误或在已关闭的结果集上调用此方法。

### 7.6 first
```java
boolean first()throws SQLException
```
将光标移动到此 PDBResult对象的第一行。   
返回：   
如果光标位于有效行，则返回 true；如果结果集中不存在任何行，则返回 false   
抛出：  
SQLException - 如果发生数据库访问错误。

### 7.7 getBoolean
```java
byte getBoolean(int columnIndex)throws SQLException
```
以 Java 编程语言中 boolean的形式获取此 PDBResult对象的当前行中指定列的值。 

参数：   
columnIndex - 第一个列是 1，第二个列是 2，……   

返回：   
列值；如果值为 SQL NULL，则返回值为 false  

抛出：   
SQLException - 如果 columnIndex 无效；或者实际类型不匹配。

### 7.8 getLong
```java
long getLong(int columnIndex)throws SQLException
```
以 Java 编程语言中 long 的形式获取此 PDBResult对象的当前行中指定列的值。   
参数：   
columnIndex - 第一个列是 1，第二个列是 2，……   
返回：   
列值；如果值为 SQL NULL，则返回值为 0  
抛出：   
SQLException - 如果 columnIndex 无效；或者实际类型不匹配。

### 7.9 getLong
```java
long getLong(String columnLabel)throws SQLException
```
以 Java 编程语言中 long 的形式获取此 PDBResult对象的当前行中指定列的值。   
参数：   
columnLabel - 列名称   
返回：   
列值；如果值为 SQL NULL，则返回值为 0   
抛出：   
SQLException - 如果 columnLabel 无效；或者实际类型不匹配。  

### 7.10 getDouble
```java
double getDouble(int columnIndex)throws SQLException
```
以 Java 编程语言中 double 的形式获取此 PDBResult对象的当前行中指定列的值。   
参数：   
columnIndex - 第一个列是 1，第二个列是 2，……   
返回：   
列值；如果值为 SQL NULL，则返回值为 0  
抛出：  
SQLException - 如果 columnIndex 无效；或者实际类型不匹配。  

### 7.11 getDouble  
```java
double getDouble(String columnLabel) throws SQLException
```
以 Java 编程语言中 double 的形式获取此 PDBResult对象的当前行中指定列的值。   
参数：  
columnLabel - 列名称   
返回：  
列值；如果值为 SQL NULL，则返回值为 0  
抛出：  
SQLException - 如果 columnLabel 无效；或者实际类型不匹配。  

### 7.12 getString
```java
String getString(int columnIndex)throws SQLException
```
以 Java 编程语言中 String 的形式获取此 PDBResult对象的当前行中指定列的值。   

参数：   
columnIndex - 第一个列是 1，第二个列是 2，……   
返回：   
列值；如果值为 SQL NULL，则返回值为 null   
抛出：   
SQLException - 如果 columnIndex 无效；或者实际类型不匹配。  

### 7.13 getString
```java
String getString(String columnLabel)throws SQLException
```
以 Java 编程语言中 String 的形式获取此 PDBResult对象的当前行中指定列的值。  
参数：  
columnLabel - 列名称   
返回：  
列值；如果值为 SQL NULL，则返回值为 null   
抛出：  
SQLException - 如果 columnLabel 无效；或者实际类型不匹配。  

### 7.14 getBytes
```java
byte[] getBytes(int columnIndex)throws SQLException
```
以 Java 编程语言中 byte 数组的形式获取此 PDBResult对象的当前行中指定列的值。   
参数：   
columnIndex - 第一个列是 1，第二个列是 2，……   

返回：   
列值； 如果值为 SQL NULL，则返回值为 null 

抛出：   
SQLException - 如果 columnIndex 无效；或者实际类型不匹配。

### 7.15 getBytes
```java
byte[] getBytes(String columnLabel)throws SQLException
```
以 Java 编程语言中 byte 数组的形式获取此 PinusDBResult对象的当前行中指定列的值。  
参数：   
columnLabel -列名称   

返回：   
列值；如果值为 SQL NULL，则返回值为 null   

抛出：   
SQLException - 如果 columnLabel 无效；或者实际类型不匹配。  

### 7.16 getObject
```java
Object getObject(int columnIndex)throws SQLException
```
以 Java 编程语言中 Object 的形式获取此 PinusDBResult对象的当前行中指定列的值。     

参数：   
columnIndex - 第一个列是 1，第二个列是 2，……   

返回：   
保存列值的 java.lang.Object   

抛出：   
SQLException - 如果 columnIndex 无效；或者实际类型不匹配。

### 7.17 getObject  
```java
Object getObject(String columnLabel)throws SQLException
```
以 Java 编程语言中 Object 的形式获取此 PDBResult对象的当前行中指定列的值。    

参数： 
columnLabel - 列名称   

返回：   
保存列值的 java.lang.Object 

抛出：   
SQLException - 如果 columnLabel 无效；或者实际类型不匹配。

### 7.18 isAfterLast
```java
boolean isAfterLast()throws SQLException
```
获取光标是否位于此 PinusDBResult对象的最后一行之后。   

返回：   
如果光标位于最后一行之后，则返回 true；如果光标位于任何其他位置或者结果集不包含任何行，则返回 false 

抛出：   
SQLException - 如果发生数据库访问错误

### 7.19 isBeforeFirst
```java
boolean isBeforeFirst()throws SQLException
```
获取光标是否位于此 PinusDBResult对象的第一行之前。 

返回：  
如果光标位于第一行之前，则返回 true；如果光标位于任何其他位置或者结果集不包含任何行，则返回 false 

抛出：  
SQLException - 如果发生数据库访问错误

### 7.20 isFirst
```java
boolean isFirst() throws SQLException
```
获取光标是否位于此 PinusDBResult对象的第一行。   

返回：   
如果光标位于第一行，则返回 true；否则返回 false 

抛出：   
SQLException - 如果发生数据库访问错误

### 7.21 isLast
```java
boolean isLast()throws SQLException
```
获取光标是否位于此 PinusDBResult对象的最后一行。  

返回：   
如果光标位于最后一行上，则返回 true；否则返回 false 

抛出：   
SQLException - 如果发生数据库访问错误

### 7.22 last
```java
boolean last()throws SQLException
```
将光标移动到此 PinusDBResult对象的最后一行。   
返回：  
如果光标位于有效行，则返回 true；如果结果集中不存在任何行，则返回 false   
抛出：   
SQLException - 如果发生数据库访问错误；

### 7.23 next
```java
boolean next()throws SQLException
```
将光标从当前位置向前移一行。PinusDBResult光标最初位于第一行之前；第一次调用 next 方法使第一行成为当前行；第二次调用使第二行成为当前行，依此类推。 

返回：   
如果新的当前行有效，则返回 true；如果不存在下一行，则返回 false   
抛出：  
SQLException - 如果发生数据库访问错误

### 7.24 previous
```java
boolean previous()throws SQLException
```
将光标移动到此 PinusDBResult对象的上一行。   
当调用 previous 方法返回 false 时，光标位于第一行之前。  
返回：  
如果光标现在位于有效行上，则返回 true；如果光标位于第一行的前面，则返回 false   
抛出：  
SQLException - 如果发生数据库访问错误；  

### 7.25 relative
```java
boolean relative(int rows)throws SQLException
```
按相对行数（或正或负）移动光标。试图移动到结果集的第一行/最后一行之外，会将光标置于第一行之前或最后一行之后。调用 relative(0) 有效，但是不更改光标位置。  

注：调用方法 relative(1) 等效于调用方法 next()，而调用方法 relative(-1) 等效于调用方法 previous()。   

参数：  
rows - 指定从当前行开始移动的行数的 int；正数表示光标向前移动；负数表示光标向后移动   

返回：  
如果光标位于行上，则返回 true；否则返回 false 

抛出：  
SQLException - 如果发生数据库访问错误；

### 7.26 getRow
```java
int getRow()throws SQLException
```
获取当前行编号。第一行为 1 号，第二行为 2 号，依此类推。   

返回： 
当前行的编号；如果不存在当前行，则返回 0   

抛出：  
SQLException - 如果发生数据库访问错误  

### 7.27 wasNull
```java
boolean wasNull()throws SQLException
```
报告最后一个读取的列是否具有值 SQL NULL。注意，必须首先对列调用一个获取方法尝试读取其值，然后调用 wasNull 方法查看读取的值是否为 SQL NULL。   

返回：  
如果最后一个读取的列值为 SQL NULL，则返回 true；否则返回 false   

抛出：   
SQLException - 如果发生数据库访问错误

## 8 PDBErrCode类  
静态类，包含所有PinusDB错误码及错误信息。

**方法摘要**  
errMsg  静态方法，根据错误值，返回错误信息  

### 8.1 errMsg
```java
String errMsg(int errCode)
```
静态方法，返回错误码的错误描述信息。   
参数：  
errCode - 错误码  
返回：   
错误描述信息