using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PDB.DotNetSDK
{
  public enum PDBErrorCode
  {
    PdbE_OK                    = 0,     // 成功

    PdbE_IOERR                 = 50000, // I/O失败
    PdbE_OPENED                = 50001, // 对象已打开
    PdbE_NOMEM                 = 50002, // 内存申请失败
    PdbE_FILE_EXIST            = 50003, // 文件已存在
    PdbE_FILE_READONLY         = 50004, // 只读文件
    PdbE_PATH_TOO_LONG         = 50005, // 路径太长
    PdbE_TABLE_CFG_ERROR       = 50006, // 表配置文件错误
    PdbE_USER_CFG_ERROR        = 50007, // 用户配置文件错误
    PdbE_DEVID_FILE_ERROR      = 50008, // 设备文件错误
    PdbE_IDX_FILE_ERROR        = 50009, // 索引文件错误
    PdbE_FILE_NOT_FOUND        = 50010, // 文件不存在
    PdbE_DATA_LOG_ERROR        = 50011, // 日志文件错误
    PdbE_END_OF_DATALOG        = 50012, // 数据日志读取完成
    PdbE_PATH_NOT_FOUND        = 50013, // 目录不存在
    PdbE_DATA_LOG_VER_ERROR    = 50014, // 数据日志版本错误
    PdbE_DATA_FILECODE_ERROR   = 50015, // 数据日志文件编号错误

    PdbE_INVALID_FILE_NAME     = 50100, // 无效的文件名
    PdbE_INVALID_PARAM         = 50101, // 无效的参数
    PdbE_INVALID_HANDLE        = 50102, // 无效的句柄
    PdbE_INVALID_USER_NAME     = 50103, // 无效的用户名
    PdbE_INVALID_USER_ROLE     = 50104, // 无效的用户角色
    PdbE_INVALID_INT_VAL       = 50105, // 无效的整型值
    PdbE_INVALID_DOUBLE_VAL    = 50106, // 无效的浮点值
    PdbE_INVALID_BLOB_VAL      = 50107, // 无效的Blob值
    PdbE_INVALID_TSTAMP_VAL    = 50108, // 无效的时间戳值
    PdbE_INVALID_DATETIME_VAL  = 50109, // 无效的DateTime值
    PdbE_INVALID_TABLE_NAME    = 50110, // 无效的表名
    PdbE_INVALID_DEVID         = 50111, // 无效的设备ID
    PdbE_INVALID_DEVNAME       = 50112, // 无效的设备名
    PdbE_INVALID_DEVEXPAND     = 50113, // 无效的设备扩展信息
    PdbE_INVALID_FIELD_NAME    = 50114, // 无效的字段名
    PdbE_INVALID_FIELD_TYPE    = 50115, // 无效的字段类型
    PdbE_INVALID_DEVID_FIELD   = 50116, // 无效的设备ID字段
    PdbE_INVALID_TSTAMP_FIELD  = 50117, // 无效的时间戳字段
    PdbE_OBJECT_INITIALIZED    = 50118, // 对象已初始化


    //记录和数据页相关
    PdbE_RECORD_FAIL           = 50200, // 错误的记录
    PdbE_RECORD_EXIST          = 50201, // 记录已存在
    PdbE_RECORD_TOO_LONG       = 50202, // 记录太长
    PdbE_PAGE_FILL             = 50203, // 数据页满
    PdbE_PAGE_ERROR            = 50204, // 数据页错误
    PdbE_VALUE_MISMATCH        = 50205, // 值类型不匹配
    PdbE_NULL_VALUE            = 50206, // 空值
    PdbE_TSTAMP_DISORDER       = 50207, // tstamp乱序
    PdbE_NODATA                = 50208, // 缺少数据
    PdbE_COMPRESS_ERROR        = 50209, // 压缩失败

    //表和字段相关
    PdbE_FIELD_NOT_FOUND       = 50300, // 字段不存在
    PdbE_FIELD_NAME_EXIST      = 50301, // 字段名存在
    PdbE_TABLE_NOT_FOUND       = 50302, // 表不存在
    PdbE_TABLE_FIELD_TOO_LESS  = 50303, // 表字段太少
    PdbE_TABLE_FIELD_TOO_MANY  = 50304, // 表字段太多
    PdbE_TABLE_PART_EXIST      = 50305, // 数据块已经存在
    PdbE_TABLE_CAPACITY_FULL   = 50306, // 表容量已满
    PdbE_TABLE_EXIST           = 50307, // 表已存在
    PdbE_DATA_FILE_IN_ACTIVE   = 50308, // 活跃的数据文件不能删除或分离
    PdbE_DATA_FILE_NOT_FOUND   = 50309, // 数据文件不存在

    //网络及任务相关
    PdbE_NET_ERROR             = 50400, // 网络错误
    PdbE_CONN_TOO_MANY         = 50401, // 客户段连接超过上限
    PdbE_PASSWORD_ERROR        = 50402, // 密码错误
    PdbE_PACKET_ERROR          = 50403, // 报文错误
    PdbE_OPERATION_DENIED      = 50404, // 操作被拒绝
    PdbE_TASK_CANCEL           = 50405, // 操作被取消
    PdbE_TASK_STATE_ERROR      = 50406, // 任务状态错误
    PdbE_RETRY                 = 50407, // 稍后重试
    PdbE_QUERY_TIME_OUT        = 50408, // 查询超时
    PdbE_NOT_LOGIN             = 50409, // 未登录
    PdbE_INSERT_PART_ERROR     = 50410, // 部分插入失败

    //SQL相关
    PdbE_SQL_LOST_ALIAS              = 50500, // 必须指定别名
    PdbE_SQL_GROUP_ERROR             = 50501, // SQL分组错误
    PdbE_SQL_GROUP_LOST_BEGIN_TSTAMP = 50502, // SQL分组缺少起始时间
    PdbE_SQL_ERROR                   = 50503, // SQL语句错误
    PdbE_SQL_CONDITION_EXPR_ERROR    = 50504, // SQL条件表达式错误
    PdbE_SQL_RESULT_ERROR            = 50505, // SQL结果集错误
    PdbE_SQL_RESULT_TOO_SMALL        = 50506, // SQL结果集太小
    PdbE_SQL_RESULT_TOO_LARGE        = 50507, // SQL结果集太大
    PdbE_SQL_LIMIT_ERROR             = 50508, // SQL Limit错误
    PdbE_SQL_NOT_QUERY               = 50509, // 不是查询SQL

    //设备、索引相关
    PdbE_RESLT_FULL                  = 50600, // 结果集已满
    PdbE_DEVID_EXISTS                = 50601, // 设备ID已存在
    PdbE_DEV_CAPACITY_FULL           = 50602, // 设备容量已满
    PdbE_USER_EXIST                  = 50603, // 用户已存在
    PdbE_USER_NOT_FOUND              = 50604, // 用户不存在
    PdbE_IDX_NOT_FOUND               = 50605, // 索引未找到
    PdbE_DEV_NOT_FOUND               = 50606 // 设备未找到

  }
}
