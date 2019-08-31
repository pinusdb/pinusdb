#include "expr/limit_opt.h"
#include "util/string_tool.h"
#include "internal.h"

LimitOpt::LimitOpt(Token* pQueryCnt)
{
  this->queryCnt_ = 0;
  this->offsetCnt_ = 0;

  if (!StringTool::StrToInt64(pQueryCnt->str_, pQueryCnt->len_, &queryCnt_))
    this->queryCnt_ = -1;
}
LimitOpt::LimitOpt(Token* pOffset, Token* pQueryCnt)
{
  this->queryCnt_ = 0;
  this->offsetCnt_ = 0;

  if (!StringTool::StrToInt64(pOffset->str_, pOffset->len_, &offsetCnt_))
    this->offsetCnt_ = -1;

  if (!StringTool::StrToInt64(pQueryCnt->str_, pQueryCnt->len_, &queryCnt_))
    this->queryCnt_ = -1;
}

LimitOpt::~LimitOpt()
{

}

PdbErr_t LimitOpt::Valid() const
{
  if (queryCnt_ < 0 || queryCnt_ > PDB_QUERY_MAX_COUNT || offsetCnt_ < 0)
    return PdbE_SQL_LIMIT_ERROR;

  return PdbE_OK;
}

size_t LimitOpt::GetOffset() const
{
  return static_cast<size_t>(offsetCnt_);
}
size_t LimitOpt::GetQueryCnt() const
{
  return static_cast<size_t>(queryCnt_);
}