# 松果时序数据库(pinusdb)

松果时序数据库是一款针对中小规模（设备数少于10万台，每天产生的数据量少于10亿条）场景设计的时序数据库。以简单、易用、高性能为设计目标。使用SQL语句进行交互，拥有极低的学习、使用成本, 提供了丰富的功能、较高的性能。  
我们的目标是成为最简单、易用、健壮的单机时序数据库。

## 1. 文档  

### 1.1 使用手册
[松果时序数据库-用户手册](https://gitee.com/pinusdb/pinusdb/blob/master/doc/pinusdb_user_manual.md)    

### 1.2 二次开发  
目前松果时序数据库提供c/c++ SDK, .Net SDK, jdbc 驱动，未来还会支持restful及更多的二次开发接口。  
[松果时序数据库-C/C++_SDK](https://gitee.com/pinusdb/pinusdb/blob/master/doc/pinusdb_c_sdk.md)  
[符合ADO.NET标准的.NET_SDK](https://gitee.com/maikebing/PinusDB.Data)  
[松果时序数据库-.Net_SDK](https://gitee.com/pinusdb/pinusdb/blob/master/doc/pinusdb_dotnet_sdk.md)  
[松果时序数据库-JDBC_SDK](https://gitee.com/pinusdb/pinusdb/blob/master/doc/pinusdb_jdbc.md)    

### 1.3 部署配置  
[松果时序数据库-Windows安装部署](https://gitee.com/pinusdb/pinusdb/blob/master/doc/pinusdb_windows_install.md)  
[松果时序数据库-管理工具使用手册](https://gitee.com/pinusdb/pinusdb/blob/master/doc/pinusdb_manage.md)  

### 1.4 内部设计  


### 1.5 其他文档  
[松果时序数据库-最佳实践](https://gitee.com/pinusdb/pinusdb/blob/master/doc/pinusdb_best_practice.md)  


## 2. 性能
在i3-7100， 8G 内存，1TB HDD windows server 2016 环境下，每条数据8个字段，达到每秒20万条数据写入。
最高数据扫描、统计达到5000万条每秒。
历史数据整理后压缩，每个设备的数据顺序存放，极大提供数据查询性能。

## 3. 压缩
松果时序数据库先将整数、浮点数按照差值压缩，然后将数据块以zlib压缩，极大提高压缩率。
不仅如此，我们还提供将浮点数按倍数放大后存储为整数，从而提高浮点数的压缩率。用户使用时以浮点数使用即可。  
real2 -> 倍数100，     取值范围[-999,999,999.99     ~ +999,999,999.99]   
real3 -> 倍数1000，    取值范围[-999,999,999.999    ~ +999,999,999.999]  
real4 -> 倍数10000，   取值范围[-999,999,999.9999   ~ +999,999,999.9999]  
real6 -> 倍数1000000， 取值范围[-999,999,999.999999 ~ +999,999,999.999999]

## 4. 容量
在松果时序数据库中，每个表每天的数据存储为一个文件，超过写入时间窗口的文件会被压缩。
所以，数据容量仅限于服务器存储的容量，并且在大容量下还能保持极高的数据读取性能。  
并且，用户可以对数据文件进行管理（分离、附加、删除）等操作，方便对数据进行备份。

## 5. 数据安全性
数据写入松果时序数据库中，首先会写commit日志，commit日志每3秒或写满64KB会刷一次磁盘，所以意外宕机，或服务器断电后只会丢失较少的数据。
松果时序数据库写数据文件时使用doublewrite，保证写入数据页时发送断电数据文件和数据页也不会损坏。

# 交流
若您需要帮助或希望给我们反馈信息，请提交Issue或发送邮件到: zhangqhn@foxmail.com

