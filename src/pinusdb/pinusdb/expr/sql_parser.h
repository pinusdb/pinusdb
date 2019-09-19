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
#include "expr/expr_item.h"
#include "expr/parse.h"
#include "pdb.h"
#include <string>

class QueryParam
{
public:
  ExprList* pSelList_;
  std::string srcTab_;
  ExprItem* pWhere_;
  GroupOpt* pGroup_;
  OrderByOpt* pOrderBy_;
  LimitOpt* pLimit_;

  bool IsQueryRaw() const
  {
    const std::vector<ExprItem*>& colItemVec = pSelList_->GetExprList();
    for (auto colIt = colItemVec.begin(); colIt != colItemVec.end(); colIt++)
    {
      if ((*colIt)->GetOp() != TK_ID && (*colIt)->GetOp() != TK_STAR)
      {
        return false;
      }
    }

    return true;
  }

  QueryParam()
  {
    pSelList_ = nullptr;
    pWhere_ = nullptr;
    pGroup_ = nullptr;
    pOrderBy_ = nullptr;
    pLimit_ = nullptr;
  }

  ~QueryParam()
  {
    if (pSelList_ != nullptr)
      delete pSelList_;

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
  std::string tabName_;
  ExprItem* pWhere_;

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

class DropTableParam
{
public:
  std::string tabName_;
};

class AttachTableParam
{
public:
  std::string tabName_;
};

class DetachTableParam
{
public:
  std::string tabName_;
};

class AttachFileParam
{
public:
  std::string tabName_;
  std::string dateStr_;
  std::string fileType_;

  AttachFileParam() {}
  ~AttachFileParam() {}
};

class DetachFileParam
{
public:
  std::string tabName_;
  std::string dateStr_;

  DetachFileParam() {}
  ~DetachFileParam() {}
};

class DropFileParam
{
public:
  std::string tabName_;
  std::string dateStr_;

  DropFileParam() {}
  ~DropFileParam() {}
};

class InsertParam
{
public:
  std::string tabName_;
  ExprList* pColList_;
  ExprList* pValList_;

  InsertParam()
  {
    this->pColList_ = nullptr;
    this->pValList_ = nullptr;
  }

  ~InsertParam()
  {
    if (this->pColList_ != nullptr)
      delete pColList_;

    if (this->pValList_ != nullptr)
      delete pValList_;
  }
};

class CreateTableParam
{
public:
  std::string tabName_;
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

class AddUserParam
{
public:
  std::string userName_;
  std::string pwd_;
};

class ChangePwdParam
{
public:
  std::string userName_;
  std::string newPwd_;
};

class ChangeRoleParam
{
public:
  std::string userName_;
  std::string roleName_;
};

class DropUserParam
{
public:
  std::string userName_;
};

class SQLParser
{
public:
  SQLParser();
  ~SQLParser();

  void SetQuery(ExprList* pSelList, Token* pSrcTab, ExprItem* pWhere, GroupOpt* pGroup, OrderByOpt* pOrderBy, LimitOpt* pLimit);
  void SetInsert(Token* pTabToken, ExprList* pColList, ExprList* pValList);
  void SetDelete(Token* pTabToken, ExprItem* pWhere);
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

  int32_t GetCmdType() const;

  const QueryParam* GetQueryParam() const;
  const DeleteParam* GetDeleteParam() const { return &deleteParam_; }
  const DropTableParam* GetDropTableParam() const;
  const InsertParam* GetInsertParam() const;
  const CreateTableParam* GetCreateTableParam() const;
  const AddUserParam* GetAddUserParam() const;
  const ChangePwdParam* GetChangePwdParam() const;
  const ChangeRoleParam* GetChangeRoleParam() const;
  const DropUserParam* GetDropUserParam() const;

  const AttachTableParam* GetAttachTableParam() const;
  const DetachTableParam* GetDetachTableParam() const;
  const AttachFileParam* GetAttachFileParam() const { return &attachFileParam_; }
  const DetachFileParam* GetDetachFileParam() const { return &detachFileParam_; }
  const DropFileParam* GetDropFileParam() const { return &dropFileParam_; }
  
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

  QueryParam queryParam_;
  DeleteParam deleteParam_;
  DropTableParam dropTableParam_;
  InsertParam insertParam_;
  CreateTableParam createTableParam_;
  AddUserParam addUserParam_;
  ChangePwdParam changePwdParam_;
  ChangeRoleParam changeRoleParam_;
  DropUserParam dropUserParam_;

  AttachTableParam attachTabParam_;
  DetachTableParam detachTabParam_;
  AttachFileParam attachFileParam_;
  DetachFileParam detachFileParam_;
  DropFileParam   dropFileParam_;
};


