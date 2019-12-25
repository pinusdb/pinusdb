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

#include "query/tstamp_filter.h"
#include "util/string_tool.h"
#include "expr/parse.h"
#include "pdb_error.h"
#include "util/date_time.h"
#include "internal.h"

TStampFilter::TStampFilter()
{
  minTimeStamp_ = MinMillis;
  maxTimeStamp_ = MaxMillis;
}
TStampFilter::~TStampFilter()
{

}

bool TStampFilter::BuildFilter(const ExprItem* pCondition)
{
  minTimeStamp_ = MinMillis;
  maxTimeStamp_ = MaxMillis;

  return _BuildFilter(pCondition);
}

bool TStampFilter::_BuildFilter(const ExprItem* pExpr)
{
  if (pExpr == nullptr)
    return true;

  int op = pExpr->GetOp();

  const ExprItem* pLeftExpr = pExpr->GetLeftExpr();
  const ExprItem* pRightExpr = pExpr->GetRightExpr();

  if (op == TK_AND)
  {
    if (pLeftExpr == nullptr || pRightExpr == nullptr)
      return false;

    if (!_BuildFilter(pLeftExpr))
      return false;

    if (!_BuildFilter(pRightExpr))
      return false;

    return true;
  }
  else
  {
    if (pLeftExpr == nullptr)
      return false;

    if (pLeftExpr->GetOp() != TK_ID)
      return true;

    const std::string& fieldName = pLeftExpr->GetValueStr();

    if (StringTool::ComparyNoCase(fieldName, TSTAMP_FIELD_NAME, (sizeof(TSTAMP_FIELD_NAME) - 1)))
    {
      //只处理时间戳的 等于，大于，大于等于，小于，小于等于
      if (op != TK_EQ
        && op != TK_GT
        && op != TK_GE
        && op != TK_LT
        && op != TK_LE)
        return true;

      int64_t timeStamp = 0;
      DBVal tmpVal;

      if (pRightExpr == nullptr)
        return false;

      if (!pRightExpr->GetDBVal(&tmpVal))
        return false;

      if (DBVAL_IS_INT64(&tmpVal))
      {
        timeStamp = DBVAL_GET_INT64(&tmpVal);
      }
      else if (DBVAL_IS_DATETIME(&tmpVal))
      {
        timeStamp = DBVAL_GET_DATETIME(&tmpVal);
      }
      else if (DBVAL_IS_STRING(&tmpVal))
      {
        DateTime dt;
        if (!dt.Parse((const char*)tmpVal.val_.pData_, tmpVal.dataLen_))
          return false;

        timeStamp = dt.TotalMilliseconds();
      }
      else
      {
        return false;
      }

      switch (op)
      {
      case TK_EQ:
      {
        if (minTimeStamp_ <= timeStamp)
        {
          minTimeStamp_ = timeStamp;
        }
        else
        {
          //empty set
          maxTimeStamp_ = 0;
        }

        if (maxTimeStamp_ >= timeStamp)
        {
          maxTimeStamp_ = timeStamp;
        }
        else
        {
          //empty set
          maxTimeStamp_ = 0;
        }

        break;
      }
      case TK_GT:
      case TK_GE:
      {
        if (minTimeStamp_ <= timeStamp)
        {
          minTimeStamp_ = timeStamp;
        }
        else
        {
          //empty set
          maxTimeStamp_ = 0;
        }
        break;
      }
      case TK_LT:
      case TK_LE:
      {
        if (maxTimeStamp_ >= timeStamp)
        {
          maxTimeStamp_ = timeStamp;
        }
        else
        {
          //empty set
          maxTimeStamp_ = 0;
        }
        break;
      }
      }
    }
  }

  return true;
}


