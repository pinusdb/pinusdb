# 松果时序数据库（PinusDB）用户手册 V3.0
2020-11-20 , version 3.0
## 1.前言
**概述**  
本文档介绍松果时序数据库(以下又称PinusDB)3.0版本的使用。

**读者对象**  
本文档适用于使用松果时序数据库的运维及开发工程师。

**联系我们**  
如果您有任何疑问或建议，请提交Issue或发送邮件到 zhangqhn@foxmail.com 

## 2.产品介绍
**产品概述**  
随着互联网的发展，计算机硬件价格下降、体积小型化使得智能设备大量普及，例如：手环、共享单车、智能电表、环境监测设备、新能源汽车、汽车充电桩等等，这些设备在运营过程中会持续产生数据；针对这些数据的分析能为企业决策、产品升级、智能调度等提供了数据支撑，人类也将步入智能时代。由于智能设备数量巨大，这就对传统数据处理方式提出了挑战。在此背景下我们根据智能设备产生的数据特点，设计、研发了高性能的松果时序数据库，帮助客户解决海量智能设备数据的处理。  

松果时序数据库（以下也称PinusDB）是一款针对物联网智能设备数据特点研发的具备高压缩比、高性能的时序数据库。广泛应用于物联网（IoT）设备、智慧城市、智慧物流、环境监测等数据处理。  

PinusDB针对传感数据基于时序的特点使用特殊的方式处理、存储。解决海量物联网设备高频率数据处理，经过我们特有的压缩算法降低数据存储空间90%以上，降低企业运维、管理成本。  
 
**产品优势**  
+ 高压缩比  
针对时序数据具备已知的取值范围及精度，提供real系列类型，最大限度压缩数据。一般能将历史数据存储空间压缩到原来的10%或更低。  
+ 高性能
具备搞笑的读写能力，普通PC机写入性能达到20万条/秒；支持交互式查询；数据按照设备分块存储，查询某设备一段时间的数据时具备极高的性能。
+ 大容量  
单表存储千亿级数据量。
+ 强大的历史数据管理  
数据库为每个表以天为单位存储为一个个单独的文件。可以以天为单位对历史数据进行管理（删除、附加、分离）等操作。 方便对数据进行转储、复制等。  
+ 简单易用  
基于类似关系库表的数据模型，使用类似SQL对进行数据操作，数据筛选、统计等，提供C/C++ SDK、 .Net SDK 以及 JDBC SDK。  

**名词解释**  
**时序数据**：持续产生的一系列数据。例如：监测某台电梯时，每秒采集电梯状态值而产生的一系列数据。  
**设备ID**(devid): 某个被监测的设备，大于0的整数，唯一标识一个设备，例如：某台电梯、某个充电桩、某辆共享单车等等。  
**时间戳**(tstamp): 数据产生的时间，取值范围1970-1-1 ~ 2999-12-31，精确到毫秒。  
**数据写入窗口**：松果时序数据库对数据的时间戳字段(tstamp)的值设定了时间窗口，只能写入处于时间窗口之内的数据。时间窗口以系统当前时间为标准，向前后扩展指定的时间（由系统表sys_config中insertValidDay项的取值确定，单位为天，可以修改系统配置文件config.ini中insertValidDay项指定合适的值，默认为1天）。例如：当前时间为2019-10-15，系统配置中insertValidDay的值为1，则数据写入窗口为 2019-10-14 0:0:0 到 2019-10-16 23:59:59，此处以格林尼治时间为标准，在中国东八区的实际写入窗口为 2019-10-14 8:0:0 到 2019-10-17 7:59:59。

## 3.数据类型
+ **bool** : 布尔类型，默认值 false
+ **tinyint** : 1字节整型，值域[-128, 127]。占用1个字节，默认值： 0  
+ **smallint** : 2字节整型，值域[-32768, 32767]。占用2个字节，默认值： 0  
+ **int** : 4字节整型，值域[-2^31, 2^31 - 1]，占用4个字节，默认值：0  
+ **bigint** : 8字节整型，值域[-2^63, 2^63 - 1], 占用8字节。默认值： 0  
+ **datetime** : 时间类型，值域[1970-1-1 ~ 2999-1-1], 精确到微秒，占用8字节。默认值：1970-1-1 0:0:0.000000  
+ **double** : 双精度浮点型，表示范围[-1.7e308, 1.7e308], 占用8字节， 默认值：0  
+ **string** : 字符串，UTF8 编码，整条数据小于8K。默认值：长度为0的空字符串
+ **blob** : 二进制数据，整条数据小于8K。默认值：长度为0的空数据
+ **real2** : 拥有2个小数位的浮点数，使用bigint存储。取值范围[-999,999,999.99 ~ +999,999,999.99], 实际读、写数据时数据类型是double，但比double类型拥有更高的压缩率。默认值: 0  
+ **real3** : 拥有3个小数位的浮点数，使用bigint存储。取值范围[-999,999,999.999 ~ +999,999,999.999], 实际读、写数据时数据类型是double，但比double类型拥有更高的压缩率。默认值: 0  
+ **real4** : 拥有4个小数位的浮点数，使用bigint存储。取值范围[-999,999,999.9999 ~ +999,999,999.9999], 实际读、写数据时数据类型是double，但比double类型拥有更高的压缩率。默认值: 0  
+ **real6** : 拥有6个小数位的浮点数，使用bigint存储。取值范围[-999,999,999.999999 ~ +999,999,999.999999], 实际读、写数据时数据类型是double，但比double类型拥有更高的压缩率。默认值: 0  

## 4.字面量
+ **数值**   
整数： 使用十进制数字书写，可以在前面加负号表示负数。  
浮点数： 使用点(.)做为整数和小数的分割，也可以在前面加负号表示负数。  

+ **字符串**  
字符串是使用单引号(')或双引号(")包含的字符序列。若使用单引号将字符串包含起来，字符串中出现单引号时，用两个连续的单引号表示一个单引号；双引号亦然。单引号包含的字符串中出现双引号时不需要处理，反之亦然。  
注意：字符串仅支持UTF8编码。  
示例：字符串 ab'cd"ef 的正确书写如下：  
'ab''cd"ef' 或 "ab'cd""ef"  

+ **二进制**  
二进制数据的书写是将一个字节分为两个十六进制数表示(0~F不区分大小写)，并使用x'和'包含起来。  
示例：0x283C 值的书写如下： x'283C'  

+ **时间类型**  
表示距离 1970-1-1 0:0:0.000000 的微秒数，可以用格式 'YYYY-MM-DD HH:mm:ss.us' 表示。 时间类型使用服务器所在的时区，若需要指定其他的时区可以在时间后加时区信息。东区为+，西区为-后面加时或时分。  
示例：北京时间可以表示为如下：  
'2019-10-10 18:41:23.327 +08' 或 '2019-10-10 18:41:23.327 +0800'  

## 5.运算符  
松果时序数据库仅支持AND逻辑运算符，支持以下比较运算符：  
+ 等于，=  
可以用于除blob外的其他数据类型。  
+ 不等于，<>  
可以用于除blob外的其他数据类型。  
+ 大于，>  
可以作用于bigint，double和real系列数据类型。  
+ 大于等于，>=  
可以作用于bigint，double和real系列数据类型。  
+ 小于，<  
可以作用于bigint，double和real系列数据类型。  
+ 小于等于，<=
可以作用于bigint，double和real系列数据类型。  
+ like  
可以作用于string类型，可以使用通配符，%匹配一个或多个任意字符；_匹配一个字符。
+ is null  
可以作用于所有类型，字段值为null，修改表结构可能会产生null值，详细信息参考相关章节，例如： *fieldName* is null  
+ is not null  
可以作用于所有类型，字段值不为null，修改表结构后可能会产生null值，详细信息参考相关章节，例如： *fieldName* is not null    
+ in  
可以作用于bigint类型，例如：devid in (1,3,5)  
+ not in  
可以作用于bigint类型，例如：devid not in (1, 3, 5)

## 6.聚合函数  
聚合函数对一组值执行计算并返回一个值。松果时序数据库的Group By子句只能引用设备列(devid)或时间戳列(tstamp)。  
聚合函数可以使用AS来指定别名。  
+ last  
功能：返回时间戳最大的值，支持所有类型的字段。返回值与字段类型一致。  
示例：last(*fieldName*)  
+ first  
功能：返回时间戳最小的值，支持所有类型的字段。返回值与字段类型一致。  
示例：first(*fieldName*)   
+ avg  
功能：返回指定字段的平均数，支持bigint，double及real系列字段。返回值与字段类型一致。  
示例：avg(*fieldName*)  
+ count  
功能：返回检索到并且值不为NULL的数据行数量，支持所有类型字段。返回值为bigint类型。  
示例：count(*fieldName*)  
+ max  
功能1：返回指定字段中最大的值，支持整数、浮点数及datetime类型字段。返回值与字段类型一致。  
示例：max(*fieldName*) 
功能2：返回指定字段(fieldName1)最大值时，字段fieldName2的值，返回值类型与fieldName2字段类型一致。  
fieldName1 支持整数、浮点数及datetime类型字段。  
fieldName2 支持所有数据类型。  
示例：max(*fieldName1*, *fieldName2*) 
+ min  
功能1：返回指定字段中最小的值，支持整数、浮点数及datetime类型字段。返回值与字段类型一致。 
示例：min(*fieldName*)  
功能2：返回指定字段(fieldName1)最小值时，字段fieldName2的值，返回值类型与fieldName2字段类型一致。  
示例：min(*fieldName1*, *fieldName2*)
+ sum  
功能：返回指定字段的和，支持bigint，double及real系列字段。返回值与字段类型一致。  
示例：sum(*fieldName*)  
+ -if
功能：所有聚合函数都可以加if后缀，来筛选满足条件的数据进行统计。 lastif, firstif, avgif, countif, maxif, minif, sumif ，这些函数的第一个参数为条件。 
示例： countif(*condition*)  
```sql
--查询val01大于100 和 val02 大于100的条数
SELECT countif(val01 > 100), countif(val02 > 100) FROM tab01  
--查询val01大于500的所有val02的平均值  
SELECT avg(val01 > 500, val02) FROM tab01
```

## 7.内置函数  
### 7.1 now函数
松果时序数据库提供获取服务器当前时间的内置函数now，在查询或插入数据时可以使用now函数获取服务器当前时间，精确到秒，例如以下示例：
```sql
insert into tab01(devid, tstamp, ...) values(1, now(), ...)
```
查询示例:
```sql
select now()
--查询结果
--|2020-11-18 22:21:04 
```

### 7.2 datetimeadd函数  
计算一个时间与另一个时间段相加，第一个参数为datetime类型，第二个参数表示微秒数，可以为负数。  
例如：计算当前时间前1分钟及后一分钟（1分钟为60000000微秒）的示例如下：
```sql
SELECT datetimeadd(now(), -60000000), datetimeadd(now(), 60000000)
```
为了方便时间运算也可以选择使用时间符号: 秒(s),分钟(m),小时(h), 如下所示：  
```sql
SELECT datetimeadd(now(), 60s), datetimeadd(now(), -1d)
```

### 7.3 datetimediff函数  
计算两个时间差的微秒数，例如:  
```sql
SELECT datetimediff(datetimeadd(now(), 1m), now())
```

### 7.4 datetimefloor函数  
时间值的向下取整，取整单位: 秒(second), 分钟(minute), 小时(hour), 天(day), 示例如下：  
```sql
--获取今天的零点
SELECT datetimefloor(now(), 'day')
```
注意：以天为单位进行取整时，使用的是服务器的时区。  

### 7.5 datetimeceil函数
时间值的向上取整, 取整单位：秒(second), 分钟(minute), 小时(hour), 天(day), 示例如下：  
```sql
SELECT datetimeceil(now(), 'minute')
```

### 7.6 add函数  
加法函数，支持 tinyint, smallint, int, bigint, float, double数据类型，示例如下:  
```sql
SELECT add(12, 3.5)
```

### 7.7 sub函数  
减法函数，支持 tinyint, smallint, int, bigint, float, double数据类型，示例如下：  
```sql
SELECT sub(12.8, 8.2)
```

### 7.8 mul函数  
乘法函数，支持 tinyint, smallint, int, bigint, float, double数据类型，示例如下：  
```sql
SELECT mul(5, 8)
```

### 7.9 div函数  
除法函数，支持 tinyint, smallint, int, bigint, float, double数据类型，示例如下：  
```sql
SELECT div(5, 8.0)
```

### 7.10 mod函数  
求模函数，支持 tinyint, smallint, int, bigint 数据类型，示例如下：  
```sql
SELECT mod(5, 8)
```

### 7.11 if函数  
条件函数，需要三个参数，第一个参数为条件，若条件为真返回第二个参数，若条件为假返回第三个参数，示例如下:  
```sql
SELECT if(5 > 8, 'true', 'false')
```
注意：第二、三个参数必须具备同样的数据类型。  

### 7.12 abs函数  
绝对值函数， 支持tinyint, smallint, int, bigint, float, double数据类型，示例如下：  
```sql
SELECT abs(-5),abs(5)
```

## 8.系统限制
+ 单台数据库最多同时打开32个表
+ 数据表名不能以sys_开头
+ 表名长度：小于48字节
+ 字段名长度：小于48字节
+ 用户名长度：小于48字节
+ 设备名长度：小于96字节
+ 设备扩展属性长度(sys_dev表expand字段)：小于128字节
+ 单条数据长度： 小于8KB
+ 一个表最多包含860个字段
+ 单次写入最大记录数：1000条
+ 单次写入最大报文长度：4MB
+ 单次查询最大记录数：10000条

## 9.服务配置
在windows上服务配置文件config.ini与松果时序数据库执行程序处于同一目录下  
在Linux上服务配置文件位于：/etc/pinusdb/config.ini  
config.ini文件中server配置节下的配置项
+ address, 服务启动的IP地址；
+ port, 服务启动的端口，默认8105；
+ cacheSize, 数据缓存大小，单位为MB；默认为0，表示系统会根据内存大小来决定缓存大小，最小为128MB，最大不超过系统内存的5/8；  
当使用默认值时：  
物理内存小于8GB，缓存为物理内存的1/4  
物理内存小于64GB，缓存为物理内存的1/2  
物理内存大于64GB，缓存为32GB  
+ writeCache, 写数据缓存大小，单位为MB，推荐使用默认值  
默认值为0，即由系统自动设置。  
当数据缓存小于24GB，写缓存为数据缓存的1/2  
当数据缓存大于24GB，写缓存为12GB  

+ queryTimeOut，查询超时时间，单位为秒，默认值：300；
+ insertValidDay，有效写入时间窗口，单位为天，默认为1，详细信息参考名词解释相关内容；
+ tabPath，表目录，存放系统中创建的数据表及表配置文件，用户配置文件。  
+ normalDataPath，普通数据目录，存放普通数据文件。
+ compressDataPath，压缩数据目录，存放压缩数据文件。
+ commitLogPath， 数据日志目录；
+ sysLogPath, 系统日志目录；
+ logLevel， 输出日志级别，取值：debug，info，warning，error；默认为info；
+ compressFlag，是否启动压缩，true - 系统将超过有效写入时间窗口的数据进行压缩；false - 不进行压缩；默认为true；

注意：松果时序数据库启动前，tabPath,normalDataPath,compressDataPath,commitLogPath,sysLogPath必须配置好，否则启动失败。  
服务启动时会检查tabPath目录下表配置文件(table.json)以及用户文件(user.json)是否存在，若不存在，则系统创建空的表文件以及包含管理员：sa，密码为：pinusdb 的用户文件。  

## 10.系统表
用户可以通过系统表查询系统信息，松果时序数据库包含如下系统表：  
**sys_config** 数据库系统的配置信息，表结构如下：  

列名 | 类型 | 描述  
-|-|-
name | string | 配置项名  
value | string | 配置值  

系统所有配置信息如下：  

列名 | 类型  
-|-  
address | 数据库服务的IP地址  
port | 数据库服务的端口号  
cacheSize | 数据库缓存大小，单位为MB  
writeCache | 写数据缓存大小，单位为MB  
queryTimeOut | 查询超时时间，单位为秒  
insertValidDay | 插入时间窗口，单位为天  
tabPath | 表目录  
normalDataPath | 普通数据目录  
compressDataPath | 压缩数据目录  
commitLogPath | 数据日志目录  
sysLogPath | 系统日志目录  
logLevel | 输出日志级别  
compressFlag | 是否启动压缩  
majorVersion | 服务主版本 
minorVersion | 服务次版本  
buildVersion | 服务编译版本  

**sys_user** 用户信息表，包含数据库的用户信息，表结构如下：  

列名 | 类型 | 描述  
-|-|-  
username | string | 用户名  
userrole | string | 用户角色，readOnly, writeOnly, readWrite, admin  

**sys_table** 表信息，包含系统中所有的表名，表结构如下：  

列名 | 类型 | 描述  
-|-|-
tabname | string | 表名  

**sys_column** 表的列信息，表结构如下：

列名 | 类型 | 描述  
-|-|-  
tabname | string | 表名  
colname | string | 列名  
datatype | string | 类型名，bool,tinyint,smallint,int,bigint,datetime,double,string,blob,real2,real3,real4,real6  
iskey | bool | 是否主键  

**sys_dataFile** 数据文件列表，表结构如下：  

列名 | 类型 | 描述
-|-|-  
tabname | string | 表名  
filedate | string | 文件存储的数据日期  
filetype | string | 文件类型，normal：普通文件；compress：压缩文件  

**sys_dev** 设备表，表结构如下：  

列名 | 类型 | 描述  
-|-|-  
tabname | string | 表名  
devid | bigint | 设备ID  
devname | string | 设备名  
expand | string | 扩展信息  

**sys_connection** 网络连接表，表结构如下：  

列名 | 类型 | 描述  
-|-|-  
host | string | 客户端IP  
port | bigint | 客户端端口号  
user | string | 登录用户  
userrole | string | 用户角色  
conntime | datetime | 连接时间  

## 11.用户管理  
松果时序数据库用户角色分为readOnly、writeOnly、readWrite以及admin。  
**readOnly** 用户只能查询数据（不包括系统表）。  
**writeOnly** 用户只能向数据表写入数据。  
**readWrite** 用户可以查询并写入数据到数据表。  
**admin** 用户可以执行所有操作。  
### 11.1 查询用户信息  
查询系统表sys_user，表结构参考系统表对应部分。  

### 11.2 添加用户  
添加用户SQL 为： ADD USER *userName* IDENTIFIED BY '*password*'  
新添加的用户权限为readOnly  
添加用户testuser并设置密码为abc123，示例如下：
```sql
ADD USER testuser IDENTIFIED BY 'abc123'
```

### 11.3 修改用户权限
修改用户权限SQL 为：SET ROLE FOR *username* = readonly | writeonly | readwrite | admin  
设置用户testuser的角色为readonly，示例如下：  
```sql
SET ROLE FOR testuser = readonly
```
注意：系统不允许修改sa用户的角色。  

### 11.4 修改用户密码  
修改用户密码的SQL为： SET PASSWORD FOR testuser= PASSWORD('*newPassword*')  
修改用户testuser, 示例如下:  
```sql
SET PASSWORD FOR testuser = PASSWORD('123abc')
```

### 11.5 删除用户  
删除用户的SQL为： DROP USER *username*  
删除用户testuser，示例如下：  
```sql
DROP USER testuser
```
 
## 12.表管理  
### 12.1 查看表信息  
在松果时序数据库中，可以通过查询系统表sys_table及sys_column获取系统表和数据表的表结构。  
### 12.2 创建数据表  
使用 CREATE TABLE 语句创建表。每个表必须有两个固定的列：设备ID(devid)列，数据类型为bigint；时间戳(tstamp)列，数据类型为datetime；表的主键为设备ID和时间戳，创建表时不需要也不能显示指定主键，也不能修改他们的类型和顺序，一个典型的创建表语句如下所示：  
```sql
CREATE TABLE tabname
(
    devid bigint,
    tstamp datetime,
    ...
)
```
一个创建表tab01的示例：  
```sql
CREATE TABLE tab01
(
    devid bigint,
    tstamp datetime,
    val01 bool,
    val02 bigint,
    val03 datetime,
    val04 double,
    val05 real2,
    val06 real3,
    val07 real4,
    val08 real6,
    val09 string,
    val10 blob
)
```

### 12.3 修改表结构
使用 ALTER TABLE 语句修改表结构，设备ID(devid)和时间戳(tstamp)列不可修改。一个典型的修改表结构语句如下所示：  
```sql
ALTER TABLE tabname
(
    devid bigint,
    tstamp datetime,
    ...
)
```
一个修改表tab01的示例：
```sql
ALTER TABLE tab01
(
    devid bigint,
    tstamp datetime,
    val01 bool,
    val02 bigint,
    val03 datetime,
    val04 double,
    val05 string,
    val06 blob
)
```
概念说明：  
**字段标识** ： 使用列名（不区分大小写）和类型的组合标识一个字段。  
**删除字段** ： 若一个字段标识在原表结构中存在，新表结构中不存在，则表示该字段被删除。  
**新增字段** ： 若一个字段标识在原表结构中不存在，新表结构中存在，则表示该字段是新增字段。需要说名的是当修改一个字段从val01 bool 到 val01 bigint 后表示删除了字段val01 bool , 新增了字段 val01 bigint。
**修改表结构** ： 若修改了任何字段之间的顺序、字段名、字段类型，都将视为修改了表结构。  
  
修改表结构后有如下影响：  
**当系统执行插入数据时**：
+ 当对应的数据文件存在时，会检查当前表结构与数据文件中的表结构是否一致（字段名、类型、顺序完全一致），若不一致则插入失败。  
+ 当对应的数据文件不存在时，会使用当前表结构创建数据文件，然后将数据插入到新建的数据文件中。  

**当系统执行查询数据时**：  
删除的字段无法查询到（数据依然存在），新添加的字段在已存在的数据文件上查询时的值为null。  

修改表结构的实例：
```sql
--1. 创建表
CREATE TABLE test
(
  devid bigint,
  tstamp datetime,
  val01 bool,
  val02 bigint,
  val03 double,
  val04 real2
)

--2. 创建设备
INSERT INTO sys_dev(tabname, devid)
VALUES('test',1)

--3. 添加数据
INSERT INTO test(devid,tstamp,val01,val02,val03)
VALUES(1, now(), true, 1, 1.1111)

--4. 查询数据
SELECT * FROM test
-- devid | tstamp              | val01| val02 | val03 | val04
--     1 | 2019-10-14 13:29:37 | true |     1 | 1.1111 | 0
--从查询结果可知，若插入数据时未指定值，则会以该字段类型默认值填充

--5. 修改表结构
ALTER TABLE test
(
  devid bigint,
  tstamp datetime,
  val02 bigint,
  val03 double,
  val04 real2,
  val05 string
)

--6. 查询数据
SELECT * FROM test
-- devid | tstamp              | val02| val03  | val04 | val05
--     1 | 2019-10-14 13:29:37 |    1 | 1.1111 |     0 |  null
--从查询结果可知，已无法查询到列val01的数据，由于val05列是新加的列，查询结果为 null

--7. 添加数据到已存在的数据文件
INSERT INTO test(devid,tstamp,val02,val03)
VALUES(1, now(), 2, 2.222)
--执行失败，当插入到已存在的数据文件上时，数据文件与当前表结构不匹配，无法插入数据

--8. 添加数据到不存在的数据文件
INSERT INTO test(devid,tstamp,val02,val03)
VALUES(1, now(1d), 3, 3.333)
--执行成功，插入数据时使用当前表结构创建数据文件

--9. 查询数据
SELECT * FROM test
-- devid | tstamp              | val02| val03  | val04 | val05
--     1 | 2019-10-14 13:29:37 |    1 | 1.1111 |     0 | null
--     2 | 2019-10-15 13:40:10 |    3 | 3.333  |     0 |
```

### 12.4 分离表
分离表可以将数据表从系统中移除，但是保留表的设备文件和所有数据文件。分离表可以将一个表的所有数据转移到另一台服务器。具体操作结合后面的附加表和数据文件操作。  
分离表的使用方法为 DETACH TABLE *tabname*  
分离表tab01的示例如下：
```sql
DETACH TABLE tab01
```

### 12.5 附加表
附加表可以将表添加到系统中，表的设备文件必须在系统表目录下（即：sys_config系统表中tabPath配置项），使用 ATTACH TABLE *tabname* 将指定的表附加到系统中。  
附加表tab01的示例如下：
```sql
ATTACH TABLE tab01
```
注意：附加表后，表原来的数据文件不会自动附加，请根据后面数据文件管理部分手动附加数据文件。

### 12.6 删除表
使用DROP TABLE 语句删除数据表，删除表会删除表的设备文件和所有数据文件，请谨慎操作。  
删除表tab01的示例如下：
```sql
DROP TABLE tab01
```

## 13.设备管理
在松果时序数据库中，每条数据都属于某个设备，所以必须先添加设备后才能写入数据。目前由官方编译、发布的执行程序每个数据库服务支持的最大设备数量为10万台。  

### 13.1 查询设备  
系统中的设备信息可以通过查询系统表sys_dev获取，详细信息参考系统表部分。

### 13.2 添加设备  
管理员（角色为admin的用户才能添加设备）通过向sys_dev表中写入数据来添加设备。tabname必须是系统中已存在的表。设备ID(devid)必须大于0并且在每个表中唯一。devname和expand字段可以不指定。  
向数据表tab01中添加设备的示例如下：  
```sql
INSERT INTO sys_dev(tabname, devid, devname)
VALUES('tab01', 1, 'device 1'),('tab01', 2, 'device 2')
```

### 13.3 删除设备
管理员通过对sys_dev执行删除来删除设备，设备删除后其对应的数据并没有删除，但是无法访问。重新添加相同的设备ID后，之间的数据可以访问。  
几个删除设备的示例：  
```sql
DELETE FROM sys_dev WHERE tabname='tab01' AND devid = 1
DELETE FROM sys_dev WHERE tabname='tab01'
```

## 14.快照视图
松果时序数据库为每个数据表提供一个快照视图，快照视图包含该表中每个设备最新的一条数据。若某个设备未添加过数据，则不会出现在快照视图中；快照视图名为表名后加".snapshot"，例如：表tab01的快照视图为tab01.snapshot  
快照视图使用实例：
```sql
--1. 创建表
CREATE TABLE testSnap
(
  devid bigint,
  tstamp datetime,
  val01 bool,
  val02 bigint
)

--2. 创建设备
INSERT INTO sys_dev(tabname, devid, devname)
VALUES('testSnap', 1, 'device 1'),
('testSnap', 2, 'device 2'),
('testsnap', 3, 'device 3')

--3. 插入数据
INSERT INTO testSnap(devid, tstamp, val01, val02)
VALUES(1, now(), true, 101),(2, now(), false, 201)

--4. 查询快照
SELECT * FrOM testSnap.snapshot
--结果集
--devid | tstamp              | val01 | val02
--    1 | 2019-10-14 15:37:41 |  true | 101
--    2 | 2019-10-14 15:37:41 | false | 201
--从查询结果可知，设备3未添加过数据，故不会出现在快照视图中

--5. 继续插入数据
INSERT INTO testSnap(devid, tstamp, val01, val02)
VALUES(2, now(), true, 202),(3, now(), false, 301)

--6. 查询快照
SELECT * FROM testSnap.snapshot
--结果集
--devid | tstamp              | val01 | val02
--    1 | 2019-10-14 15:37:41 |  true | 101
--    2 | 2019-10-14 15:42:27 |  true | 202
--    3 | 2019-10-14 15:42:27 | false | 301
--从查询结果可知，每个设备只有最新的一条数据在快照视图中
```
下面是利用快照视图提供的一些功能，使用上例中testSnap表：  
+ 查询快照中满足某些条件的设备
```sql
SELECT * 
FROM testSnap.snapshot
WHERE devid in (1, 3, 5) AND val01 = true
```

+ 以5分钟作为设备离线的标准，即，某设备5分钟之内未上传新数据则认为该设备已离线:
```sql
--1. 查询离线设备的总数
SELECT count(*) AS cnt
FROM testSnap.snapshot
WHERE tstamp < now(-5m)

--2. 查询离线的设备，前1000条
SELECT devid
FROM testSnap.snapshot
WHERE tstamp < now(-5m)
LIMIT 1000
```

在松果时序数据库中，每次查询最多获取10000条数据，以查询离线设备为例：若要查询的设备超过10000条，则需要多次查询，如果简单使用LIMIT子句查询，两次查询之间可能有一些数据插入会导致某些数据无法查询到。
```sql
--1.创建设备
INSERT INTO sys_dev(tabname, devid, devname)
VALUES('testSnap', 101, 'device 101'),
('testSnap', 102, 'device 102'),
('testSnap', 103, 'device 103'),
('testSnap', 104, 'device 104'),
('testSnap', 105, 'device 105'),
('testSnap', 106, 'device 106')

--2.插入数据,以5分钟离线为例，所有设备都已离线
INSERT INTO testSnap(devid, tstamp, val01, val02)
VALUES(101, now(-10m), true, 1001),
(102, now(-10m), true, 2001),
(103, now(-10m), true, 3001),
(104, now(-10m), true, 4001),
(105, now(-10m), true, 5001),
(106, now(-10m), true, 6001)

--3.查询离线数据，每次查询两条
--3.1第一次查询
SELECT * FROM testSnap.snapshot
WHERE tstamp < now(-5m)
LIMIT 0,2
--查询到设备 101,102

--3.2第二次查询
SELECT * FROM testSnap.snapshot
WHERE tstamp < now(-5m)
LIMIT 2,2
--查询到设备 103,104

--3.3第三次查询
SELECT * FROM testSnap.snapshot
WHERE tstamp < now(-5m)
LIMIT 4,2
--查询到设备 105,106
```

在上例中，若3.1和3.2之间其他客户端插入了设备101的数据，如下示例：
```sql
--3.1第一次查询
SELECT * FROM testSnap.snapshot
WHERE tstamp < now(-5m)
LIMIT 0,2
--查询到设备 101,102

--插入数据
INSERT INTO testSnap(devid, tstamp, val01, val02)
VALUES(101, now(), false, 1002)

--3.2第二次查询
SELECT * FROM testSnap.snapshot
WHERE tstamp < now(-5m)
LIMIT 2,2
--查询到设备 104,105

--3.3第三次查询
SELECT * FROM testSnap.snapshot
WHERE tstamp < now(-5m)
LIMIT 4,2
--查询到设备 106
```
设备103未被查询到，故：直接使用LIMIT子句多次查询可能会少查询到数据，正确的方式应该如下：
```sql
--1. 第一次查询
SELECT * FROM testSnap.snapshot
WHERE tstamp < now(-5m)
LIMIT 2
--查询到设备 101,102

--2. 后面的查询
SELECT * FROM testSnap.snapshot
WHERE tstamp < now(-5m) AND devid > 102
LIMIT 2
```

从第二次开始每次查询限定查询的设备ID大于上一次最大的设备ID，不但能保证查询的数据正确性，还能提高查询性能。


## 15.数据文件管理
松果时序数据库将每个表每天的数据存储在一个单独的文件中（时间以格林尼治时间为标准对齐，由于中国处于东八区，实际存储时间为每天8点到次日8点前的数据，也就是文件'2019-10-10'实际存储的是'2019-10-10 8:0:0' 到 '2019-10-11 7:59:59'的数据）。  
管理员可以以文件为单位对数据进行管理，包括分离、附加、删除。可以方便对数据进行转储等操作。  
数据文件存储路径为：数据目录/表名/年/月/数据文件名。  
系统普通数据目录由sys_config表中normalDataPath项指定。  
系统压缩数据目录由sys_config表中compressDataPath项指定。  
数据文件名由表名_年月日组成。  
例如：  
系统普通数据目录为: e:/data，表tab01 在2019-8-10的普通文件：  
+ 数据文件路径: e:/data/tab01/2019/08/tab01_20190810.dat  
+ 索引文件路径：e:/data/tab01/2019/08/tab01_20190810.idx  

在目录中月和日占两个字符，若小于10在前面补0  
系统压缩数据目录为：e:/compress， 表tab01 在2019-8-5的压缩文件：
+ 数据文件路径：e:/compress/tab01/2019/08/tab01_20190805.cdat  

### 15.1 分离数据文件  
管理员可以将数据文件从系统中分离，分离后无法查询到对应数据。只能分离时间早于数据写入窗口（见产品介绍>名词解释部分）的数据文件。  
分离文件使用方法: DETACH DATAFILE '*date*' FROM *tabname*  
例如：分离表tab01在2019-8-10的数据文件实例如下：
```sql
DETACH DATAFILE '2019-8-10' FROM tab01
```

### 15.2 附加数据文件  
将数据文件附加在表上，可以查询到对应的数据。  
附加数据文件使用方法: ATTACH DATAFILE '*date*','*filetype*' FROM *tabname*  
其中filetype的取值：normal 普通文件；compress 压缩文件  
附加数据文件实例
```sql
--附加表tab01在2019-8-5的压缩文件
ATTACH DATAFILE '2019-8-5','compress' FROM tab01

--附加表tab01在2019-8-10的普通文件
ATTACH DATAFILE '2019-8-10','normal' FROM tab01
```

### 15.3 删除数据文件
将数据文件删除后，无法恢复，请谨慎操作！  
要删除的数据文件的日期必须早于数据写入窗口。  
删除数据文件使用方法：DROP DATAFILE '*date*' FROM *tabname*  
删除表tab01在2019-8-5的数据文件实例如下
```sql
DROP DATAFILE '2019-8-5' FROM tab01
```

## 16.数据写入
松果时序数据库支持文本协议(sql语句)和二进制协议数据写入，他们的差别如下：  
+ 文本协议：执行SQL的insert语句插入数据，sql需要服务端解析，性能稍低。  
+ 二进制协议：直接从报文中获取数据，带宽占用比文本协议低，性能高。
在.Net中以DataTable为参数写入；在java中以PreparedStatement配合addBatch，并且所有数据都必须使用参数。  

松果时序数据库支持一次写入一条或多条数据，不管是文本协议还是二进制协议都必须满足下面的条件：  
(1) 单次请求只能向一个表中写入数据。  
(2) 单次请求写入数据的上限为1000条。  
(3) 单次请求写入的数据报文大小不超过4MB。
(4) 写入数据的时间戳必须处于写入时间窗口。  

写入数据时，必须指定设备Id(devid)和时间戳(tstamp)的值，其他字段若没有指定值，会以该类型的默认值填充。  

### 16.1 顺序写入和随机写入的差别  
松果时序数据库写入数据时，根据数据的时间戳(tstamp)的值分配到不同的数据文件。数据文件以天为单位组织，需要注意的是：天的划分以格林尼治时间为标准，用于中国位于东八区，实际的划分为每天早上八点到第二天早上八点前的数据存储在一个文件中。  
若针对某个设备，写入记录的时间戳大于所处数据文件中该设备所有记录的时间戳，或大部分写入(90%以上)满足前述条件，此时认为写入是顺序写入；否则，认为写入是随机写入。  
顺序写入和随机写入有以下差别：  
(1) 顺序写入性能高于随机写入。  
(2) 对于普通数据文件(sys_datafile中的filetype值为normal)，同样的数据以顺序写入占用空间小于随机写入占用的空间。由于压缩文件(sys_datafile中filetype值为compress)不支持用户直接写入，不存在这个差别。  

高频写入数据会由于数据库服务内存不能及时写入磁盘，插入数据时返回 PdbE_RETRY错误码，此时可以暂停3-5秒后重新执行。

### 16.2 以文本协议写入数据
使用松果时序数据库的SDK执行写入，传递如下数据即可：
```sql
INSERT INTO tab01(devid, tstamp, val01)
VALUES(1, now(), 101),(2, now(), 102),(3, now(), 103)
```
注意：以文本协议写入多条数据时，若某条数据写入失败，则中止后面的数据写入，返回对应的错误码及成功写入的数据条数。

### 16.3 以二进制协议写入数据
在.Net中支持以一个DataTable写入，具体使用方式参考.Net的SDK文档。  
以DataTable写入时，除了需要满足本节开头列出的条件外，还需要满足：
+ 松果时序数据库中存在对应的表。  
+ DataTable中出现的列都在相应的表中存在，并且类型匹配。列名不区分大小写。

在Java中以PreparedStatement配合addBatch，并且每个值都必须使用参数指定。来使用二进制协议写入数据。具体细节参考JDBC的SDK文档。  
注意：与以文本协议写入数据不同，当其中某条数据写入失败时，其后的数据会继续写入，最终返回对应的错误码、成功写入的数据条数以及每条数据执行的结果。具体使用方式参考对应SDK文档。  

### 16.4 数据更新  
若需要更新已写入的数据，将新数据重新写入即可。需要注意的是，未指定的列将会以默认值填充。


## 17.数据查询
目前松果时序数据库不支持子查询以及多表join，查询语法如下所示：  
SELECT {* | expr_list}  
FROM *tabname*  
WHERE *condition*  
ORDER BY tstamp {ASC | DESC}  
GROUP BY {devid | tstamp time}  
LIMIT {*querycnt* | *offset*, *querycnt*}  

其中：WHERE、ORDER BY、GROUP BY、LIMIT 子句为可选。  
支持的聚合函数以及条件参考前面对应章节。  
ORDER BY 子句和GROUP BY子句不能同时存在。  
若不指定LIMIT 子句，默认查询前1000条数据，一次最多查询10000条数据。
