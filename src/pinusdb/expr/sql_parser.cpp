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
#include "expr/expr_value.h"
#include "expr/target_list.h"
#include "expr/group_opt.h"
#include "util/string_tool.h"
#include <vector>
#include <string>
#include <stack>

SQLParser::SQLParser()
{
  this->cmdType_ = CmdType::CT_None;
  this->isError_ = false;

  pQueryParam_ = nullptr;
  pDeleteParam_ = nullptr;
  pInsertParam_ = nullptr;
  pCreateTableParam_ = nullptr;
  pUserParam_ = nullptr;
  pDataFileParam_ = nullptr;
}

SQLParser::~SQLParser()
{
  if (pQueryParam_ != nullptr) delete pQueryParam_;
  if (pDeleteParam_ != nullptr) delete pDeleteParam_;
  if (pInsertParam_ != nullptr) delete pInsertParam_;
  if (pCreateTableParam_ != nullptr) delete pCreateTableParam_;
  if (pUserParam_ != nullptr) delete pUserParam_;
  if (pDataFileParam_ != nullptr) delete pDataFileParam_;
}

void SQLParser::SetQuery(TargetList* pTagList, Token* pSrcTab, ExprValue* pWhere,
  GroupOpt* pGroup, OrderByOpt* pOrderBy, LimitOpt* pLimit)
{
  cmdType_ = CmdType::CT_Select;
  this->pQueryParam_ = new QueryParam();

  this->pQueryParam_->pTagList_ = pTagList;
  if (pSrcTab != nullptr)
    this->tableName_ = std::string(pSrcTab->str_, pSrcTab->len_);
  else
    this->tableName_ = "";

  this->pQueryParam_->pWhere_ = pWhere;
  this->pQueryParam_->pGroup_ = pGroup;
  this->pQueryParam_->pOrderBy_ = pOrderBy;
  this->pQueryParam_->pLimit_ = pLimit;
}


void SQLParser::SetInsert(Token* pTabName, TargetList* pTagList, RecordList* pRecList)
{
  cmdType_ = CmdType::CT_Insert;
  this->pInsertParam_ = new InsertParam();

  this->tableName_ = std::string(pTabName->str_, pTabName->len_);
  this->pInsertParam_->pTagList_ = pTagList;
  this->pInsertParam_->pValRecList_ = pRecList;
}

void SQLParser::SetDelete(Token* pTabToken, ExprValue* pWhere)
{
  cmdType_ = CmdType::CT_Delete;
  this->pDeleteParam_ = new DeleteParam();

  this->tableName_ = std::string(pTabToken->str_, pTabToken->len_);
  this->pDeleteParam_->pWhere_ = pWhere;
}

void SQLParser::SetCreateTable(Token* pTabName, ColumnList* pColList)
{
  cmdType_ = CmdType::CT_CreateTable;
  this->pCreateTableParam_ = new CreateTableParam();
  
  this->tableName_ = std::string(pTabName->str_, pTabName->len_);
  this->pCreateTableParam_->pColList_ = pColList;
}

void SQLParser::SetAlterTable(Token* pTabName, ColumnList* pColList)
{
  cmdType_ = CmdType::CT_AlterTable;
  this->pCreateTableParam_ = new CreateTableParam();

  this->tableName_ = std::string(pTabName->str_, pTabName->len_);
  this->pCreateTableParam_->pColList_ = pColList;
}

void SQLParser::SetDropTable(Token* pTabName)
{
  cmdType_ = CmdType::CT_DropTable;
  this->tableName_ = std::string(pTabName->str_, pTabName->len_);
}

void SQLParser::SetAddUser(Token* pNameToken, Token* pPwdToken)
{
  cmdType_ = CmdType::CT_AddUser;
  this->pUserParam_ = new UserParam();

  this->pUserParam_->userName_ = std::string(pNameToken->str_, pNameToken->len_);
  this->pUserParam_->pwd_ = std::string(pPwdToken->str_, pPwdToken->len_);

}

void SQLParser::SetChangePwd(Token* pNameToken, Token* pPwdToken)
{
  cmdType_ = CmdType::CT_ChangePwd;
  this->pUserParam_ = new UserParam();

  this->pUserParam_->userName_ = std::string(pNameToken->str_, pNameToken->len_);
  this->pUserParam_->pwd_ = std::string(pPwdToken->str_, pPwdToken->len_);
}

void SQLParser::SetChangeRole(Token* pNameToken, Token* pRoleToken)
{
  cmdType_ = CmdType::CT_ChangeRole;
  this->pUserParam_ = new UserParam();

  this->pUserParam_->userName_ = std::string(pNameToken->str_, pNameToken->len_);
  this->pUserParam_->roleName_ = std::string(pRoleToken->str_, pRoleToken->len_);
}

void SQLParser::SetDropUser(Token* pNameToken)
{
  cmdType_ = CmdType::CT_DropUser;
  this->pUserParam_ = new UserParam();

  this->pUserParam_->userName_ = std::string(pNameToken->str_, pNameToken->len_);
}


void SQLParser::SetAttachTable(Token* pTabToken)
{
  cmdType_ = CmdType::CT_AttachTable;

  this->tableName_ = std::string(pTabToken->str_, pTabToken->len_);
}

void SQLParser::SetDetachTable(Token* pTabToken)
{
  cmdType_ = CmdType::CT_DetachTable;
  
  this->tableName_ = std::string(pTabToken->str_, pTabToken->len_);
}

void SQLParser::SetAttachFile(Token* pTabToken, Token* pDate, Token* pType)
{
  cmdType_ = CmdType::CT_AttachFile;
  this->pDataFileParam_ = new DataFileParam();
  this->tableName_ = std::string(pTabToken->str_, pTabToken->len_);
  this->pDataFileParam_->dateStr_ = std::string(pDate->str_, pDate->len_);
  this->pDataFileParam_->fileType_ = std::string(pType->str_, pType->len_);
}

void SQLParser::SetDetachFile(Token* pTabToken, Token* pDate)
{
  cmdType_ = CmdType::CT_DetachFile;
  this->pDataFileParam_ = new DataFileParam();
  this->tableName_ = std::string(pTabToken->str_, pTabToken->len_);
  this->pDataFileParam_->dateStr_ = std::string(pDate->str_, pDate->len_);
}

void SQLParser::SetDropFile(Token* pTabToken, Token* pDate)
{
  cmdType_ = CmdType::CT_DropFile;
  this->pDataFileParam_ = new DataFileParam();
  this->tableName_ = std::string(pTabToken->str_, pTabToken->len_);
  this->pDataFileParam_->dateStr_ = std::string(pDate->str_, pDate->len_);
}

bool SQLParser::GetError()
{
  return isError_;
}
void SQLParser::SetError()
{
  isError_ = true;
}



