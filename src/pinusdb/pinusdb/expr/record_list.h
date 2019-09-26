#pragma once
#include "expr/expr_item.h"
#include "expr/pdb_db_int.h"

class RecordList
{
public:
  RecordList() {}
  ~RecordList()
  {
    for (auto iter = recList_.begin(); iter != recList_.end(); iter++)
    {
      delete *iter;
    }
  }

  const std::list<ExprList*>& GetRecList() const { return recList_; }

  static RecordList* AppendRecordList(RecordList* pRecList, ExprList* pRec)
  {
    if (pRecList == nullptr)
    {
      pRecList = new RecordList();
    }

    if (pRec != nullptr)
      pRecList->recList_.push_back(pRec);

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
  std::list<ExprList*> recList_;
};


