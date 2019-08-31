#include "expr/insert_sql.h"
#include "expr/parse.h"
#include "util/string_tool.h"
#include "util/date_time.h"
#include <math.h>

#define CONVERT_TO_REAL_VALUE(pVal, kMultiple) do { \
  if(DBVAL_IS_INT64(pVal) && DBVAL_GET_INT64(pVal) >= PDB_MIN_REAL_VALUE && DBVAL_GET_INT64(pVal) <= PDB_MAX_REAL_VALUE) \
  { \
    DBVAL_SET_INT64(pVal, (DBVAL_GET_INT64(pVal) * kMultiple)); \
  } \
  else if (DBVAL_IS_DOUBLE(pVal) && DBVAL_GET_DOUBLE(pVal) >= PDB_MIN_REAL_VALUE && DBVAL_GET_DOUBLE(pVal) <= PDB_MAX_REAL_VALUE) \
  { \
    DBVAL_SET_INT64(pVal, static_cast<int64_t>(std::round(DBVAL_GET_DOUBLE(pVal) * kMultiple))); \
  } \
  else \
  { \
    return PdbE_VALUE_MISMATCH;  \
  } \
} while(false)

PdbErr_t ConvertDBVal(DBVal* pVal, int valType) 
{
  int64_t intVal = 0;
  double dval = 0;
  switch (valType)
  {
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    if (DBVAL_IS_STRING(pVal))
    {
      if (!DateTime::Parse(DBVAL_GET_STRING(pVal), DBVAL_GET_LEN(pVal), &intVal))
        return PdbE_INVALID_DATETIME_VAL;

      DBVAL_SET_DATETIME(pVal, intVal);
    }
    else if (DBVAL_IS_INT64(pVal) && DBVAL_GET_INT64(pVal) >= MinMillis && DBVAL_GET_INT64(pVal) <= MaxMillis)
    {
      DBVAL_SET_DATETIME(pVal, DBVAL_GET_INT64(pVal));
    }
    else
    {
      return PdbE_VALUE_MISMATCH;
    }
    break;
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    if (DBVAL_IS_INT64(pVal))
    {
      DBVAL_SET_DOUBLE(pVal, static_cast<double>(DBVAL_GET_INT64(pVal)));
    }
    else
    {
      return PdbE_VALUE_MISMATCH;
    }
    break;
  case PDB_FIELD_TYPE::TYPE_REAL2:
    CONVERT_TO_REAL_VALUE(pVal, DBVAL_REAL2_MULTIPLE);
    break;
  case PDB_FIELD_TYPE::TYPE_REAL3:
    CONVERT_TO_REAL_VALUE(pVal, DBVAL_REAL3_MULTIPLE);
    break;
  case PDB_FIELD_TYPE::TYPE_REAL4:
    CONVERT_TO_REAL_VALUE(pVal, DBVAL_REAL4_MULTIPLE);
    break;
  case PDB_FIELD_TYPE::TYPE_REAL6:
    CONVERT_TO_REAL_VALUE(pVal, DBVAL_REAL6_MULTIPLE);
    break;
  default:
    return PdbE_VALUE_MISMATCH;
  }

  return PdbE_OK;
}


InsertSql::InsertSql()
{
}

InsertSql::~InsertSql()
{
}

PdbErr_t InsertSql::AddToken(int32_t type, int32_t len, const char* pStr)
{
  SqlToken token;
  token.type_ = type;
  token.len_ = len;
  token.str_ = pStr;

  tokenList_.push_back(token);
  return PdbE_OK;
}

PdbErr_t InsertSql::ParseMeta()
{
  PdbErr_t retVal = PdbE_OK;
  colNameVec_.clear();
  curIter_ = tokenList_.begin();

  if (tokenList_.size() <= 4)
    return PdbE_SQL_ERROR;

  if (tokenList_.back().type_ == TK_SEMI)
  {
    tokenList_.pop_back();
  }

  if (curIter_->type_ != TK_INSERT)
    return PdbE_SQL_ERROR;

  curIter_++;
  if (curIter_->type_ != TK_INTO)
    return PdbE_SQL_ERROR;

  curIter_++;
  if (curIter_->type_ != TK_ID)
    return PdbE_SQL_ERROR;

  tabName_ = std::string(curIter_->str_, curIter_->len_);

  //×óÀ¨ºÅ
  curIter_++;
  if (curIter_->type_ != TK_LP)
    return PdbE_SQL_ERROR;

  curIter_++;
  while (curIter_ != tokenList_.end())
  {
    if (curIter_->type_ != TK_ID)
      return PdbE_SQL_ERROR;

    colNameVec_.push_back(std::string(curIter_->str_, curIter_->len_));

    curIter_++;
    if (curIter_ == tokenList_.end())
      return PdbE_SQL_ERROR;

    if (curIter_->type_ != TK_COMMA)
      break;

    curIter_++;
  }

  //ÓÒÀ¨ºÅ
  if (curIter_ == tokenList_.end())
    return PdbE_SQL_ERROR;
  if (curIter_->type_ != TK_RP)
    return PdbE_SQL_ERROR;

  //VALUES
  curIter_++;
  if (curIter_ == tokenList_.end())
    return PdbE_SQL_ERROR;
  if (curIter_->type_ != TK_VALUES)
    return PdbE_SQL_ERROR;

  curIter_++;
  return PdbE_OK;
}

PdbErr_t InsertSql::InitTableInfo(const TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  size_t fieldPos = 0;
  posVec_.clear();

  for (auto nameIt = colNameVec_.begin(); nameIt != colNameVec_.end(); nameIt++)
  {
    retVal = pTabInfo->GetFieldInfo(nameIt->c_str(), &fieldPos, &fieldType);
    if (retVal != PdbE_OK)
      return retVal;

    posVec_.push_back(fieldPos);
  }

  return PdbE_OK;
}


PdbErr_t InsertSql::GetNextRec(int* pTypes, DBVal* pVals, size_t valCnt)
{
  bool minusFlag = false;
  DBVal val;
  int64_t int64Val = 0;
  double doubleVal = 0;
  size_t idx = 0;
  int pos = 0;
  PdbErr_t retVal = PdbE_OK;

  if (curIter_ == tokenList_.end())
    return PdbE_SQL_ERROR;

  //×óÀ¨ºÅ
  if (curIter_->type_ != TK_LP)
    return PdbE_SQL_ERROR;

  curIter_++;
  while (curIter_ != tokenList_.end())
  {
    if (idx >= posVec_.size())
      return PdbE_SQL_ERROR;

    if (curIter_->type_ == TK_MINUS || curIter_->type_ == TK_PLUS)
    {
      minusFlag = curIter_->type_ == TK_MINUS;

      curIter_++;
      if (curIter_ == tokenList_.end())
        return PdbE_SQL_ERROR;

      if (curIter_->type_ == TK_INTEGER)
        curIter_->type_ = minusFlag ? TK_UINTEGER : TK_INTEGER;
      else if (curIter_->type_ == TK_DOUBLE)
        curIter_->type_ = minusFlag ? TK_UDOUBLE : TK_DOUBLE;
      else
        return PdbE_SQL_ERROR;
    }

    switch (curIter_->type_)
    {
    case TK_TRUE:
    {
      DBVAL_SET_BOOL(&val, true);
      break;
    }
    case TK_FALSE:
    {
      DBVAL_SET_BOOL(&val, false);
      break;
    }
    case TK_INTEGER:
    case TK_UINTEGER:
    {
      if (!StringTool::StrToInt64(curIter_->str_, curIter_->len_, &int64Val))
        return PdbE_INVALID_INT_VAL;

      if (curIter_->type_ == TK_UINTEGER)
        int64Val *= -1;

      DBVAL_SET_INT64(&val, int64Val);
      break;
    }
    case TK_DOUBLE:
    case TK_UDOUBLE:
    {
      if (!StringTool::StrToDouble(curIter_->str_, curIter_->len_, &doubleVal))
        return PdbE_INVALID_DOUBLE_VAL;

      if (curIter_->type_ == TK_UDOUBLE)
        doubleVal *= -1;

      DBVAL_SET_DOUBLE(&val, doubleVal);
      break;
    }
    case TK_STRING:
      DBVAL_SET_STRING(&val, curIter_->str_, curIter_->len_);
      break;
    case TK_BLOB:
      DBVAL_SET_BLOB(&val, curIter_->str_, curIter_->len_);
      break;
    default:
      return PdbE_SQL_ERROR;
    }

    size_t pos = posVec_[idx];
    if (pTypes[pos] != DBVAL_GET_TYPE(&val))
    {
      retVal = ConvertDBVal(&val, pTypes[pos]);
      if (retVal != PdbE_OK)
        return retVal;
    }

    pVals[pos] = val;

    curIter_++;
    if (curIter_ == tokenList_.end())
      return PdbE_SQL_ERROR;

    if (curIter_->type_ != TK_COMMA)
      break;

    curIter_++;
    idx++;
  }

  if (curIter_ == tokenList_.end())
    return PdbE_SQL_ERROR;

  //RP
  if (curIter_->type_ != TK_RP)
    return PdbE_SQL_ERROR;

  if ((idx + 1) != posVec_.size())
    return PdbE_SQL_ERROR;

  curIter_++;
  if (curIter_ == tokenList_.end())
    return PdbE_OK;

  if (curIter_->type_ == TK_COMMA)
    curIter_++;

  return PdbE_OK;
}

/////////////////////////////////////////////////////////////////////////

InsertTable::InsertTable(size_t fieldCnt, size_t recCnt)
{
  fieldCnt_ = fieldCnt;
  recCnt_ = recCnt;
}

InsertTable::~InsertTable()
{
}

PdbErr_t InsertTable::AddVal(const DBVal* pVal)
{
  valList_.push_back(*pVal);
  return PdbE_OK;
}

PdbErr_t InsertTable::ParseMeta()
{
  if (valList_.size() != ((fieldCnt_ * (recCnt_ + 1)) + 1))
    return PdbE_PACKET_ERROR;

  valIter_ = valList_.begin();

  //±íÃû
  if (valIter_->dataType_ != PDB_VALUE_TYPE::VAL_STRING)
    return PdbE_INVALID_TABLE_NAME;

  tabName_ = std::string((const char*)valIter_->val_.pData_, valIter_->dataLen_);

  valIter_++;
  //×Ö¶Î
  for (size_t idx = 0; idx < fieldCnt_; idx++)
  {
    if (valIter_->dataType_ != PDB_VALUE_TYPE::VAL_STRING)
      return PdbE_INVALID_FIELD_NAME;

    colNameVec_.push_back(std::string((const char*)valIter_->val_.pData_, valIter_->dataLen_));
    valIter_++;
  }

  return PdbE_OK;
}

PdbErr_t InsertTable::InitTableInfo(const TableInfo* pTabInfo)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  size_t fieldPos = 0;
  posVec_.clear();

  for (auto nameIt = colNameVec_.begin(); nameIt != colNameVec_.end(); nameIt++)
  {
    retVal = pTabInfo->GetFieldInfo(nameIt->c_str(), &fieldPos, &fieldType);
    if (retVal != PdbE_OK)
      return retVal;

    posVec_.push_back(fieldPos);
  }

  return PdbE_OK;

}

PdbErr_t InsertTable::GetNextRec(int* pTypes, DBVal* pVals, size_t valCnt)
{
  PdbErr_t retVal = PdbE_OK;
  int64_t int64Val = 0;
  DBVal val;

  for (size_t idx = 0; idx < fieldCnt_; idx++)
  {
    size_t pos = posVec_[idx];
    if (valIter_->dataType_ == PDB_VALUE_TYPE::VAL_NULL)
    {
      switch (pTypes[pos])
      {
      case PDB_FIELD_TYPE::TYPE_BOOL:
        DBVAL_ELE_SET_BOOL(pVals, pos, false);
        break;
      case PDB_FIELD_TYPE::TYPE_INT64:
      case PDB_FIELD_TYPE::TYPE_REAL2:
      case PDB_FIELD_TYPE::TYPE_REAL3:
      case PDB_FIELD_TYPE::TYPE_REAL4:
      case PDB_FIELD_TYPE::TYPE_REAL6:
        DBVAL_ELE_SET_INT64(pVals, pos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_DATETIME:
        DBVAL_ELE_SET_DATETIME(pVals, pos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_DOUBLE:
        DBVAL_ELE_SET_DOUBLE(pVals, pos, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_STRING:
        DBVAL_ELE_SET_STRING(pVals, pos, nullptr, 0);
        break;
      case PDB_FIELD_TYPE::TYPE_BLOB:
        DBVAL_ELE_SET_BLOB(pVals, pos, nullptr, 0);
        break;
      }
    }
    else if (valIter_->dataType_ == pTypes[pos])
    {
      pVals[pos] = *valIter_;
    }
    else
    {
      val = *valIter_;
      if (ConvertDBVal(&val, pTypes[pos]) != PdbE_OK)
        retVal = PdbE_VALUE_MISMATCH;

      pVals[pos] = val;
    }

    valIter_++;
  }

  return retVal;
}
