package cn.pinusdb.jdbc;

import java.util.HashMap;

public class PDBErrCode {
	public static final int PdbE_OK                    = 0;     // 成功
	
	public static final int PdbE_IOERR                 = 50000; // I/O失败
	public static final int PdbE_OPENED                = 50001; // 对象已打开
	public static final int PdbE_NOMEM                 = 50002; // 内存申请失败
	public static final int PdbE_FILE_EXIST            = 50003; // 文件已存在
	public static final int PdbE_FILE_READONLY         = 50004; // 只读文件
	public static final int PdbE_PATH_TOO_LONG         = 50005; // 路径太长
	public static final int PdbE_TABLE_CFG_ERROR       = 50006; // 表配置文件错误
	public static final int PdbE_USER_CFG_ERROR        = 50007; // 用户配置文件错误
	public static final int PdbE_DEVID_FILE_ERROR      = 50008; // 设备文件错误
	public static final int PdbE_IDX_FILE_ERROR        = 50009; // 索引文件错误
	public static final int PdbE_FILE_NOT_FOUND        = 50010; // 文件不存在
	public static final int PdbE_DATA_LOG_ERROR        = 50011; // 日志文件错误
	public static final int PdbE_END_OF_DATALOG        = 50012; // 数据日志读取完成
	public static final int PdbE_PATH_NOT_FOUND        = 50013; // 目录不存在
	public static final int PdbE_DATA_LOG_VER_ERROR    = 50014; // 数据日志版本错误
	public static final int PdbE_DATA_FILECODE_ERROR   = 50015; // 数据日志文件编号错误
	
	public static final int PdbE_INVALID_FILE_NAME     = 50100; // 无效的文件名
	public static final int PdbE_INVALID_PARAM         = 50101; // 无效的参数
	public static final int PdbE_INVALID_HANDLE        = 50102; // 无效的句柄
	public static final int PdbE_INVALID_USER_NAME     = 50103; // 无效的用户名
	public static final int PdbE_INVALID_USER_ROLE     = 50104; // 无效的用户角色
	public static final int PdbE_INVALID_INT_VAL       = 50105; // 无效的整型值
	public static final int PdbE_INVALID_DOUBLE_VAL    = 50106; // 无效的浮点值
	public static final int PdbE_INVALID_BLOB_VAL      = 50107; // 无效的Blob值
	public static final int PdbE_INVALID_TSTAMP_VAL    = 50108; // 无效的时间戳值
	public static final int PdbE_INVALID_DATETIME_VAL  = 50109; // 无效的DateTime值
	public static final int PdbE_INVALID_TABLE_NAME    = 50110; // 无效的表名
	public static final int PdbE_INVALID_DEVID         = 50111; // 无效的设备ID
	public static final int PdbE_INVALID_DEVNAME       = 50112; // 无效的设备名
	public static final int PdbE_INVALID_DEVEXPAND     = 50113; // 无效的设备扩展信息
	public static final int PdbE_INVALID_FIELD_NAME    = 50114; // 无效的字段名
	public static final int PdbE_INVALID_FIELD_TYPE    = 50115; // 无效的字段类型
	public static final int PdbE_INVALID_DEVID_FIELD   = 50116; // 无效的设备ID字段
	public static final int PdbE_INVALID_TSTAMP_FIELD  = 50117; // 无效的时间戳字段
	public static final int PdbE_OBJECT_INITIALIZED    = 50118; // 对象已初始化
	
	
	    //记录和数据页相关
	public static final int PdbE_RECORD_FAIL           = 50200; // 错误的记录
	public static final int PdbE_RECORD_EXIST          = 50201; // 记录已存在
	public static final int PdbE_RECORD_TOO_LONG       = 50202; // 记录太长
	public static final int PdbE_PAGE_FILL             = 50203; // 数据页满
	public static final int PdbE_PAGE_ERROR            = 50204; // 数据页错误
	public static final int PdbE_VALUE_MISMATCH        = 50205; // 值类型不匹配
	public static final int PdbE_NULL_VALUE            = 50206; // 空值
	public static final int PdbE_TSTAMP_DISORDER       = 50207; // tstamp乱序
	public static final int PdbE_NODATA                = 50208; // 缺少数据
	public static final int PdbE_COMPRESS_ERROR        = 50209; // 压缩失败
	
	    //表和字段相关
	public static final int PdbE_FIELD_NOT_FOUND       = 50300; // 字段不存在
	public static final int PdbE_FIELD_NAME_EXIST      = 50301; // 字段名存在
	public static final int PdbE_TABLE_NOT_FOUND       = 50302; // 表不存在
	public static final int PdbE_TABLE_FIELD_TOO_LESS  = 50303; // 表字段太少
	public static final int PdbE_TABLE_FIELD_TOO_MANY  = 50304; // 表字段太多
	public static final int PdbE_TABLE_PART_EXIST      = 50305; // 数据块已经存在
	public static final int PdbE_TABLE_CAPACITY_FULL   = 50306; // 表容量已满
	public static final int PdbE_TABLE_EXIST           = 50307; // 表已存在
	public static final int PdbE_DATA_FILE_IN_ACTIVE   = 50308; // 活跃的数据文件不能删除或分离
	public static final int PdbE_DATA_FILE_NOT_FOUND   = 50309; // 数据文件不存在
	
	    //网络及任务相关
	public static final int PdbE_NET_ERROR             = 50400; // 网络错误
	public static final int PdbE_CONN_TOO_MANY         = 50401; // 客户段连接超过上限
	public static final int PdbE_PASSWORD_ERROR        = 50402; // 密码错误
	public static final int PdbE_PACKET_ERROR          = 50403; // 报文错误
	public static final int PdbE_OPERATION_DENIED      = 50404; // 操作被拒绝
	public static final int PdbE_TASK_CANCEL           = 50405; // 操作被取消
	public static final int PdbE_TASK_STATE_ERROR      = 50406; // 任务状态错误
	public static final int PdbE_RETRY                 = 50407; // 稍后重试
	public static final int PdbE_QUERY_TIME_OUT        = 50408; // 查询超时
	public static final int PdbE_NOT_LOGIN             = 50409; // 未登录
	public static final int PdbE_INSERT_PART_ERROR     = 50410; // 部分插入失败
	
	    //SQL相关
	public static final int PdbE_SQL_LOST_ALIAS              = 50500; // 必须指定别名
	public static final int PdbE_SQL_GROUP_ERROR             = 50501; // SQL分组错误
	public static final int PdbE_SQL_GROUP_LOST_BEGIN_TSTAMP = 50502; // SQL分组缺少起始时间
	public static final int PdbE_SQL_ERROR                   = 50503; // SQL语句错误
	public static final int PdbE_SQL_CONDITION_EXPR_ERROR    = 50504; // SQL条件表达式错误
	public static final int PdbE_SQL_RESULT_ERROR            = 50505; // SQL结果集错误
	public static final int PdbE_SQL_RESULT_TOO_SMALL        = 50506; // SQL结果集太小
	public static final int PdbE_SQL_RESULT_TOO_LARGE        = 50507; // SQL结果集太大
	public static final int PdbE_SQL_LIMIT_ERROR             = 50508; // SQL Limit错误
	public static final int PdbE_SQL_NOT_QUERY               = 50509; // 不是查询SQL
	
	    //设备、索引相关
	public static final int PdbE_RESULT_FULL                 = 50600; // 结果集已满
	public static final int PdbE_DEVID_EXISTS                = 50601; // 设备ID已存在
	public static final int PdbE_DEV_CAPACITY_FULL           = 50602; // 设备容量已满
	public static final int PdbE_USER_EXIST                  = 50603; // 用户已存在
	public static final int PdbE_USER_NOT_FOUND              = 50604; // 用户不存在
	public static final int PdbE_IDX_NOT_FOUND               = 50605; // 索引未找到
	public static final int PdbE_DEV_NOT_FOUND               = 50606; // 设备未找到
	
	private static HashMap<Integer, String> errMsgMap_;
	
	static {
		errMsgMap_ = new HashMap<Integer, String>();
        errMsgMap_.put(PdbE_OK, "成功");
    	
        errMsgMap_.put(PdbE_IOERR, "I/O失败");
        errMsgMap_.put(PdbE_OPENED, "对象已打开");
        errMsgMap_.put(PdbE_NOMEM, "内存申请失败");
        errMsgMap_.put(PdbE_FILE_EXIST, "文件已存在");
        errMsgMap_.put(PdbE_FILE_READONLY, "只读文件");
        errMsgMap_.put(PdbE_PATH_TOO_LONG, "路径太长");
        errMsgMap_.put(PdbE_TABLE_CFG_ERROR, "表配置文件错误");
        errMsgMap_.put(PdbE_USER_CFG_ERROR, "用户配置文件错误");
        errMsgMap_.put(PdbE_DEVID_FILE_ERROR, "设备文件错误");
        errMsgMap_.put(PdbE_IDX_FILE_ERROR, "索引文件错误");
        errMsgMap_.put(PdbE_FILE_NOT_FOUND, "文件不存在");
        errMsgMap_.put(PdbE_DATA_LOG_ERROR, "日志文件错误");
        errMsgMap_.put(PdbE_END_OF_DATALOG, "数据日志读取完成");
        errMsgMap_.put(PdbE_PATH_NOT_FOUND, "目录不存在");
        errMsgMap_.put(PdbE_DATA_LOG_VER_ERROR, "数据日志版本错误");
        errMsgMap_.put(PdbE_DATA_FILECODE_ERROR, "数据日志文件编号错误");
	
        errMsgMap_.put(PdbE_INVALID_FILE_NAME, "无效的文件名");
        errMsgMap_.put(PdbE_INVALID_PARAM, "无效的参数");
        errMsgMap_.put(PdbE_INVALID_HANDLE, "无效的句柄");
        errMsgMap_.put(PdbE_INVALID_USER_NAME, "无效的用户名");
        errMsgMap_.put(PdbE_INVALID_USER_ROLE, "无效的用户角色");
        errMsgMap_.put(PdbE_INVALID_INT_VAL, "无效的整型值");
        errMsgMap_.put(PdbE_INVALID_DOUBLE_VAL, "无效的浮点值");
        errMsgMap_.put(PdbE_INVALID_BLOB_VAL, "无效的Blob值");
        errMsgMap_.put(PdbE_INVALID_TSTAMP_VAL, "无效的时间戳值");
        errMsgMap_.put(PdbE_INVALID_DATETIME_VAL, "无效的DateTime值");
        errMsgMap_.put(PdbE_INVALID_TABLE_NAME, "无效的表名");
        errMsgMap_.put(PdbE_INVALID_DEVID, "无效的设备ID");
        errMsgMap_.put(PdbE_INVALID_DEVNAME, "无效的设备名");
        errMsgMap_.put(PdbE_INVALID_DEVEXPAND, "无效的设备扩展信息");
        errMsgMap_.put(PdbE_INVALID_FIELD_NAME, "无效的字段名");
        errMsgMap_.put(PdbE_INVALID_FIELD_TYPE, "无效的字段类型");
        errMsgMap_.put(PdbE_INVALID_DEVID_FIELD, "无效的设备ID字段");
        errMsgMap_.put(PdbE_INVALID_TSTAMP_FIELD, "无效的时间戳字段");
        errMsgMap_.put(PdbE_OBJECT_INITIALIZED, "对象已初始化");
	
	
	    //记录和数据页相关
        errMsgMap_.put(PdbE_RECORD_FAIL, "错误的记录");
        errMsgMap_.put(PdbE_RECORD_EXIST, "记录已存在");
        errMsgMap_.put(PdbE_RECORD_TOO_LONG, "记录太长");
        errMsgMap_.put(PdbE_PAGE_FILL, "数据页满");
        errMsgMap_.put(PdbE_PAGE_ERROR, "数据页错误");
        errMsgMap_.put(PdbE_VALUE_MISMATCH, "值类型不匹配");
        errMsgMap_.put(PdbE_NULL_VALUE, "空值");
        errMsgMap_.put(PdbE_TSTAMP_DISORDER, "tstamp乱序");
        errMsgMap_.put(PdbE_NODATA, "缺少数据");
        errMsgMap_.put(PdbE_COMPRESS_ERROR, "压缩失败");
	
	    //表和字段相关
        errMsgMap_.put(PdbE_FIELD_NOT_FOUND, "字段不存在");
        errMsgMap_.put(PdbE_FIELD_NAME_EXIST, "字段名存在");
        errMsgMap_.put(PdbE_TABLE_NOT_FOUND, "表不存在");
        errMsgMap_.put(PdbE_TABLE_FIELD_TOO_LESS, "表字段太少");
        errMsgMap_.put(PdbE_TABLE_FIELD_TOO_MANY, "表字段太多");
        errMsgMap_.put(PdbE_TABLE_PART_EXIST, "数据块已经存在");
        errMsgMap_.put(PdbE_TABLE_CAPACITY_FULL, "表容量已满");
        errMsgMap_.put(PdbE_TABLE_EXIST, "表已存在");
        errMsgMap_.put(PdbE_DATA_FILE_IN_ACTIVE, "活跃的数据文件不能删除或分离");
        errMsgMap_.put(PdbE_DATA_FILE_NOT_FOUND, "数据文件不存在");
	
	    //网络及任务相关
        errMsgMap_.put(PdbE_NET_ERROR, "网络错误");
        errMsgMap_.put(PdbE_CONN_TOO_MANY, "客户段连接超过上限");
        errMsgMap_.put(PdbE_PASSWORD_ERROR, "密码错误");
        errMsgMap_.put(PdbE_PACKET_ERROR, "报文错误");
        errMsgMap_.put(PdbE_OPERATION_DENIED, "操作被拒绝");
        errMsgMap_.put(PdbE_TASK_CANCEL, "操作被取消");
        errMsgMap_.put(PdbE_TASK_STATE_ERROR, "任务状态错误");
        errMsgMap_.put(PdbE_RETRY, "稍后重试");
        errMsgMap_.put(PdbE_QUERY_TIME_OUT, "查询超时");
        errMsgMap_.put(PdbE_NOT_LOGIN, "未登录");
        errMsgMap_.put(PdbE_INSERT_PART_ERROR, "部分插入失败");
	
	    //SQL相关
        errMsgMap_.put(PdbE_SQL_LOST_ALIAS, "必须指定别名");
        errMsgMap_.put(PdbE_SQL_GROUP_ERROR, "SQL分组错误");
        errMsgMap_.put(PdbE_SQL_GROUP_LOST_BEGIN_TSTAMP, "SQL分组缺少起始时间");
        errMsgMap_.put(PdbE_SQL_ERROR, "SQL语句错误");
        errMsgMap_.put(PdbE_SQL_CONDITION_EXPR_ERROR, "SQL条件表达式错误");
        errMsgMap_.put(PdbE_SQL_RESULT_ERROR, "SQL结果集错误");
        errMsgMap_.put(PdbE_SQL_RESULT_TOO_SMALL, "SQL结果集太小");
        errMsgMap_.put(PdbE_SQL_RESULT_TOO_LARGE, "SQL结果集太大");
        errMsgMap_.put(PdbE_SQL_LIMIT_ERROR, "SQL Limit错误");
        errMsgMap_.put(PdbE_SQL_NOT_QUERY, "不是查询SQL");
	
	    //设备、索引相关
        errMsgMap_.put(PdbE_RESULT_FULL, "结果集已满");
        errMsgMap_.put(PdbE_DEVID_EXISTS, "设备ID已存在");
        errMsgMap_.put(PdbE_DEV_CAPACITY_FULL, "设备容量已满");
        errMsgMap_.put(PdbE_USER_EXIST, "用户已存在");
        errMsgMap_.put(PdbE_USER_NOT_FOUND, "用户不存在");
        errMsgMap_.put(PdbE_IDX_NOT_FOUND, "索引未找到");
        errMsgMap_.put(PdbE_DEV_NOT_FOUND, "设备未找到");
	}
	
	public static String errMsg(int errCode) {
		if (errMsgMap_.containsKey(errCode)) {
			return errMsgMap_.get(errCode)  + ", " + errCode;
		}
		
		return "未知的错误, " + errCode;
	}
}

