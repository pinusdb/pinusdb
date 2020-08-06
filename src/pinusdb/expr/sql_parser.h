/*
* Copyright (c) 2019 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; If not, see <http://www.gnu.org/licenses>
*/

#pragma once

#include "expr/pdb_db_int.h"
#include "expr/column_item.h"
#include "expr/group_opt.h"
#include "expr/orderby_opt.h"
#include "expr/limit_opt.h"
#include "expr/expr_value.h"
#include "expr/target_list.h"
#include "expr/parse.h"
#include "expr/record_list.h"
#include "table/table_info.h"
#include "pdb.h"
#include <string>

class QueryParam
{
public:
  TargetList* pTagList_;
  ExprValue* pWhere_;
  GroupOpt* pGroup_;
  OrderByOpt* pOrderBy_;
  LimitOpt* pLimit_;

  QueryParam()
  {
    pTagList_ = nullptr;
    pWhere_ = nullptr;
    pGroup_ = nullptr;
    pOrderBy_ = nullptr;
    pLimit_ = nullptr;
  }

  ~QueryParam()
  {
    if (pTagList_ != nullptr)
      delete pTagList_;

    if (pWhere_ != nullptr)
      delete pWhere_;

    if (pGroup_ != nullptr)
      delete pGroup_;

    if (pOrderBy_ != nullptr)
      delete pOrderBy_;

    if (pLimit_ != nullptr)
      delete pLimit_;
  }
};

class DeleteParam
{
public:
  ExprValue* pWhere_;

  DeleteParam()
  {
    pWhere_ = nullptr;
  }

  ~DeleteParam()
  {
    if (pWhere_ != nullptr)
      delete pWhere_;
  }

};

class DataFileParam
{
public:
  std::string dateStr_;
  std::string fileType_;

  DataFileParam() {}
  ~DataFileParam() {}
};

class InsertParam
{
public:
  TargetList* pTagList_;
  RecordList* pValRecList_;

  InsertParam()
  {
    this->pTagList_ = nullptr;
    this->pValRecList_ = nullptr;
  }

  ~InsertParam()
  {
    if (this->pTagList_ != nullptr)
      delete pTagList_;

    if (this->pValRecList_ != nullptr)
      delete pValRecList_;
  }
};

class CreateTableParam
{
public:
  ColumnList* pColList_;

  CreateTableParam()
  {
    this->pColList_ = nullptr;
  }

  ~CreateTableParam()
  {
    if (this->pColList_ != nullptr)
      delete pColList_;
  }
};

class UserParam
{
public:
  std::string userName_;
  std::string pwd_;
  std::string roleName_;
};

class SQLParser
{
public:
  SQLParser();
  ~SQLParser();

  void SetQuery(TargetList* pTagList, Token* pSrcTab, ExprValue* pWhere,
    GroupOpt* pGroup, OrderByOpt* pOrderBy, LimitOpt* pLimit);
  void SetInsert(Token* pTabName, TargetList* pTagList, RecordList* pRecList);
  void SetDelete(Token* pTabToken, ExprValue* pWhere);
  void SetCreateTable(Token* pTabName, ColumnList* pColList);
  void SetAlterTable(Token* pTabName, ColumnList* pColList);
  void SetDropTable(Token* pTabName);
  void SetAddUser(Token* pNameToken, Token* pPwdToken);
  void SetChangePwd(Token* pNameToken, Token* pPwdToken);
  void SetChangeRole(Token* pNameToken, Token* pRoleToken);
  void SetDropUser(Token* pNameToken);

  void SetAttachTable(Token* pTabToken);
  void SetDetachTable(Token* pTabToken);
  void SetAttachFile(Token* pTabToken, Token* pDate, Token* pType);
  void SetDetachFile(Token* pTabToken, Token* pDate);
  void SetDropFile(Token* pTabToken, Token* pDate);

  bool GetError();
  void SetError();

  int32_t GetCmdType() const { return cmdType_; }
  const char* GetTableName() const { return tableName_.c_str(); }

  const QueryParam* GetQueryParam() const { return pQueryParam_; }
  const DeleteParam* GetDeleteParam() const { return pDeleteParam_; }
  const InsertParam* GetInsertParam() const { return pInsertParam_; }
  const CreateTableParam* GetCreateTableParam() const { return pCreateTableParam_; }
  const UserParam* GetUserParam() const { return pUserParam_; }
  const DataFileParam* GetDataFileParam() const { return pDataFileParam_; }

  enum CmdType
  {
    CT_None = 0,            // 空
    CT_Select = 1,          // 查询数据
    CT_Delete = 2,
    CT_DropTable = 3,       // 删除表
    CT_AttachTable = 4,     // 附加表
    CT_DetachTable = 5,     // 分离表
    CT_AttachFile = 6,      // 附加文件
    CT_DetachFile = 7,      // 分离文件
    CT_DropFile = 8,        // 删除文件
    CT_Insert = 9,          // 插入
    CT_CreateTable = 10,     // 创建表
    CT_AddUser = 11,         // 添加用户
    CT_ChangePwd = 12,       // 修改密码
    CT_ChangeRole = 13,      // 修改权限
    CT_DropUser = 14,        // 删除用户
    CT_AlterTable = 15,      // 修改表结构
  };

private:
  int32_t cmdType_;
  bool isError_;
  std::string tableName_;

  QueryParam* pQueryParam_;
  DeleteParam* pDeleteParam_;
  InsertParam* pInsertParam_;
  CreateTableParam* pCreateTableParam_;
  UserParam* pUserParam_;

  DataFileParam* pDataFileParam_;
};


