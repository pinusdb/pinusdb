
#include "expr/target_list.h"
#include "expr/expr_value.h"
#include "expr/parse.h"

TargetList::TargetList()
{
  fieldNo_ = 1;
}

TargetList::~TargetList()
{
  for (auto targetIt = targetVec_.begin(); targetIt != targetVec_.end(); targetIt++)
  {
    delete targetIt->first;
  }
}

TargetList* TargetList::AppendExprValue(TargetList* pList, ExprValue* pValue, Token* pAliasName)
{
  DBVal tmpVal;
  char nameBuf[32];
  std::string aliasName;

  if (pList == nullptr)
  {
    pList = new TargetList();
  }

  do {
    if (pAliasName != nullptr)
    {
      aliasName = std::string(pAliasName->str_, pAliasName->len_);
      break;
    }

    //if (pValue->GetValueType() == TK_ID || pValue->GetValueType() == TK_STRING)
    if (pValue->GetValueType() == TK_ID)
    {
      tmpVal = pValue->GetValue();
      if (!DBVAL_IS_STRING(&tmpVal))
        break;

      aliasName = std::string(DBVAL_GET_STRING(&tmpVal), DBVAL_GET_LEN(&tmpVal));
      break;
    }

    if (pValue->GetValueType() == TK_STAR)
      break;

    sprintf(nameBuf, "_field_%d_", pList->fieldNo_++);
    aliasName = nameBuf;
  } while (false);

  pList->targetVec_.push_back(std::pair<ExprValue*, std::string>(pValue, aliasName));
  return pList;
}

void TargetList::FreeTargetList(TargetList* pList)
{
  if (pList != nullptr)
  {
    delete pList;
  }
}


