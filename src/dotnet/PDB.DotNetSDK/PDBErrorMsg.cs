using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PDB.DotNetSDK
{

  public class PDBErrorMsg
  {
    private static Dictionary<PDBErrorCode, string> errMsgDic;

    public static string GetErrorMsg(PDBErrorCode errCode)
    {
      if (errMsgDic == null)
      {
        errMsgDic = new Dictionary<PDBErrorCode, string>();

        errMsgDic.Add(PDBErrorCode.PdbE_OK, "成功");

        errMsgDic.Add(PDBErrorCode.PdbE_IOERR, "I/O失败");
        errMsgDic.Add(PDBErrorCode.PdbE_OPENED, "对象已打开");
        errMsgDic.Add(PDBErrorCode.PdbE_NOMEM, "内存申请失败");
        errMsgDic.Add(PDBErrorCode.PdbE_FILE_EXIST, "文件已存在");
        errMsgDic.Add(PDBErrorCode.PdbE_FILE_READONLY, "只读文件");
        errMsgDic.Add(PDBErrorCode.PdbE_PATH_TOO_LONG, "路径太长");
        errMsgDic.Add(PDBErrorCode.PdbE_TABLE_CFG_ERROR, "表配置文件错误");
        errMsgDic.Add(PDBErrorCode.PdbE_USER_CFG_ERROR, "用户配置文件错误");
        errMsgDic.Add(PDBErrorCode.PdbE_DEVID_FILE_ERROR, "设备文件错误");
        errMsgDic.Add(PDBErrorCode.PdbE_IDX_FILE_ERROR, "索引文件错误");
        errMsgDic.Add(PDBErrorCode.PdbE_FILE_NOT_FOUND, "文件不存在");
        errMsgDic.Add(PDBErrorCode.PdbE_DATA_LOG_ERROR, "日志文件错误");
        errMsgDic.Add(PDBErrorCode.PdbE_END_OF_DATALOG, "数据日志读取完成");
        errMsgDic.Add(PDBErrorCode.PdbE_PATH_NOT_FOUND, "目录不存在");
        errMsgDic.Add(PDBErrorCode.PdbE_DATA_LOG_VER_ERROR, "数据日志版本错误");
        errMsgDic.Add(PDBErrorCode.PdbE_DATA_FILECODE_ERROR, "数据日志文件编号错误");

        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_FILE_NAME, "无效的文件名");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_PARAM, "无效的参数");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_HANDLE, "无效的句柄");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_USER_NAME, "无效的用户名");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_USER_ROLE, "无效的用户角色");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_INT_VAL, "无效的整型值");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_DOUBLE_VAL, "无效的浮点值");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_BLOB_VAL, "无效的Blob值");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_TSTAMP_VAL, "无效的时间戳值");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_DATETIME_VAL, "无效的DateTime值");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_TABLE_NAME, "无效的表名");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_DEVID, "无效的设备ID");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_DEVNAME, "无效的设备名");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_DEVEXPAND, "无效的设备扩展信息");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_FIELD_NAME, "无效的字段名");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_FIELD_TYPE, "无效的字段类型");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_DEVID_FIELD, "无效的设备ID字段");
        errMsgDic.Add(PDBErrorCode.PdbE_INVALID_TSTAMP_FIELD, "无效的时间戳字段");
        errMsgDic.Add(PDBErrorCode.PdbE_OBJECT_INITIALIZED, "对象已初始化");

        //记录和数据页相关
        errMsgDic.Add(PDBErrorCode.PdbE_RECORD_FAIL, "错误的记录");
        errMsgDic.Add(PDBErrorCode.PdbE_RECORD_EXIST, "记录已存在");
        errMsgDic.Add(PDBErrorCode.PdbE_RECORD_TOO_LONG, "记录太长");
        errMsgDic.Add(PDBErrorCode.PdbE_PAGE_FILL, "数据页满");
        errMsgDic.Add(PDBErrorCode.PdbE_PAGE_ERROR, "数据页错误");
        errMsgDic.Add(PDBErrorCode.PdbE_VALUE_MISMATCH, "值类型不匹配");
        errMsgDic.Add(PDBErrorCode.PdbE_NULL_VALUE, "空值");
        errMsgDic.Add(PDBErrorCode.PdbE_TSTAMP_DISORDER, "tstamp乱序");
        errMsgDic.Add(PDBErrorCode.PdbE_NODATA, "缺少数据");
        errMsgDic.Add(PDBErrorCode.PdbE_COMPRESS_ERROR, "压缩失败");

        //表和字段相关
        errMsgDic.Add(PDBErrorCode.PdbE_FIELD_NOT_FOUND, "字段不存在");
        errMsgDic.Add(PDBErrorCode.PdbE_FIELD_NAME_EXIST, "字段名存在");
        errMsgDic.Add(PDBErrorCode.PdbE_TABLE_NOT_FOUND, "表不存在");
        errMsgDic.Add(PDBErrorCode.PdbE_TABLE_FIELD_TOO_LESS, "表字段太少");
        errMsgDic.Add(PDBErrorCode.PdbE_TABLE_FIELD_TOO_MANY, "表字段太多");
        errMsgDic.Add(PDBErrorCode.PdbE_TABLE_PART_EXIST, "数据块已经存在");
        errMsgDic.Add(PDBErrorCode.PdbE_TABLE_CAPACITY_FULL, "表容量已满");
        errMsgDic.Add(PDBErrorCode.PdbE_TABLE_EXIST, "表已存在");
        errMsgDic.Add(PDBErrorCode.PdbE_DATA_FILE_IN_ACTIVE, "活跃的数据文件不能删除或分离");
        errMsgDic.Add(PDBErrorCode.PdbE_DATA_FILE_NOT_FOUND, "数据文件不存在");


        //网络及任务相关
        errMsgDic.Add(PDBErrorCode.PdbE_NET_ERROR, "网络错误");
        errMsgDic.Add(PDBErrorCode.PdbE_CONN_TOO_MANY, "客户段连接超过上限");
        errMsgDic.Add(PDBErrorCode.PdbE_PASSWORD_ERROR, "密码错误");
        errMsgDic.Add(PDBErrorCode.PdbE_PACKET_ERROR, "报文错误");
        errMsgDic.Add(PDBErrorCode.PdbE_OPERATION_DENIED, "操作被拒绝");
        errMsgDic.Add(PDBErrorCode.PdbE_TASK_CANCEL, "操作被取消");
        errMsgDic.Add(PDBErrorCode.PdbE_TASK_STATE_ERROR, "任务状态错误");
        errMsgDic.Add(PDBErrorCode.PdbE_RETRY, "稍后重试");
        errMsgDic.Add(PDBErrorCode.PdbE_QUERY_TIME_OUT, "查询超时");
        errMsgDic.Add(PDBErrorCode.PdbE_NOT_LOGIN, "未登录");
        errMsgDic.Add(PDBErrorCode.PdbE_INSERT_PART_ERROR, "部分插入失败");

        //SQL相关
        errMsgDic.Add(PDBErrorCode.PdbE_SQL_LOST_ALIAS, "必须指定别名");
        errMsgDic.Add(PDBErrorCode.PdbE_SQL_GROUP_ERROR, "SQL分组错误");
        errMsgDic.Add(PDBErrorCode.PdbE_SQL_GROUP_LOST_BEGIN_TSTAMP, "SQL分组缺少起始时间");
        errMsgDic.Add(PDBErrorCode.PdbE_SQL_ERROR, "SQL语句错误");
        errMsgDic.Add(PDBErrorCode.PdbE_SQL_CONDITION_EXPR_ERROR, "SQL条件表达式错误");
        errMsgDic.Add(PDBErrorCode.PdbE_SQL_RESULT_ERROR, "SQL结果集错误");
        errMsgDic.Add(PDBErrorCode.PdbE_SQL_RESULT_TOO_SMALL, "SQL结果集太小");
        errMsgDic.Add(PDBErrorCode.PdbE_SQL_RESULT_TOO_LARGE, "SQL结果集太大");
        errMsgDic.Add(PDBErrorCode.PdbE_SQL_LIMIT_ERROR, "SQL Limit错误");
        errMsgDic.Add(PDBErrorCode.PdbE_SQL_NOT_QUERY, "不是查询SQL");

        //设备、索引相关
        errMsgDic.Add(PDBErrorCode.PdbE_RESLT_FULL, "结果集已满");
        errMsgDic.Add(PDBErrorCode.PdbE_DEVID_EXISTS, "设备ID已存在");
        errMsgDic.Add(PDBErrorCode.PdbE_DEV_CAPACITY_FULL, "设备容量已满");
        errMsgDic.Add(PDBErrorCode.PdbE_USER_EXIST, "用户已存在");
        errMsgDic.Add(PDBErrorCode.PdbE_USER_NOT_FOUND, "用户不存在");
        errMsgDic.Add(PDBErrorCode.PdbE_IDX_NOT_FOUND, "索引未找到");
        errMsgDic.Add(PDBErrorCode.PdbE_DEV_NOT_FOUND, "设备未找到");
        
      }

      if (errMsgDic.ContainsKey(errCode))
      {
        return errMsgDic[errCode];
      }

      return Convert.ToInt32(errCode).ToString() + ":未知的错误码";
    }

  }
}
