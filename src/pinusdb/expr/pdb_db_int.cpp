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

#include "expr/pdb_db_int.h"
#include "expr/parse.h"
#include <stack>
#include <vector>
#include "expr/sql_parser.h"
#include "expr/expr_value.h"
#include "expr/target_list.h"
#include "expr/column_item.h"
#include "expr/group_opt.h"
#include "expr/orderby_opt.h"
#include "expr/limit_opt.h"
#include "expr/record_list.h"

void pdbSetError(SQLParser* pParse, const char* pErrMsg)
{
  if (pParse != nullptr)
  {
    pParse->SetError();
  }
}

void pdbDelete(SQLParser* pParse, Token* pTabName, ExprValue* pWhere)
{
  if (pParse != nullptr)
  {
    pParse->SetDelete(pTabName, pWhere);
  }
}

void pdbDropTable(SQLParser* pParse, Token* pTabToken)
{
  if (pParse != nullptr)
  {
    pParse->SetDropTable(pTabToken);
  }
}

void pdbAttachTable(SQLParser* pParse, Token* pTabToken)
{
  if (pParse != nullptr)
  {
    pParse->SetAttachTable(pTabToken);
  }
}

void pdbDetachTable(SQLParser* pParse, Token* pTabToken)
{
  if (pParse != nullptr)
  {
    pParse->SetDetachTable(pTabToken);
  }
}

void pdbAttachFile(SQLParser* pParse, Token* pTabName, Token* pDate, Token* pType)
{
  if (pParse != nullptr)
  {
    pParse->SetAttachFile(pTabName, pDate, pType);
  }
}

void pdbDetachFile(SQLParser* pParse, Token* pTabName, Token* pDate)
{
  if (pParse != nullptr)
  {
    pParse->SetDetachFile(pTabName, pDate);
  }
}

void pdbDropFile(SQLParser* pParse, Token* pTabName, Token* pDate)
{
  if (pParse != nullptr)
  {
    pParse->SetDropFile(pTabName, pDate);
  }
}

void pdbSelect(SQLParser* pParse, TargetList* pTagList, Token* pSrcTab,
  ExprValue* pWhere, GroupOpt* pGroup, OrderByOpt* pOrderBy, LimitOpt* pLimit)
{
  if (pParse != nullptr)
  {
    pParse->SetQuery(pTagList, pSrcTab, pWhere, pGroup, pOrderBy, pLimit);
  }
}

/*
void pdbSelect2(SQLParser* pParse, TargetList* pTagList1, Token* pSrcTab, ExprValue* pWhere1,
  GroupOpt* pGroup, OrderByOpt* pOrderBy, LimitOpt* pLimit, TargetList* pTagList2, ExprValue* pWhere2)
{
  if (pParse != nullptr)
  {
    ExprValueList* pWhere = pWhere1;
    if (pWhere2 != nullptr)
    {
      if (pWhere == nullptr)
      {
        pWhere = pWhere2;
      }
      else
      {
        const std::vector<ExprValue*>* pExprList = pWhere2->GetValueList();
        for (size_t i = 0; i < pExprList->size(); i++)
        {
          ExprValueList::AppendExprValue(pWhere, (*pExprList)[i]);
        }

        pWhere2->Clear();
        delete pWhere2;
      }
    }

    TargetList* pTagList = pTagList2;
    const std::vector<TargetItem>* pTList = pTagList->GetTargetList();
    if (pTList->size() == 1)
    {
      if ((*pTList)[0].first->GetValueType() == TK_STAR)
      {
        pTagList = pTagList1;
      }
    }

    if (pTagList == pTagList1)
    {
      delete pTagList2;
    }
    else
    {
      delete pTagList1;
    }

    pParse->SetQuery(pTagList, pSrcTab, pWhere, pGroup, pOrderBy, pLimit);
  }
}
*/

void pdbCreateTable(SQLParser* pParse, Token* pTabName, ColumnList* pColList)
{
  if (pParse != nullptr)
  {
    pParse->SetCreateTable(pTabName, pColList);
  }
}

void pdbAlterTable(SQLParser* pParse, Token* pTabName, ColumnList* pColList)
{
  if (pParse != nullptr)
  {
    pParse->SetAlterTable(pTabName, pColList);
  }
}

void pdbAddUser(SQLParser* pParse, Token* pNameToken, Token* pPwdToken)
{
  if (pParse != nullptr)
  {
    pParse->SetAddUser(pNameToken, pPwdToken);
  }
}

void pdbChangePwd(SQLParser* pParse, Token* pNameToken, Token* pPwdToken)
{
  if (pParse != nullptr)
  {
    pParse->SetChangePwd(pNameToken, pPwdToken);
  }
}

void pdbChangeRole(SQLParser* pParse, Token* pNameToken, Token* pRoleToken)
{
  if (pParse != nullptr)
  {
    pParse->SetChangeRole(pNameToken, pRoleToken);
  }
}

void pdbDropUser(SQLParser* pParse, Token* pNameToken)
{
  if (pParse != nullptr)
  {
    pParse->SetDropUser(pNameToken);
  }
}

void pdbInsert(SQLParser* pParse, Token* pTabName, TargetList* pTagList, RecordList* pRecList)
{
  if (pParse != nullptr)
  {
    pParse->SetInsert(pTabName, pTagList, pRecList);
  }
}