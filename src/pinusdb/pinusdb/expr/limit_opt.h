#pragma once
#include <stdint.h>
#include "expr/pdb_db_int.h"
#include "pdb.h"

class LimitOpt
{
public:
  LimitOpt(Token* pQueryCnt);
  LimitOpt(Token* pOffset, Token* pQueryCnt);
  ~LimitOpt();

  PdbErr_t Valid() const;

  size_t GetOffset() const;
  size_t GetQueryCnt() const;

private:
  int64_t offsetCnt_;
  int64_t queryCnt_;
};
