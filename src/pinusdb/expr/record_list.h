#pragma once
#include "expr/expr_value.h"
#include "expr/target_list.h"
#include "expr/pdb_db_int.h"

class RecordList
{
public:
  RecordList() {}
  ~RecordList()
  {
    for (auto iter = recVec_.begin(); iter != recVec_.end(); iter++)
    {
      delete *iter;
    }
  }

  const std::vector<ExprValueList*>& GetRecList() const { return recVec_; }

  static RecordList* AppendRecordList(RecordList* pRecList, ExprValueList* pRec)
  {
    if (pRecList == nullptr)
    {
      pRecList = new RecordList();
    }

    if (pRec != nullptr)
      pRecList->recVec_.push_back(pRec);

    return pRecList;
  }

  static void FreeRecordList(RecordList* pRecList)
  {
    if (pRecList != nullptr)
    {
      delete pRecList;
    }
  }

private:
  std::vector<ExprValueList*> recVec_;
};


