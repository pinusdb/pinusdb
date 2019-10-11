#include "pdb_error_msg.h"

PdbErrorMsg* PdbErrorMsg::instance_ = nullptr;

PdbErrorMsg* PdbErrorMsg::GetInstance()
  {
    if (instance_ == nullptr)
    {
      instance_ = new PdbErrorMsg();
    }

    return instance_;
  }

PdbErrorMsg::~PdbErrorMsg()
{

}

const char* PdbErrorMsg::GetMsgInfo(PdbErr_t errCode)
{
  auto errIt = errMsgMap_.find(errCode);

  if (errIt != errMsgMap_.end())
  {
    return errIt->second.c_str();
  }

  return unknownMsg_.c_str();
}


PdbErrorMsg::PdbErrorMsg()
{
  unknownMsg_ = "未知的错误码";

  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_OK, "成功"));

  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_IOERR, "I/O失败"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_OPENED, "对象已打开"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_NOMEM, "内存申请失败"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_FILE_EXIST, "文件已存在"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_FILE_READONLY, "只读文件"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_PATH_TOO_LONG, "路径太长"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_TABLE_CFG_ERROR, "表配置文件错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_USER_CFG_ERROR, "用户配置文件错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_DEVID_FILE_ERROR, "设备文件错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_IDX_FILE_ERROR, "索引文件错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_FILE_NOT_FOUND, "文件不存在"));

  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_FILE_NAME, "无效的文件名"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_PARAM, "无效的参数"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_HANDLE, "无效的句柄"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_USER_NAME, "无效的用户名"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_USER_ROLE, "无效的用户角色"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_INT_VAL, "无效的整型值"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_DOUBLE_VAL, "无效的浮点值"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_BLOB_VAL, "无效的Blob值"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_TSTAMP_VAL, "无效的时间戳值"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_DATETIME_VAL, "无效的DateTime值"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_TABLE_NAME, "无效的表名"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_DEVID, "无效的设备ID"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_DEVNAME, "无效的设备名"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_DEVEXPAND, "无效的设备扩展信息"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_FIELD_NAME, "无效的字段名"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_FIELD_TYPE, "无效的字段类型"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_DEVID_FIELD, "无效的设备ID字段"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_INVALID_TSTAMP_FIELD, "无效的时间戳字段"));


  //记录和数据页相关
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_RECORD_FAIL, "错误的记录"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_RECORD_EXIST, "记录已存在"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_RECORD_TOO_LONG, "记录太长"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_PAGE_FILL, "数据页满"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_PAGE_ERROR, "数据页错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_VALUE_MISMATCH, "值类型不匹配"));

  //表和字段相关
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_FIELD_NOT_FOUND, "字段不存在"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_FIELD_NAME_EXIST, "字段名存在"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_TABLE_NOT_FOUND, "表不存在"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_TABLE_FIELD_TOO_LESS, "表字段太少"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_TABLE_FIELD_TOO_MANY, "表字段太多"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_TABLE_PART_EXIST, "数据块已经存在"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_TABLE_CAPACITY_FULL, "表容量已满"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_TABLE_EXIST, "表已存在"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_DATA_FILE_IN_ACTIVE, "活跃的数据文件不能删除或分离"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_DATA_FILE_NOT_FOUND, "数据文件不存在"));


  //网络及任务相关
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_NET_ERROR, "网络错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_CONN_TOO_MANY, "客户段连接超过上限"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_PASSWORD_ERROR, "密码错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_PACKET_ERROR, "报文错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_OPERATION_DENIED, "操作被拒绝"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_TASK_CANCEL, "操作被取消"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_TASK_STATE_ERROR, "任务状态错误"));

  //SQL相关
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_SQL_LOST_ALIAS, "必须指定别名"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_SQL_GROUP_ERROR, "SQL分组错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_SQL_GROUP_LOST_BEGIN_TSTAMP, "SQL分组缺少起始时间"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_SQL_ERROR, "SQL语句错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_SQL_CONDITION_EXPR_ERROR, "SQL条件表达式错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_SQL_RESULT_ERROR, "SQL结果集错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_SQL_RESULT_TOO_SMALL, "SQL结果集太小"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_SQL_RESULT_TOO_LARGE, "SQL结果集太大"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_SQL_LIMIT_ERROR, "SQL Limit错误"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_SQL_NOT_QUERY, "不是查询SQL"));

  //设备、索引相关
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_RESLT_FULL, "结果集已满"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_DEVID_EXISTS, "设备ID已存在"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_DEV_CAPACITY_FULL, "设备容量已满"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_USER_EXIST, "用户已存在"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_USER_NOT_FOUND, "用户不存在"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_IDX_NOT_FOUND, "索引未找到"));
  errMsgMap_.insert(std::make_pair<PdbErr_t, std::string>(PdbE_DEV_NOT_FOUND, "设备未找到"));
}

