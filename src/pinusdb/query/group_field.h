#pragma once

#include "internal.h"
#include "table/db_value.h"
#include "query/query_field.h"

template<int ValType, typename T>
class AvgFunc : public QueryField
{
public:
  AvgFunc(size_t fieldPos)
  {
    this->fieldPos_ = fieldPos;
    this->totalVal_ = 0;
    this->dataCnt_ = 0;
  }

  virtual ~AvgFunc() { }

  virtual int32_t FieldType() { return ValType; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (DBVAL_ELE_IS_NULL(pVals, fieldPos_))
      return PdbE_OK;

    if (DBVAL_ELE_GET_TYPE(pVals, fieldPos_) != ValType)
      return PdbE_INVALID_PARAM;

    if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT64)
    {
      totalVal_ += DBVAL_ELE_GET_INT64(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_DOUBLE)
    {
      totalVal_ += DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_);
    }
    else
    {
      return PdbE_INVALID_PARAM;
    }

    dataCnt_++;
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    if (dataCnt_ > 0)
    {
      if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT64)
      {
        DBVAL_SET_INT64(pVal, (totalVal_ / dataCnt_));
      }
      else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_DOUBLE)
      {
        DBVAL_SET_DOUBLE(pVal, (totalVal_ / dataCnt_));
      }
      else
      {
        return PdbE_INVALID_PARAM;
      }
    }
    else
    {
      DBVAL_SET_NULL(pVal);
    }
    return PdbE_OK;
  }

  virtual QueryField* NewField(int64_t devId, int64_t tstamp)
  {
    return new AvgFunc<ValType, T>(fieldPos_);
  }

private:
  size_t fieldPos_;
  T totalVal_;
  int64_t dataCnt_;
};

///////////////////////////////////////////////////////////////////////////////

class CountFunc : public QueryField
{
public:
  CountFunc(size_t fieldPos)
  {
    this->fieldPos_ = fieldPos;
    this->dataCnt_ = 0;
  }

  virtual ~CountFunc() {}

  virtual int32_t FieldType() { return PDB_FIELD_TYPE::TYPE_INT64; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (!DBVAL_ELE_IS_NULL(pVals, fieldPos_))
    {
      this->dataCnt_++;
    }

    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    DBVAL_SET_INT64(pVal, dataCnt_);
    return PdbE_OK;
  }

  virtual QueryField* NewField(int64_t devId, int64_t tstamp)
  {
    return new CountFunc(fieldPos_);
  }

private:
  size_t fieldPos_;
  int64_t dataCnt_;
};

/////////////////////////////////////////////////////////////////////////////////////

template<int ValType, bool IsFirst>
class FirstOrLastValueFunc : public QueryField
{
  static constexpr bool IsBlockValue = (ValType == PDB_FIELD_TYPE::TYPE_STRING || ValType == PDB_FIELD_TYPE::TYPE_BLOB);

public:
  FirstOrLastValueFunc(size_t fieldPos)
  {
    fieldPos_ = fieldPos;
    curTs_ = IsFirst ? MaxMillis : MinMillis;
    DBVAL_SET_NULL(&curVal_);

    if constexpr (IsBlockValue)
    {
      pValBuf_ = new std::string();
    }
    else
    {
      pValBuf_ = nullptr;
    }
  }

  virtual ~FirstOrLastValueFunc()
  {
    if (pValBuf_ != nullptr)
    {
      delete pValBuf_;
    }
  }

  virtual int32_t FieldType() { return ValType; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (DBVAL_ELE_GET_TYPE(pVals, fieldPos_) != PDB_VALUE_TYPE::VAL_NULL
      && DBVAL_ELE_GET_TYPE(pVals, fieldPos_) != ValType)
    {
      return PdbE_INVALID_PARAM;
    }

    if constexpr (IsFirst)
    {
      if (DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX) >= curTs_)
        return PdbE_OK;
    }

    if constexpr (!IsFirst)
    {
      if (DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX) <= curTs_)
        return PdbE_OK;
    }

    if constexpr (IsBlockValue)
    {
      if (DBVAL_ELE_IS_NULL(pVals, fieldPos_))
      {
        DBVAL_SET_NULL(&curVal_);
      }
      else
      {
        size_t dataLen = DBVAL_ELE_GET_LEN(pVals, fieldPos_);
        if (dataLen > pValBuf_->capacity())
        {
          pValBuf_->resize(dataLen);
        }

        memcpy(&((*pValBuf_)[0]), DBVAL_ELE_GET_BLOB(pVals, fieldPos_), dataLen);
        pValBuf_->resize(dataLen);
        DBVAL_SET_BLOCK_VALUE(&curVal_, ValType, &((*pValBuf_)[0]), dataLen);
      }
    }
    else
    {
      curVal_ = pVals[fieldPos_];
    }

    curTs_ = DBVAL_ELE_GET_DATETIME(pVals, PDB_TSTAMP_INDEX);
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    *pVal = curVal_;
    return PdbE_OK;
  }

  virtual QueryField* NewField(int64_t devId, int64_t tstamp)
  {
    return new FirstOrLastValueFunc<ValType, IsFirst>(fieldPos_);
  }

  virtual bool IsFirstFunc() { return IsFirst; }
  virtual bool IsLastFunc() { return !IsFirst; }

private:
  size_t fieldPos_;
  int64_t curTs_;
  DBVal curVal_;
  std::string* pValBuf_;
};

/////////////////////////////////////////////////////////////////////////////////////

template<int CompareType, int TargetType, bool IsMax>
class ExtremeValueFunc : public QueryField
{
public:
  ExtremeValueFunc(size_t comparePos, size_t targetPos)
  {
    comparePos_ = comparePos;
    targetPos_ = targetPos;
    DBVAL_SET_NULL(&targetVal_);
    DBVAL_SET_NULL(&compareVal_);
    if constexpr (TargetType == PDB_FIELD_TYPE::TYPE_STRING || TargetType == PDB_FIELD_TYPE::TYPE_BLOB)
    {
      pTargetBuf_ = new std::string();
    }
    else
    {
      pTargetBuf_ = nullptr;
    }
  }

  virtual ~ExtremeValueFunc() { if (pTargetBuf_ != nullptr) { delete pTargetBuf_; } }
  virtual int32_t FieldType() { return TargetType; }
  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (DBVAL_ELE_IS_NULL(pVals, comparePos_))
      return PdbE_OK;

    if ( DBVAL_ELE_GET_TYPE(pVals, targetPos_) != PDB_VALUE_TYPE::VAL_NULL
      && DBVAL_ELE_GET_TYPE(pVals, targetPos_) != TargetType)
      return PdbE_INVALID_PARAM;

    if (DBVAL_ELE_GET_TYPE(pVals, comparePos_) != CompareType)
      return PdbE_INVALID_PARAM;

    do {
      if (DBVAL_IS_NULL(&compareVal_))
        break;

      if constexpr (IsMax)
      {
        if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_DOUBLE)
        {
          if (DBVAL_ELE_GET_DOUBLE(pVals, comparePos_) <= DBVAL_GET_DOUBLE(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT64)
        {
          if (DBVAL_ELE_GET_INT64(pVals, comparePos_) <= DBVAL_GET_INT64(&compareVal_))
            return PdbE_OK;
        }
      }
      else
      {
        if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_DOUBLE)
        {
          if (DBVAL_ELE_GET_DOUBLE(pVals, comparePos_) >= DBVAL_GET_DOUBLE(&compareVal_))
            return PdbE_OK;
        }
        else if constexpr (CompareType == PDB_FIELD_TYPE::TYPE_INT64)
        {
          if (DBVAL_ELE_GET_INT64(pVals, comparePos_) >= DBVAL_GET_INT64(&compareVal_))
            return PdbE_OK;
        }
      }

    } while (false);

    compareVal_ = pVals[comparePos_];
    if constexpr (TargetType == PDB_FIELD_TYPE::TYPE_STRING || TargetType == PDB_FIELD_TYPE::TYPE_BLOB)
    {
      if (DBVAL_ELE_IS_NULL(pVals, targetPos_))
      {
        DBVAL_SET_NULL(&targetVal_);
      }
      else
      {
        size_t dataLen = DBVAL_ELE_GET_LEN(pVals, targetPos_);
        if (dataLen > pTargetBuf_->capacity())
        {
          pTargetBuf_->resize(dataLen);
        }

        memcpy(&((*pTargetBuf_)[0]), DBVAL_ELE_GET_BLOB(pVals, targetPos_), dataLen);
        pTargetBuf_->resize(dataLen);
        DBVAL_SET_BLOCK_VALUE(&targetVal_, TargetType, &((*pTargetBuf_)[0]), dataLen);
      }
    }
    else
    {
      targetVal_ = pVals[targetPos_];
    }
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    *pVal = targetVal_;
    return PdbE_OK;
  }

  virtual QueryField* NewField(int64_t devId, int64_t tstamp)
  {
    return new ExtremeValueFunc<CompareType, TargetType, IsMax>(comparePos_, targetPos_);
  }

private:
  size_t comparePos_;
  size_t targetPos_;
  DBVal compareVal_;
  DBVal targetVal_;
  std::string* pTargetBuf_;
};

/////////////////////////////////////////////////////////////////////////////////////

template<int ValType, typename T>
class SumFunc : public QueryField
{
public:
  SumFunc(size_t fieldPos)
  {
    haveVal_ = false;
    fieldPos_ = fieldPos;
    sumVal_ = 0;
  }

  virtual ~SumFunc() {}

  virtual int32_t FieldType() { return ValType; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    if (DBVAL_ELE_IS_NULL(pVals, fieldPos_))
      return PdbE_OK;

    if (DBVAL_ELE_GET_TYPE(pVals, fieldPos_) != ValType)
      return PdbE_INVALID_PARAM;

    haveVal_ = true;
    if constexpr (ValType == PDB_FIELD_TYPE::TYPE_INT64)
    {
      sumVal_ += DBVAL_ELE_GET_INT64(pVals, fieldPos_);
    }
    else if constexpr (ValType == PDB_FIELD_TYPE::TYPE_DOUBLE)
    {
      sumVal_ += DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_);
    }
    else
    {
      return PdbE_INVALID_PARAM;
    }
    
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    if (!haveVal_)
    {
      DBVAL_SET_NULL(pVal);
    }
    else
    {
      if (ValType == PDB_FIELD_TYPE::TYPE_INT64)
      {
        DBVAL_SET_INT64(pVal, sumVal_);
      }
      else if (ValType == PDB_FIELD_TYPE::TYPE_DOUBLE)
      {
        DBVAL_SET_DOUBLE(pVal, sumVal_);
      }
    }

    return PdbE_OK;
  }

  virtual QueryField* NewField(int64_t devId, int64_t tstamp)
  {
    return new SumFunc<ValType, T>(fieldPos_);
  }

private:
  bool haveVal_;
  size_t fieldPos_;
  T sumVal_;
};

/////////////////////////////////////////////////////////////////////////////////////

class GroupTstampField : public QueryField
{
public:
  GroupTstampField(int64_t tstamp)
  {
    tstamp_ = tstamp;
  }

  virtual ~GroupTstampField() {}

  virtual int32_t FieldType() { return PDB_FIELD_TYPE::TYPE_DATETIME; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    DBVAL_SET_DATETIME(pVal, tstamp_);
    return PdbE_OK;
  }

  virtual QueryField* NewField(int64_t devId, int64_t tstamp)
  {
    return new GroupTstampField(tstamp);
  }

  virtual bool IsLastFunc() { return true; };
  virtual bool IsFirstFunc() { return true; }

private:
  int64_t tstamp_;
};

class GroupDevIdField : public QueryField
{
public:
  GroupDevIdField(int64_t devId)
  {
    devId_ = devId;
  }

  virtual ~GroupDevIdField() {}

  virtual int32_t FieldType() { return PDB_FIELD_TYPE::TYPE_INT64; }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    DBVAL_SET_INT64(pVal, devId_);
    return PdbE_OK;
  }

  virtual QueryField* NewField(int64_t devId, int64_t tstamp)
  {
    return new GroupDevIdField(devId);
  }

  virtual bool IsLastFunc() { return true; };
  virtual bool IsFirstFunc() { return true; }

private:
  int64_t devId_;
};

/////////////////////////////////////////////////////////////////////////////////////

class AggIfExtendFunc : public QueryField
{
public:
  AggIfExtendFunc(ValueItem* pCondition, QueryField* pField, bool isRoot)
  {
    root_ = isRoot;
    pCondition_ = pCondition;
    pField_ = pField;
  }

  virtual ~AggIfExtendFunc() 
  {
    if (root_ && pCondition_ != nullptr)
      delete pCondition_;

    if (pField_ != nullptr)
      delete pField_;
  }

  virtual int32_t FieldType() { return pField_->FieldType(); }

  virtual PdbErr_t AppendData(const DBVal* pVals, size_t valCnt)
  {
    PdbErr_t retVal = PdbE_OK;
    DBVal condiVal;
    retVal = pCondition_->GetValue(pVals, &condiVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (DBVAL_IS_BOOL(&condiVal) && DBVAL_GET_BOOL(&condiVal))
    {
      return pField_->AppendData(pVals, valCnt);
    }

    return PdbE_OK;
  }

  virtual PdbErr_t GetResult(DBVal* pVal)
  {
    return pField_->GetResult(pVal);
  }

  virtual QueryField* NewField(int64_t devId, int64_t tstamp)
  {
    QueryField* pNewField = pField_->NewField(devId, tstamp);
    return new AggIfExtendFunc(pCondition_, pNewField, false);
  }

private:
  bool root_;
  ValueItem* pCondition_;
  QueryField* pField_;
};






