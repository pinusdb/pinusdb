#include "expr/group_opt.h"
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
GroupOpt::GroupOpt(Token* pToken1, Token* pToken2, Token* pToken3)
{
  this->valid_ = false;
  this->isTableName_ = false;
  this->isDevId_ = false;
  this->isTStamp_ = true;
  this->tVal_ = 0;

  if (StringTool::ComparyNoCase(pToken1->str_, pToken1->len_, TSTAMP_FIELD_NAME, (sizeof(TSTAMP_FIELD_NAME) - 1))
    && StringTool::StrToInt64(pToken2->str_, pToken2->len_, &tVal_))
  {
    if (tVal_ > 0)
    {
      if (pToken3->len_ == 1)
      {
        this->valid_ = true;
        switch (*pToken3->str_)
        {
        case 's':
        case 'S':
          this->tVal_ *= 1000;                    // Ãë
          break;
        case 'm':
        case 'M':
          this->tVal_ *= (60 * 1000);             // ·Ö
          break;
        case 'h':
        case 'H':
          this->tVal_ *= (60 * 60 * 1000);        // Ê±
          break;
        case 'd':
        case 'D':
          this->tVal_ *= (24 * 60 * 60 * 1000);   // Ìì
          break;
        default:
          this->valid_ = false;
        }
      }
    }
  }

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
GroupOpt* GroupOpt::MakeGroupOpt(Token* pToken1, Token* pToken2, Token* pToken3)
{
  return new GroupOpt(pToken1, pToken2, pToken3);
}
void GroupOpt::FreeGroupOpt(GroupOpt* pGroupOpt)
{
  delete pGroupOpt;
}


