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

#include "expr/group_opt.h"
#include "expr/expr_value.h"
#include "expr/parse.h"
#include "util/string_tool.h"
#include "pdb_error.h"
#include "internal.h"
#include <string>

#define SYS_TAB_TABNAME_FIELD_NAME   "tabname"

GroupOpt::GroupOpt(Token* pToken1)
{
  this->valid_ = true;
  this->isTableName_ = false;
  this->isDevId_ = false;
  this->isTStamp_ = false;
  this->tVal_ = 0;

  if (StringTool::ComparyNoCase(pToken1->str_, pToken1->len_, DEVID_FIELD_NAME, (sizeof(DEVID_FIELD_NAME) - 1)))
    this->isDevId_ = true;
  else if (StringTool::ComparyNoCase(pToken1->str_, pToken1->len_, SYS_TAB_TABNAME_FIELD_NAME, ((sizeof(SYS_TAB_TABNAME_FIELD_NAME) - 1))))
    this->isTableName_ = true;
  else
    this->valid_ = false;
}
GroupOpt::GroupOpt(Token* pToken1, ExprValue* pTimeVal)
{
  DBVal tmpVal;
  this->valid_ = false;
  this->isTableName_ = false;
  this->isDevId_ = false;
  this->isTStamp_ = true;
  this->tVal_ = 0;

  do {
    if (pToken1 == nullptr || pTimeVal == nullptr)
      break;

    if (pTimeVal->GetValueType() != TK_TIMEVAL)
      break;

    tmpVal = pTimeVal->GetValue();
    if (DBVAL_IS_INT64(&tmpVal))
    {
      tVal_ = DBVAL_GET_INT64(&tmpVal);
      if (tVal_ <= 0)
        break;

      this->valid_ = true;
    }

  } while (false);
  
}
GroupOpt::~GroupOpt()
{

}

bool GroupOpt::Valid() const
{
  return valid_;
}
bool GroupOpt::IsTableNameGroup() const
{
  return valid_ ? isTableName_ : false;
}
bool GroupOpt::IsDevIdGroup() const
{
  return valid_ ? isDevId_ : false;
}
bool GroupOpt::IsTStampGroup() const
{
  return valid_ ? isTStamp_ : false;
}
PdbErr_t GroupOpt::GetTStampStep(int64_t& timeStampStep) const
{
  if (valid_ && isTStamp_)
  {
    timeStampStep = tVal_;
    return PdbE_OK;
  }

  return PdbE_INVALID_PARAM;
}

GroupOpt* GroupOpt::MakeGroupOpt(Token* pToken1)
{
  return new GroupOpt(pToken1);
}
GroupOpt* GroupOpt::MakeGroupOpt(Token* pToken1, ExprValue* pTimeVal)
{
  return new GroupOpt(pToken1, pTimeVal);
}
void GroupOpt::FreeGroupOpt(GroupOpt* pGroupOpt)
{
  delete pGroupOpt;
}


