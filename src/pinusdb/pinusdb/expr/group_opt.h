#pragma once

#include "pdb.h"
#include "expr/pdb_db_int.h"
#include <string>
#include <vector>

class GroupOpt
{
public:
  GroupOpt(Token* pToken1);
  GroupOpt(Token* pToken1, Token* pToken2, Token* pToken3);
  ~GroupOpt();

  bool Valid() const;
  bool IsTableNameGroup() const;
  bool IsDevIdGroup() const;
  bool IsTStampGroup() const;
  PdbErr_t GetTStampStep(int64_t& timeStampStep) const;

  static GroupOpt* MakeGroupOpt(Token* pToken1);
  static GroupOpt* MakeGroupOpt(Token* pToken1, Token* pToken2, Token* pToken3);
  static void FreeGroupOpt(GroupOpt* pGroupOpt);

private:
  bool valid_;
  bool isTableName_;
  bool isDevId_;
  bool isTStamp_;
  int64_t tVal_;

};
