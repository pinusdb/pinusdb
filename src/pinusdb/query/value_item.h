#pragma once
#include "internal.h"
#include "util/string_tool.h"
#include "table/db_value.h"
#include "util/date_time.h"
#include "expr/expr_value.h"
#include "table/table_info.h"
#include "query/query_field.h"
#include <list>

class ValueItem
{
public:
  ValueItem() {}
  virtual ~ValueItem() {}

  //获取值
  virtual PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const = 0;
  //获取值类型
  virtual int32_t GetValueType() const = 0;
  //是否符合规则
  virtual bool IsValid() const = 0;
  //是否是常数值
  virtual bool IsConstValue() const = 0;

  virtual bool IsDevIdCondition() const { return false; }
  virtual bool IsTstampCondition() const { return false; }
  virtual bool GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const { return false; }
  virtual bool GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const { return false; }
};

//常数
class ConstValue : public ValueItem
{
public:
  ConstValue(int valType)
  {
    this->valType_ = valType;
    DBVAL_SET_NULL(&val_);
  }

  ConstValue(bool value)
  {
    this->valType_ = PDB_VALUE_TYPE::VAL_BOOL;
    DBVAL_SET_BOOL(&val_, value);
  }

  ConstValue(int64_t value, bool isTime)
  {
    if (isTime)
    {
      this->valType_ = PDB_VALUE_TYPE::VAL_DATETIME;
      if (value >= MinMillis && value < MaxMillis)
      {
        DBVAL_SET_DATETIME(&val_, value);
      }
      else
      {
        DBVAL_SET_NULL(&val_);
      }
    }
    else
    {
      this->valType_ = PDB_VALUE_TYPE::VAL_INT64;
      DBVAL_SET_INT64(&val_, value);
    }
  }

  ConstValue(double value)
  {
    this->valType_ = PDB_VALUE_TYPE::VAL_DOUBLE;
    DBVAL_SET_DOUBLE(&val_, value);
  }

  ConstValue(const char* pStr, size_t len, bool isBlob)
  {
    if (isBlob)
    {
      this->valType_ = PDB_VALUE_TYPE::VAL_BLOB;
      DBVAL_SET_BLOB(&val_, pStr, len);
    }
    else
    {
      this->valType_ = PDB_VALUE_TYPE::VAL_STRING;
      DBVAL_SET_STRING(&val_, pStr, len);
    }
  }

  ConstValue(DBVal val)
  {
    val_ = val;
    valType_ = DBVAL_GET_TYPE(&val_);
  }

  int32_t GetValueType() const override
  {
    return valType_;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pResult != nullptr)
    {
      *pResult = val_;
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    return true;
  }

  bool IsConstValue() const override
  {
    return true;
  }

private:
  int32_t valType_;
  DBVal val_;
};


//字段值
class FieldValue : public ValueItem
{
public:
  FieldValue(size_t fieldPos, int32_t fieldType)
  {
    fieldPos_ = fieldPos;
    fieldType_ = fieldType;
  }

  virtual ~FieldValue()
  {}

  int32_t GetValueType() const override
  {
    return fieldType_;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pVals == nullptr)
      return PdbE_INVALID_PARAM;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_ELE_GET_TYPE(pVals, fieldPos_) == fieldType_)
    {
      *pResult = pVals[fieldPos_];
    }
    else
    {
      DBVAL_SET_NULL(pResult);
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    return true;
  }

  bool IsConstValue() const override
  {
    return false;
  }

private:
  size_t fieldPos_;
  int32_t fieldType_;
};

ValueItem* BuildGeneralValueItem(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMillis);
PdbErr_t BuildTargetGroupItem(const TableInfo* pTableInfo, const ExprValue* pExpr,
  TableInfo* pGroupInfo, std::vector<QueryField*>& fieldVec, int64_t nowMillis);

bool IncludeAggFunction(const ExprValue* pExpr);


