#pragma once

#include "pdb.h"
#include "internal.h"
#include "expr/pdb_db_int.h"
#include "util/string_tool.h"

class OrderByOpt
{
public:
  OrderByOpt(Token* pToken, bool isAsc)
  {
    valid_ = StringTool::ComparyNoCase(pToken->str_, (size_t)pToken->len_,
      TSTAMP_FIELD_NAME, (sizeof(TSTAMP_FIELD_NAME) - 1));
    isAsc_ = isAsc;
  }

  ~OrderByOpt() { }

  bool Valid() const { return valid_; }
  bool IsASC() const { return isAsc_; }

private:
  bool valid_;
  bool isAsc_;
};


