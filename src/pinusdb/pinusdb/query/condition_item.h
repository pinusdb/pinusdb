#pragma once

#include "internal.h"
#include "util/string_tool.h"
#include "table/db_value.h"

class ConditionItem
{
public:
  ConditionItem() {}
  virtual ~ConditionItem() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const = 0;
};

///////////////////////////////////////////////////////////////////

template<int ValType>
class LtNumCondition : public ConditionItem
{
public:
  LtNumCondition(size_t fieldPos, int64_t valParam)
  {
    fieldPos_ = fieldPos;
    valParam_ = valParam;
  }

  virtual ~LtNumCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return false;

    return DBVAL_ELE_GET_INT64(pVals, fieldPos_) < valParam_;
  }

private:
  size_t fieldPos_;
  int64_t valParam_;
};

class LtDoubleCondition : public ConditionItem
{
public:
  LtDoubleCondition(size_t fieldPos, double valParam)
  {
    fieldPos_ = fieldPos;
    valParam_ = valParam;
  }

  virtual ~LtDoubleCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_DOUBLE(pVals, fieldPos_))
      return false;

    return DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_) < valParam_;
  }

private:
  size_t fieldPos_;
  double valParam_;
};

////////////////////////////////////////////////////////////////////

template<int ValType>
class LeNumCondition : public ConditionItem
{
public:
  LeNumCondition(size_t fieldPos, int64_t valParam)
  {
    fieldPos_ = fieldPos;
    valParam_ = valParam;
  }

  virtual ~LeNumCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return false;

    return DBVAL_ELE_GET_INT64(pVals, fieldPos_) <= valParam_;
  }

private:
  size_t fieldPos_;
  int64_t valParam_;
};

class LeDoubleCondition : public ConditionItem
{
public:
  LeDoubleCondition(size_t fieldPos, double valParam)
  {
    fieldPos_ = fieldPos;
    valParam_ = valParam + DOUBLE_PRECISION;
  }

  virtual ~LeDoubleCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_DOUBLE(pVals, fieldPos_))
      return false;

    return DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_) < valParam_;
  }

private:
  size_t fieldPos_;
  double valParam_;
};

////////////////////////////////////////////////////////////////////

template<int ValType>
class GtNumCondition : public ConditionItem
{
public:
  GtNumCondition(size_t fieldPos, int64_t valParam)
  {
    fieldPos_ = fieldPos;
    valParam_ = valParam;
  }

  virtual ~GtNumCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return false;

    return DBVAL_ELE_GET_INT64(pVals, fieldPos_) > valParam_;
  }

private:
  size_t fieldPos_;
  int64_t valParam_;
};

class GtDoubleCondition : public ConditionItem
{
public:
  GtDoubleCondition(size_t fieldPos, double valParam)
  {
    fieldPos_ = fieldPos;
    valParam_ = valParam;
  }

  virtual ~GtDoubleCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_DOUBLE(pVals, fieldPos_))
      return false;

    return DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_) > valParam_;
  }

private:
  size_t fieldPos_;
  double valParam_;
};

//////////////////////////////////////////////////////////////////////////

template<int ValType>
class GeNumCondition : public ConditionItem
{
public:
  GeNumCondition(size_t fieldPos, int64_t valParam)
  {
    fieldPos_ = fieldPos;
    valParam_ = valParam;
  }

  virtual ~GeNumCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return false;

    return DBVAL_ELE_GET_INT64(pVals, fieldPos_) >= valParam_;
  }

private:
  size_t fieldPos_;
  int64_t valParam_;
};

class GeDoubleCondition : public ConditionItem
{
public:
  GeDoubleCondition(size_t fieldPos, double valParam)
  {
    fieldPos_ = fieldPos;
    valParam_ = valParam;
  }

  virtual ~GeDoubleCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_DOUBLE(pVals, fieldPos_))
      return false;

    return DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_) > valParam_;
  }

private:
  size_t fieldPos_;
  double valParam_;
};

//////////////////////////////////////////////////////////////////////////

template<int ValType>
class NeNumCondition : public ConditionItem
{
public:
  NeNumCondition(size_t fieldPos, int64_t valParam)
  {
    fieldPos_ = fieldPos;
    valParam_ = valParam;
  }

  virtual ~NeNumCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return false;

    return valParam_ != DBVAL_ELE_GET_INT64(pVals, fieldPos_);
  }

private:
  size_t fieldPos_;
  int64_t valParam_;
};

class NeDoubleCondition : public ConditionItem
{
public:
  NeDoubleCondition(size_t fieldPos, double valParam)
  {
    fieldPos_ = fieldPos;
    minParam_ = valParam - DOUBLE_PRECISION;
    maxParam_ = valParam + DOUBLE_PRECISION;
  }

  virtual ~NeDoubleCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_DOUBLE(pVals, fieldPos_))
      return false;

    return (DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_) < minParam_) || (DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_) > maxParam_);
  }

private:
  size_t fieldPos_;
  double minParam_;
  double maxParam_;
};

class NeStringCondition : public ConditionItem
{
public:
  NeStringCondition(size_t fieldPos, const char* pStr, size_t strLen)
  {
    fieldPos_ = fieldPos;
    pStr_ = pStr;
    strLen_ = strLen;
  }

  virtual ~NeStringCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_STRING(pVals, fieldPos_))
      return false;

    const char* pCurStr = DBVAL_ELE_GET_STRING(pVals, fieldPos_);
    size_t curLen = DBVAL_ELE_GET_LEN(pVals, fieldPos_);

    if (curLen != strLen_)
      return true;

    return memcmp(pCurStr, pStr_, strLen_) != 0;
  }

private:
  size_t fieldPos_;
  const char* pStr_;
  size_t strLen_;
};

//////////////////////////////////////////////////////////////////////////

template<int ValType>
class EqNumCondition : public ConditionItem
{
public:
  EqNumCondition(size_t fieldPos, int64_t valParam)
  {
    fieldPos_ = fieldPos;
    valParam_ = valParam;
  }

  virtual ~EqNumCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, ValType))
      return false;

    return valParam_ == DBVAL_ELE_GET_INT64(pVals, fieldPos_);
  }

private:
  size_t fieldPos_;
  int64_t valParam_;
};

class EqDoubleCondition : public ConditionItem
{
public:
  EqDoubleCondition(size_t fieldPos, double valParam)
  {
    fieldPos_ = fieldPos;
    minParam_ = valParam - DOUBLE_PRECISION;
    maxParam_ = valParam + DOUBLE_PRECISION;
  }

  virtual ~EqDoubleCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_DOUBLE(pVals, fieldPos_))
      return false;

    return (DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_) > minParam_) && (DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_) < maxParam_);
  }

private:
  size_t fieldPos_;
  double minParam_;
  double maxParam_;
};

class EqBoolCondition : public ConditionItem
{
public:
  EqBoolCondition(size_t fieldPos, bool valParam)
  {
    fieldPos_ = fieldPos;
    valParam_ = valParam;
  }

  virtual ~EqBoolCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_BOOL(pVals, fieldPos_))
      return false;

    return valParam_ == DBVAL_ELE_GET_BOOL(pVals, fieldPos_);
  }

private:
  size_t fieldPos_;
  bool valParam_;
};

class EqStringCondition : public ConditionItem
{
public:
  EqStringCondition(size_t fieldPos, const char* pStr, size_t strLen)
  {
    fieldPos_ = fieldPos;
    pStr_ = pStr;
    strLen_ = strLen;
  }

  virtual ~EqStringCondition() {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_STRING(pVals, fieldPos_))
      return false;

    const char* pCurStr = DBVAL_ELE_GET_STRING(pVals, fieldPos_);
    size_t curLen = DBVAL_ELE_GET_LEN(pVals, fieldPos_);

    if (curLen != strLen_)
      return false;

    return memcmp(pCurStr, pStr_, strLen_) == 0;
  }

private:
  size_t fieldPos_;
  const char* pStr_;
  size_t strLen_;
};

//////////////////////////////////////////////////////////////////////////

class LikeCondition : public ConditionItem
{
public:
  LikeCondition(size_t fieldPos, const char* pStr, size_t strLen)
  {
    this->fieldPos_ = fieldPos;
    this->pStrVal_ = pStr;
    this->strLen_ = strLen;
  }

  virtual ~LikeCondition()
  {}

  virtual bool GetLogic(const DBVal* pVals, size_t valCnt) const
  {
    if (!DBVAL_ELE_IS_STRING(pVals, fieldPos_))
      return false;

    const char* pCurStr = DBVAL_ELE_GET_STRING(pVals, fieldPos_);
    size_t curLen = DBVAL_ELE_GET_LEN(pVals, fieldPos_);

    return StringTool::Utf8LikeCompare(pStrVal_, pCurStr, curLen);
  }

private:
  size_t fieldPos_;
  const char* pStrVal_;
  size_t strLen_;
};