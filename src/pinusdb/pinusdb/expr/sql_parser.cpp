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

#include "expr/sql_parser.h"
#include "expr/parse.h"
#include "expr/pdb_db_int.h"
#include "expr/expr_item.h"
#include "expr/group_opt.h"
#include "util/string_tool.h"
#include <vector>
#include <string>
#include <stack>

SQLParser::SQLParser()
{
  this->cmdType_ = CmdType::CT_None;
  this->isError_ = false;

}
SQLParser::~SQLParser()
{

}

void SQLParser::SetQuery(ExprList* pSelList, Token* pSrcTab, ExprItem* pWhere, 
  GroupOpt* pGroup, OrderByOpt* pOrderBy, LimitOpt* pLimit)
{
  cmdType_ = CmdType::CT_Select;

  this->queryParam_.pSelList_ = pSelList;

  this->queryParam_.srcTab_ = std::string(pSrcTab->str_, pSrcTab->len_);

  this->queryParam_.pWhere_ = pWhere;
  this->queryParam_.pGroup_ = pGroup;
  this->queryParam_.pOrderBy_ = pOrderBy;
  this->queryParam_.pLimit_ = pLimit;
}


void SQLParser::SetInsert(Token* pTabToken, 
  ExprList* pColList, ExprList* pValList)
{
  cmdType_ = CmdType::CT_Insert;

  this->insertParam_.tabName_ = std::string(pTabToken->str_, pTabToken->len_);

  this->insertParam_.pColList_ = pColList;
  this->insertParam_.pValList_ = pValList;

}

void SQLParser::SetDelete(Token* pTabToken, ExprItem* pWhere)
{
  cmdType_ = CmdType::CT_Delete;

  this->deleteParam_.tabName_ = std::string(pTabToken->str_, pTabToken->len_);
  this->deleteParam_.pWhere_ = pWhere;
}

void SQLParser::SetCreateTable(Token* pTabName, ColumnList* pColList)
{
  cmdType_ = CmdType::CT_CreateTable;
  
  this->createTableParam_.tabName_ = std::string(pTabName->str_, pTabName->len_);
  this->createTableParam_.pColList_ = pColList;
}

void SQLParser::SetDropTable(Token* pTabName)
{
  cmdType_ = CmdType::CT_DropTable;

  this->dropTableParam_.tabName_ = std::string(pTabName->str_, pTabName->len_);
}

void SQLParser::SetAddUser(Token* pNameToken, Token* pPwdToken)
{
  cmdType_ = CmdType::CT_AddUser;

  this->addUserParam_.userName_ = std::string(pNameToken->str_, pNameToken->len_);
  this->addUserParam_.pwd_ = std::string(pPwdToken->str_, pPwdToken->len_);

}

void SQLParser::SetChangePwd(Token* pNameToken, Token* pPwdToken)
{
  cmdType_ = CmdType::CT_ChangePwd;

  this->changePwdParam_.userName_ = std::string(pNameToken->str_, pNameToken->len_);
  this->changePwdParam_.newPwd_ = std::string(pPwdToken->str_, pPwdToken->len_);
}

void SQLParser::SetChangeRole(Token* pNameToken, Token* pRoleToken)
{
  cmdType_ = CmdType::CT_ChangeRole;

  this->changeRoleParam_.userName_ = std::string(pNameToken->str_, pNameToken->len_);
  this->changeRoleParam_.roleName_ = std::string(pRoleToken->str_, pRoleToken->len_);
}

void SQLParser::SetDropUser(Token* pNameToken)
{
  cmdType_ = CmdType::CT_DropUser;

  dropUserParam_.userName_ = std::string(pNameToken->str_, pNameToken->len_);
}


void SQLParser::SetAttachTable(Token* pTabToken)
{
  cmdType_ = CmdType::CT_AttachTable;

  attachTabParam_.tabName_ = std::string(pTabToken->str_, pTabToken->len_);
}

void SQLParser::SetDetachTable(Token* pTabToken)
{
  cmdType_ = CmdType::CT_DetachTable;
  detachTabParam_.tabName_ = std::string(pTabToken->str_, pTabToken->len_);
}

void SQLParser::SetAttachFile(Token* pTabToken, Token* pDate, Token* pType)
{
  cmdType_ = CmdType::CT_AttachFile;
  attachFileParam_.tabName_ = std::string(pTabToken->str_, pTabToken->len_);
  attachFileParam_.dateStr_ = std::string(pDate->str_, pDate->len_);
  attachFileParam_.fileType_ = std::string(pType->str_, pType->len_);
}

void SQLParser::SetDetachFile(Token* pTabToken, Token* pDate)
{
  cmdType_ = CmdType::CT_DetachFile;
  detachFileParam_.tabName_ = std::string(pTabToken->str_, pTabToken->len_);
  detachFileParam_.dateStr_ = std::string(pDate->str_, pDate->len_);
}

void SQLParser::SetDropFile(Token* pTabToken, Token* pDate)
{
  cmdType_ = CmdType::CT_DropFile;
  dropFileParam_.tabName_ = std::string(pTabToken->str_, pTabToken->len_);
  dropFileParam_.dateStr_ = std::string(pDate->str_, pDate->len_);
}


bool SQLParser::GetError()
{
  return isError_;
}
void SQLParser::SetError()
{
  isError_ = true;
}

const QueryParam* SQLParser::GetQueryParam() const
{
  return &queryParam_;
}

const DropTableParam* SQLParser::GetDropTableParam() const
{
  return &dropTableParam_;
}
const InsertParam* SQLParser::GetInsertParam() const
{
  return &insertParam_;
}
const CreateTableParam* SQLParser::GetCreateTableParam() const
{
  return &createTableParam_;
}
const AddUserParam* SQLParser::GetAddUserParam() const
{
  return &addUserParam_;
}

const ChangePwdParam* SQLParser::GetChangePwdParam() const
{
  return &changePwdParam_;
}
const ChangeRoleParam* SQLParser::GetChangeRoleParam() const
{
  return &changeRoleParam_;
}
const DropUserParam* SQLParser::GetDropUserParam() const
{
  return &dropUserParam_;
}

const AttachTableParam* SQLParser::GetAttachTableParam() const
{
  return &attachTabParam_;
}

const DetachTableParam* SQLParser::GetDetachTableParam() const
{
  return &detachTabParam_;
}

int32_t SQLParser::GetCmdType() const
{
  return cmdType_;
}


