#include "query/value_item.h"
#include "expr/parse.h"
#include "query/group_field.h"
#include <math.h>


#define GET_LEFT_RIGHT_VALUE(leftType, rightType)   do {         \
  retVal = pLeft_->GetValue(pVals, &leftValue);                  \
  if (retVal != PdbE_OK)                                         \
    return retVal;                                               \
                                                                 \
  retVal = pRight_->GetValue(pVals, &rightValue);                \
  if (retVal != PdbE_OK)                                         \
    return retVal;                                               \
                                                                 \
  if (pResult == nullptr)                                        \
    return PdbE_OK;                                              \
                                                                 \
  if (DBVAL_GET_TYPE(&leftValue) != (leftType)                   \
    || DBVAL_GET_TYPE(&rightValue) != (rightType))               \
  {                                                              \
    DBVAL_SET_NULL(pResult);                                     \
    return PdbE_OK;                                              \
  }                                                              \
}while(false)

#define RIGHT_VALUE_NOT_ZERO(rightType)            do {          \
  if ((rightType) == PDB_VALUE_TYPE::VAL_INT64)                  \
  {                                                              \
    if (DBVAL_GET_INT64(&rightValue) == 0 )                      \
    {                                                            \
      DBVAL_SET_NULL(pResult);                                   \
      return PdbE_OK;                                            \
    }                                                            \
  }                                                              \
                                                                 \
  if ((rightType) == PDB_VALUE_TYPE::VAL_DOUBLE)                 \
  {                                                              \
    if (DOUBLE_EQUAL_ZERO(DBVAL_GET_DOUBLE(&rightValue)))        \
    {                                                            \
      DBVAL_SET_NULL(pResult);                                   \
      return PdbE_OK;                                            \
    }                                                            \
  }                                                              \
} while(false)

#define CONVERT_LEFT_RIGHT_VALUE_TO_DOUBLE(leftType, rightType) do {  \
  if ((leftType) == PDB_VALUE_TYPE::VAL_INT64) {                      \
    leftDouble = static_cast<double>(DBVAL_GET_INT64(&leftValue));    \
  } else {                                                            \
    leftDouble = DBVAL_GET_DOUBLE(&leftValue);                        \
  }                                                                   \
                                                                      \
  if ((rightType) == PDB_VALUE_TYPE::VAL_INT64) {                     \
    rightDouble = static_cast<double>(DBVAL_GET_INT64(&rightValue));  \
  } else {                                                            \
    rightDouble = DBVAL_GET_DOUBLE(&rightValue);                      \
  }                                                                   \
} while(false)

#define VALID_ARITUMETIC_FUNC(leftType, rightType)          do { \
  if (pLeft_ == nullptr || pRight_ == nullptr)                   \
    return false;                                                \
                                                                 \
  if (!pLeft_->IsValid())                                        \
    return false;                                                \
                                                                 \
  if (!pRight_->IsValid())                                       \
    return false;                                                \
                                                                 \
  if ((leftType) != PDB_VALUE_TYPE::VAL_INT64                    \
    && (leftType) != PDB_VALUE_TYPE::VAL_DOUBLE)                 \
  {                                                              \
    return false;                                                \
  }                                                              \
                                                                 \
  if ((rightType) != PDB_VALUE_TYPE::VAL_INT64                   \
    && (rightType) != PDB_VALUE_TYPE::VAL_DOUBLE)                \
  {                                                              \
    return false;                                                \
  }                                                              \
                                                                 \
  if (pLeft_->GetValueType() != (leftType))                      \
    return false;                                                \
                                                                 \
  if (pRight_->GetValueType() != (rightType))                    \
    return false;                                                \
} while(false)


//加法函数
template<int LeftType, int RightType>
class AddFunction : public ValueItem
{
public:
  AddFunction(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~AddFunction()
  {
    if (pLeft_ != nullptr)
      delete pLeft_;
    if (pRight_ != nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override
  {
    return (LeftType == PDB_VALUE_TYPE::VAL_INT64 && RightType == PDB_VALUE_TYPE::VAL_INT64) ?
      PDB_VALUE_TYPE::VAL_INT64 : PDB_VALUE_TYPE::VAL_DOUBLE;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    double leftDouble, rightDouble;
    DBVal leftValue, rightValue;
    GET_LEFT_RIGHT_VALUE(LeftType, RightType);

    if constexpr (LeftType == PDB_VALUE_TYPE::VAL_INT64 && RightType == PDB_VALUE_TYPE::VAL_INT64)
    {
      DBVAL_SET_INT64(pResult, (DBVAL_GET_INT64(&leftValue) + DBVAL_GET_INT64(&rightValue)));
    }
    else
    {
      CONVERT_LEFT_RIGHT_VALUE_TO_DOUBLE(LeftType, RightType);
      DBVAL_SET_DOUBLE(pResult, (leftDouble + rightDouble));
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    VALID_ARITUMETIC_FUNC(LeftType, RightType);
    return true;
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

protected:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

//减法函数
template<int LeftType, int RightType>
class SubFunction : public ValueItem
{
public:
  SubFunction(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~SubFunction()
  {
    if (pLeft_ != nullptr)
      delete pLeft_;
    if (pRight_ != nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override
  {
    return (LeftType == PDB_VALUE_TYPE::VAL_INT64 && RightType == PDB_VALUE_TYPE::VAL_INT64) ?
      PDB_VALUE_TYPE::VAL_INT64 : PDB_VALUE_TYPE::VAL_DOUBLE;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    double leftDouble, rightDouble;
    DBVal leftValue, rightValue;
    GET_LEFT_RIGHT_VALUE(LeftType, RightType);

    if constexpr (LeftType == PDB_VALUE_TYPE::VAL_INT64 && RightType == PDB_VALUE_TYPE::VAL_INT64)
    {
      DBVAL_SET_INT64(pResult, (DBVAL_GET_INT64(&leftValue) - DBVAL_GET_INT64(&rightValue)));
    }
    else
    {
      CONVERT_LEFT_RIGHT_VALUE_TO_DOUBLE(LeftType, RightType);
      DBVAL_SET_DOUBLE(pResult, (leftDouble - rightDouble));
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    VALID_ARITUMETIC_FUNC(LeftType, RightType);
    return true;
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

protected:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

//乘法函数
template<int LeftType, int RightType>
class MulFunction : public ValueItem
{
public:
  MulFunction(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~MulFunction()
  {
    if (pLeft_ == nullptr)
      delete pLeft_;
    if (pRight_ == nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override
  {
    return (LeftType == PDB_VALUE_TYPE::VAL_INT64 && RightType == PDB_VALUE_TYPE::VAL_INT64) ?
      PDB_VALUE_TYPE::VAL_INT64 : PDB_VALUE_TYPE::VAL_DOUBLE;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    double leftDouble, rightDouble;
    DBVal leftValue, rightValue;
    GET_LEFT_RIGHT_VALUE(LeftType, RightType);

    if constexpr (LeftType == PDB_VALUE_TYPE::VAL_INT64 && RightType == PDB_VALUE_TYPE::VAL_INT64)
    {
      DBVAL_SET_INT64(pResult, (DBVAL_GET_INT64(&leftValue) * DBVAL_GET_INT64(&rightValue)));
    }
    else
    {
      CONVERT_LEFT_RIGHT_VALUE_TO_DOUBLE(LeftType, RightType);
      DBVAL_SET_DOUBLE(pResult, (leftDouble * rightDouble));
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    VALID_ARITUMETIC_FUNC(LeftType, RightType);
    return true;
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

protected:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

//除法函数
template<int LeftType, int RightType>
class DivFunction : public ValueItem
{
public:
  DivFunction(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~DivFunction()
  {
    if (pLeft_ == nullptr)
      delete pLeft_;
    if (pRight_ == nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override
  {
    return (LeftType == PDB_VALUE_TYPE::VAL_INT64 && RightType == PDB_VALUE_TYPE::VAL_INT64) ?
      PDB_VALUE_TYPE::VAL_INT64 : PDB_VALUE_TYPE::VAL_DOUBLE;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal leftValue, rightValue;
    GET_LEFT_RIGHT_VALUE(LeftType, RightType);
    RIGHT_VALUE_NOT_ZERO(RightType);

    if constexpr (LeftType == PDB_VALUE_TYPE::VAL_INT64 && RightType == PDB_VALUE_TYPE::VAL_INT64)
    {
      DBVAL_SET_INT64(pResult, (DBVAL_GET_INT64(&leftValue) / DBVAL_GET_INT64(&rightValue)));
    }
    else
    {
      double leftDouble, rightDouble;
      CONVERT_LEFT_RIGHT_VALUE_TO_DOUBLE(LeftType, RightType);
      DBVAL_SET_DOUBLE(pResult, (leftDouble / rightDouble));
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    VALID_ARITUMETIC_FUNC(LeftType, RightType);
    return true;
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

protected:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

//求模
template<int LeftType, int RightType>
class ModFunction : public ValueItem
{
public:
  ModFunction(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~ModFunction()
  {
    if (pLeft_ == nullptr)
      delete pLeft_;
    if (pRight_ == nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override
  {
    return (LeftType == PDB_VALUE_TYPE::VAL_INT64 && RightType == PDB_VALUE_TYPE::VAL_INT64) ?
      PDB_VALUE_TYPE::VAL_INT64 : PDB_VALUE_TYPE::VAL_DOUBLE;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal leftValue, rightValue;
    GET_LEFT_RIGHT_VALUE(LeftType, RightType);
    RIGHT_VALUE_NOT_ZERO(RightType);

    if constexpr (LeftType == PDB_VALUE_TYPE::VAL_INT64 && RightType == PDB_VALUE_TYPE::VAL_INT64)
    {
      DBVAL_SET_INT64(pResult, (DBVAL_GET_INT64(&leftValue) % DBVAL_GET_INT64(&rightValue)));
    }
    else
    {
      double leftDouble, rightDouble;
      CONVERT_LEFT_RIGHT_VALUE_TO_DOUBLE(LeftType, RightType);
#ifdef _WIN32
      double resultDobule = std::fmod(leftDouble, rightDouble);
#else
      double resultDobule = fmod(leftDouble, rightDouble);
#endif
      DBVAL_SET_DOUBLE(pResult, resultDobule);
    }
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    VALID_ARITUMETIC_FUNC(LeftType, RightType);
    return true;
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

protected:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

bool ConvertStringToDateTime(ValueItem** ppValue)
{
  if (ppValue == nullptr)
    return false;

  if (!(*ppValue)->IsConstValue())
    return false;

  DBVal strVal;
  if ((*ppValue)->GetValue(nullptr, &strVal) != PdbE_OK)
    return false;

  if (!DBVAL_IS_STRING(&strVal))
    return false;

  DateTime dt;
  if (dt.Parse(DBVAL_GET_STRING(&strVal), DBVAL_GET_LEN(&strVal)))
  {
    ValueItem* pNewItem = new ConstValue(dt.TotalMilliseconds(), true);
    delete* ppValue;
    *ppValue = pNewItem;
    return true;
  }

  return false;
}

//时间 加
class DateTimeAdd : public ValueItem
{
public:
  DateTimeAdd(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~DateTimeAdd()
  {
    if (pLeft_ == nullptr)
      delete pLeft_;
    if (pRight_ == nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override { return PDB_VALUE_TYPE::VAL_DATETIME; }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal leftValue, rightValue;
    retVal = pLeft_->GetValue(pVals, &leftValue);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pRight_->GetValue(pVals, &rightValue);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_GET_TYPE(&leftValue) != PDB_VALUE_TYPE::VAL_DATETIME
      || DBVAL_GET_TYPE(&rightValue) != PDB_VALUE_TYPE::VAL_INT64)
    {
      DBVAL_SET_NULL(pResult);
      return PdbE_OK;
    }

    int64_t dtVal = DBVAL_GET_DATETIME(&leftValue) + DBVAL_GET_INT64(&rightValue);
    if (dtVal >= MinMillis && dtVal <= MaxMillis)
    {
      DBVAL_SET_DATETIME(pResult, dtVal);
    }
    else
    {
      DBVAL_SET_NULL(pResult);
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pLeft_ == nullptr || pRight_ == nullptr)
      return false;

    if (!pLeft_->IsValid())
      return false;

    if (!pRight_->IsValid())
      return false;

    return (pLeft_->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME
      && pRight_->GetValueType() == PDB_VALUE_TYPE::VAL_INT64);
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

  //调整值，string 转 datetime 等
  void AdjustDateTime()
  {
    if (pLeft_ == nullptr || pRight_ == nullptr)
      return;

    if (pLeft_->GetValueType() == PDB_VALUE_TYPE::VAL_STRING)
    {
      ConvertStringToDateTime(&pLeft_);
    }

    if (pRight_->GetValueType() == PDB_VALUE_TYPE::VAL_STRING)
    {
      ConvertStringToDateTime(&pRight_);
    }

    if (pLeft_->GetValueType() == PDB_VALUE_TYPE::VAL_INT64
      && pRight_->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME)
    {
      ValueItem* pTmp = pRight_;
      pRight_ = pLeft_;
      pLeft_ = pTmp;
    }
  }

protected:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

//时间 差
class DateTimeDiff : public ValueItem
{
public:
  DateTimeDiff(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~DateTimeDiff()
  {
    if (pLeft_ == nullptr)
      delete pLeft_;
    if (pRight_ == nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override { return PDB_VALUE_TYPE::VAL_INT64; }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal leftValue, rightValue;
    retVal = pLeft_->GetValue(pVals, &leftValue);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pRight_->GetValue(pVals, &rightValue);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_GET_TYPE(&leftValue) != PDB_VALUE_TYPE::VAL_DATETIME
      || DBVAL_GET_TYPE(&rightValue) != PDB_VALUE_TYPE::VAL_DATETIME)
    {
      DBVAL_SET_NULL(pResult);
      return PdbE_OK;
    }

    int64_t diffVal = DBVAL_GET_DATETIME(&leftValue) - DBVAL_GET_DATETIME(&rightValue);
    DBVAL_SET_INT64(pResult, diffVal);
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pLeft_ == nullptr || pRight_ == nullptr)
      return false;

    if (!pLeft_->IsValid())
      return false;

    if (!pRight_->IsValid())
      return false;

    return (pLeft_->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME
      && pRight_->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME);
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

  void AjustValueType()
  {
    if (pLeft_ == nullptr || pRight_ == nullptr)
      return;

    if (pLeft_->GetValueType() == PDB_VALUE_TYPE::VAL_STRING)
    {
      ConvertStringToDateTime(&pLeft_);
    }

    if (pRight_->GetValueType() == PDB_VALUE_TYPE::VAL_STRING)
    {
      ConvertStringToDateTime(&pRight_);
    }
  }

protected:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

//时间向下取整
class DateTimeFloor : public ValueItem
{
public:
  DateTimeFloor(ValueItem* pValue, int64_t millis)
  {
    pTimeValue_ = pValue;
    millis_ = millis;
  }

  virtual ~DateTimeFloor()
  {
    if (pTimeValue_ != nullptr)
      delete pTimeValue_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_DATETIME;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pTimeValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (!DBVAL_IS_DATETIME(&tmpVal))
    {
      DBVAL_SET_NULL(pResult);
      return PdbE_OK;
    }

    int64_t dtVal = DBVAL_GET_DATETIME(&tmpVal);
    if (millis_ > 0)
    {
      dtVal -= (dtVal % millis_);
      if (millis_ == MillisPerDay)
      {
        dtVal += DateTime::GetSysTimeZone();
      }
    }

    DBVAL_SET_DATETIME(pResult, dtVal);
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pTimeValue_ == nullptr)
      return false;

    if (!pTimeValue_->IsValid())
      return false;

    if (millis_ < 0)
      return false;

    return pTimeValue_->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME;
  }

  bool IsConstValue() const override
  {
    return pTimeValue_->IsConstValue();
  }

  void AjustDateTime()
  {
    if (pTimeValue_ == nullptr)
      return;

    if (pTimeValue_->GetValueType() == PDB_VALUE_TYPE::VAL_STRING)
    {
      ConvertStringToDateTime(&pTimeValue_);
    }
  }

private:
  ValueItem* pTimeValue_;
  int64_t millis_;
};

//时间向上取整
class DateTimeCeil : public ValueItem
{
public:
  DateTimeCeil(ValueItem* pValue, int64_t millis)
  {
    pTimeValue_ = pValue;
    millis_ = millis;
  }

  virtual ~DateTimeCeil()
  {
    if (pTimeValue_ != nullptr)
      delete pTimeValue_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_DATETIME;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pTimeValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (!DBVAL_IS_DATETIME(&tmpVal))
    {
      DBVAL_SET_NULL(pResult);
      return PdbE_OK;
    }

    int64_t dtVal = DBVAL_GET_DATETIME(&tmpVal);
    if (millis_ > 0)
    {
      dtVal += (millis_ - 1);
      dtVal -= (dtVal % millis_);
      if (millis_ == MillisPerDay)
      {
        dtVal += DateTime::GetSysTimeZone();
      }
    }

    DBVAL_SET_DATETIME(pResult, dtVal);
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pTimeValue_ == nullptr)
      return false;

    if (!pTimeValue_->IsValid())
      return false;

    if (millis_ < 0)
      return false;

    return pTimeValue_->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME;
  }

  bool IsConstValue() const override
  {
    return pTimeValue_->IsConstValue();
  }

  void AjustDateTime()
  {
    if (pTimeValue_ == nullptr)
      return;

    if (pTimeValue_->GetValueType() == PDB_VALUE_TYPE::VAL_STRING)
    {
      ConvertStringToDateTime(&pTimeValue_);
    }
  }

private:
  ValueItem* pTimeValue_;
  int64_t millis_;
};

//绝对值函数
class AbsFunction : public ValueItem
{
public:
  AbsFunction(ValueItem* pValue)
  {
    this->pValue_ = pValue;
  }

  virtual ~AbsFunction()
  {
    if (pValue_ != nullptr)
      delete pValue_;
  }

  int32_t GetValueType() const override
  {
    return pValue_->GetValueType();
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_IS_INT64(&tmpVal))
    {
#ifdef _WIN32
      DBVAL_SET_INT64(pResult, std::abs(DBVAL_GET_INT64(&tmpVal)));
#else
      DBVAL_SET_INT64(pResult, abs(DBVAL_GET_INT64(&tmpVal)));
#endif
    }
    else if (DBVAL_IS_DOUBLE(&tmpVal))
    {
#ifdef _WIN32
      DBVAL_SET_DOUBLE(pResult, std::abs(DBVAL_GET_DOUBLE(&tmpVal)));
#else
      DBVAL_SET_DOUBLE(pResult, fabs(DBVAL_GET_DOUBLE(&tmpVal)));
#endif
    }
    else
    {
      DBVAL_SET_NULL(pResult);
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pValue_ == nullptr)
      return false;

    int32_t resultType = pValue_->GetValueType();

    return resultType == PDB_VALUE_TYPE::VAL_INT64 || resultType == PDB_VALUE_TYPE::VAL_DOUBLE;
  }

  bool IsConstValue() const override
  {
    return pValue_->IsConstValue();
  }

private:
  ValueItem* pValue_;
};

class IfFunction : public ValueItem
{
public:
  IfFunction(ValueItem* pCondition, ValueItem* pResult0, ValueItem* pResult1)
  {
    this->pCondition_ = pCondition;
    this->pResult0_ = pResult0;
    this->pResult1_ = pResult1;
    this->resultType_ = PDB_VALUE_TYPE::VAL_NULL;
  }

  virtual ~IfFunction()
  {
    if (pCondition_ != nullptr)
      delete pCondition_;

    if (pResult0_ != nullptr)
      delete pResult0_;

    if (pResult1_ != nullptr)
      delete pResult1_;
  }

  int32_t GetValueType() const override
  {
    return resultType_;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal condiVal, rst0Val, rst1Val;
    DBVal* pRst;
    retVal = pCondition_->GetValue(pVals, &condiVal);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pResult0_->GetValue(pVals, &rst0Val);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pResult1_->GetValue(pVals, &rst1Val);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    pRst = (DBVAL_IS_BOOL(&condiVal) && DBVAL_GET_BOOL(&condiVal)) ? &rst0Val : &rst1Val;

    if (DBVAL_GET_TYPE(pRst) == resultType_)
    {
      *pResult = *pRst;
      return PdbE_OK;
    }

    if (DBVAL_GET_TYPE(pRst) == PDB_VALUE_TYPE::VAL_INT64 && resultType_ == PDB_VALUE_TYPE::VAL_DOUBLE)
    {
      DBVAL_SET_DOUBLE(pResult, static_cast<double>(DBVAL_GET_INT64(pRst)));
      return PdbE_OK;
    }

    DBVAL_SET_NULL(pResult);
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pCondition_ == nullptr || pResult0_ == nullptr || pResult1_ == nullptr)
      return false;

    if (!pCondition_->IsValid())
      return false;

    if (!pResult0_->IsValid())
      return false;

    if (!pResult1_->IsValid())
      return false;

    if (pCondition_->GetValueType() != PDB_VALUE_TYPE::VAL_BOOL)
      return false;

    if (resultType_ == PDB_VALUE_TYPE::VAL_NULL)
      return false;

    return true;
  }

  bool IsConstValue() const override
  {
    return pCondition_->IsConstValue()
      && pResult0_->IsConstValue() && pResult1_->IsConstValue();
  }

  void AdjustDateTime()
  {
    int rst0Type = pResult0_->GetValueType();
    int rst1Type = pResult1_->GetValueType();

    if (rst0Type == PDB_VALUE_TYPE::VAL_STRING && rst1Type == PDB_VALUE_TYPE::VAL_DATETIME)
    {
      ConvertStringToDateTime(&pResult0_);
    }
    else if (rst0Type == PDB_VALUE_TYPE::VAL_DATETIME && rst1Type == PDB_VALUE_TYPE::VAL_STRING)
    {
      ConvertStringToDateTime(&pResult1_);
    }

    rst0Type = pResult0_->GetValueType();
    rst1Type = pResult1_->GetValueType();

    if (rst0Type == rst1Type)
    {
      resultType_ = rst0Type;
    }
    else if (rst0Type == PDB_VALUE_TYPE::VAL_INT64 && rst1Type == PDB_VALUE_TYPE::VAL_DOUBLE)
    {
      resultType_ = PDB_VALUE_TYPE::VAL_DOUBLE;
    }
    else if (rst1Type == PDB_VALUE_TYPE::VAL_INT64 && rst0Type == PDB_VALUE_TYPE::VAL_DOUBLE)
    {
      resultType_ = PDB_VALUE_TYPE::VAL_DOUBLE;
    }
  }

private:
  int32_t resultType_;
  ValueItem* pCondition_;
  ValueItem* pResult0_;
  ValueItem* pResult1_;
};

//isnull
class IsNullFunction : public ValueItem
{
public:
  IsNullFunction(ValueItem* pValue)
  {
    pValue_ = pValue;
  }

  virtual ~IsNullFunction()
  {
    if (pValue_ != nullptr)
      delete pValue_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    DBVAL_SET_BOOL(pResult, DBVAL_IS_NULL(&tmpVal));
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pValue_ == nullptr)
      return false;

    return pValue_->IsValid();
  }

  bool IsConstValue() const override
  {
    return pValue_->IsConstValue();
  }

private:
  ValueItem* pValue_;
};

//isnotnull
class IsNotNullFunction : public ValueItem
{
public:
  IsNotNullFunction(ValueItem* pValue)
  {
    pValue_ = pValue;
  }

  virtual ~IsNotNullFunction()
  {
    if (pValue_ != nullptr)
      delete pValue_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    DBVAL_SET_BOOL(pResult, !(DBVAL_IS_NULL(&tmpVal)));
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pValue_ == nullptr)
      return false;

    return pValue_->IsValid();
  }

  bool IsConstValue() const override
  {
    return pValue_->IsConstValue();
  }

private:
  ValueItem* pValue_;
};

class LikeFunction : public ValueItem
{
public:
  LikeFunction(ValueItem* pValue, const char* pPattern, size_t patternLen)
  {
    this->pValue_ = pValue;
    this->patternStr_ = std::string(pPattern, patternLen);
  }

  virtual ~LikeFunction()
  {
    if (pValue_ != nullptr)
      delete pValue_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (!DBVAL_IS_STRING(&tmpVal))
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    const char* pStrValue = DBVAL_GET_STRING(&tmpVal);
    size_t strLen = DBVAL_GET_LEN(&tmpVal);

    DBVAL_SET_BOOL(pResult, StringTool::Utf8LikeCompare(patternStr_.c_str(), pStrValue, strLen));
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pValue_ == nullptr)
      return false;

    return pValue_->IsValid();
  }

  bool IsConstValue() const override
  {
    return pValue_->IsConstValue();
  }

private:
  ValueItem* pValue_;
  std::string patternStr_;
};

class InFunction : public ValueItem
{
public:
  InFunction(ValueItem* pValue, const std::list<int64_t>& valList)
  {
    pValue_ = pValue;
    for (auto valIt = valList.begin(); valIt != valList.end(); valIt++)
    {
      if (valSet_.find(*valIt) == valSet_.end())
      {
        valSet_.insert(*valIt);
      }
    }
  }

  virtual ~InFunction()
  {
    if (pValue_ != nullptr)
      delete pValue_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (!DBVAL_IS_INT64(&tmpVal))
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if (valSet_.find(DBVAL_GET_INT64(&tmpVal)) != valSet_.end())
    {
      DBVAL_SET_BOOL(pResult, true);
    }
    else
    {
      DBVAL_SET_BOOL(pResult, false);
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pValue_ == nullptr)
      return false;

    if (!pValue_->IsValid())
      return false;

    return pValue_->GetValueType() == PDB_VALUE_TYPE::VAL_INT64;
  }

  bool IsConstValue() const override
  {
    return pValue_->IsConstValue();
  }

private:
  ValueItem* pValue_;
  std::unordered_set<int64_t> valSet_;
};

class NotInFunction : public ValueItem
{
public:
  NotInFunction(ValueItem* pValue, const std::list<int64_t>& valList)
  {
    pValue_ = pValue;
    for (auto valIt = valList.begin(); valIt != valList.end(); valIt++)
    {
      if (valSet_.find(*valIt) == valSet_.end())
      {
        valSet_.insert(*valIt);
      }
    }
  }

  virtual ~NotInFunction()
  {
    if (pValue_ != nullptr)
      delete pValue_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal tmpVal;
    retVal = pValue_->GetValue(pVals, &tmpVal);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (!DBVAL_IS_INT64(&tmpVal))
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if (valSet_.find(DBVAL_GET_INT64(&tmpVal)) == valSet_.end())
    {
      DBVAL_SET_BOOL(pResult, true);
    }
    else
    {
      DBVAL_SET_BOOL(pResult, false);
    }

    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pValue_ == nullptr)
      return false;

    if (!pValue_->IsValid())
      return false;

    return pValue_->GetValueType() == PDB_VALUE_TYPE::VAL_INT64;
  }

  bool IsConstValue() const override
  {
    return pValue_->IsConstValue();
  }

private:
  ValueItem* pValue_;
  std::unordered_set<int64_t> valSet_;
};


#define VALID_LOGIC_FUNC(leftType, rightType)               do { \
  if (pLeft_ == nullptr || pRight_ == nullptr)                   \
    return false;                                                \
                                                                 \
  if (!pLeft_->IsValid())                                        \
    return false;                                                \
                                                                 \
  if (!pRight_->IsValid())                                       \
    return false;                                                \
                                                                 \
  if (pLeft_->GetValueType() != (leftType))                      \
    return false;                                                \
                                                                 \
  if (pRight_->GetValueType() != (rightType))                    \
    return false;                                                \
} while(false)

template<int LeftType, int RightType> inline
bool DBValEqual(DBVal* pLeft, DBVal* pRight)
{
  if (LeftType == PDB_FIELD_TYPE::TYPE_BOOL)
}


template<int LeftType, int RightType, typename T, int CompOp>
class ValueCompareFunction : public ValueItem
{
public:
  ValueCompareFunction(ValueItem* pLeft, ValueItem* pRight)
  {
    this->pLeft_ = pLeft;
    this->pRight_ = pRight;
  }

  virtual ~ValueCompareFunction()
  {
    if (pLeft_ == nullptr)
      delete pLeft_;
    if (pRight_ == nullptr)
      delete pRight_;
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    PdbErr_t retVal;
    DBVal leftValue, rightValue;
    bool result = false;

    retVal = pLeft_->GetValue(pVals, &leftValue);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = pRight_->GetValue(pVals, &rightValue);
    if (retVal != PdbE_OK)
      return retVal;

    if (pResult == nullptr)
      return PdbE_OK;

    if (DBVAL_GET_TYPE(&leftValue) != LeftType || DBVAL_GET_TYPE(&rightValue) != RightType)
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if constexpr (LeftType == RightType && LeftType == PDB_FIELD_TYPE::TYPE_STRING)
    {
      if constexpr (CompOp == TK_EQ)
      {
        if (DBVAL_GET_LEN(&leftValue) == DBVAL_GET_LEN(&rightValue))
          result = strncmp(DBVAL_GET_STRING(&leftValue), DBVAL_GET_STRING(&rightValue), DBVAL_GET_LEN(&leftValue)) == 0;
      }
      else if constexpr (CompOp == TK_NE)
      {
        if (DBVAL_GET_LEN(&leftValue) != DBVAL_GET_LEN(&rightValue))
          result = true;
        else
          result = strncmp(DBVAL_GET_STRING(&leftValue), DBVAL_GET_STRING(&rightValue), DBVAL_GET_LEN(&leftValue)) != 0;
      }
    }
    else if constexpr (LeftType == RightType && LeftType == PDB_FIELD_TYPE::TYPE_BOOL)
    {
      if constexpr (CompOp == TK_EQ)
        result = DBVAL_GET_BOOL(&leftValue) == DBVAL_GET_BOOL(&rightValue);
      else
        result = DBVAL_GET_BOOL(&leftValue) != DBVAL_GET_BOOL(&rightValue);
    }
    else
    {
      T lv;
      T rv;
      if constexpr (LeftType == PDB_FIELD_TYPE::TYPE_INT64)
        lv = DBVAL_GET_INT64(&leftValue);
      else if constexpr (LeftType == PDB_FIELD_TYPE::TYPE_DATETIME)
        lv = DBVAL_GET_DATETIME(&leftValue);
      else if constexpr (LeftType == PDB_FIELD_TYPE::TYPE_DOUBLE)
        lv = DBVAL_GET_DOUBLE(&leftValue);

      if constexpr (RightType == PDB_FIELD_TYPE::TYPE_INT64)
        rv = DBVAL_GET_INT64(&rightValue);
      else if constexpr (RightType == PDB_FIELD_TYPE::TYPE_DATETIME)
        rv = DBVAL_GET_DATETIME(&rightValue);
      else if constexpr (RightType == PDB_FIELD_TYPE::TYPE_DOUBLE)
        rv = DBVAL_GET_DOUBLE(&rightValue);

      if constexpr (CompOp == TK_LT)
        result = lv < rv;
      else if constexpr (CompOp == TK_LE)
        result = lv <= rv;
      else if constexpr (CompOp == TK_GT)
        result = lv > rv;
      else if constexpr (CompOp == TK_GE)
        result = lv >= rv;
      else if constexpr (CompOp == TK_EQ)
        result = lv == rv;
      else if constexpr (CompOp == TK_NE)
        result = lv != rv;
    }

    DBVAL_SET_BOOL(pResult, result);
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (pLeft_->GetValueType() != LeftType)
      return false;

    if (pRight_->GetValueType() != RightType)
      return false;

    if (CompOp != TK_LT && CompOp != TK_LE
      && CompOp != TK_GT && CompOp != TK_GE
      && CompOp != TK_EQ && CompOp != TK_NE)
      return false;

    if (LeftType == PDB_FIELD_TYPE::TYPE_BOOL
      || LeftType == PDB_FIELD_TYPE::TYPE_STRING
      || RightType == PDB_FIELD_TYPE::TYPE_BOOL
      || RightType == PDB_FIELD_TYPE::TYPE_STRING)
    {
      if (RightType != LeftType)
        return false;

      if (CompOp != TK_EQ && CompOp != TK_NE)
        return false;

      return true;
    }

    return (LeftType == PDB_FIELD_TYPE::TYPE_INT64 || LeftType == PDB_FIELD_TYPE::TYPE_DATETIME || LeftType == PDB_FIELD_TYPE::TYPE_DOUBLE)
      && (RightType == PDB_FIELD_TYPE::TYPE_INT64 || RightType == PDB_FIELD_TYPE::TYPE_DATETIME || RightType == PDB_FIELD_TYPE::TYPE_DOUBLE);
  }

  bool IsConstValue() const override
  {
    return pLeft_->IsConstValue() && pRight_->IsConstValue();
  }

private:
  ValueItem* pLeft_;
  ValueItem* pRight_;
};

template<int FieldType, typename T, int CompOp>
class FieldCompareFunction : public ValueItem
{
public:
  FieldCompareFunction(size_t fieldPos, T val)
  {
    this->fieldPos_ = fieldPos;
    this->val_ = val;
  }

  virtual ~FieldCompareFunction() {}

  int32_t GetValueType() const override { return PDB_VALUE_TYPE::VAL_BOOL; }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    if (pResult == nullptr)
      return PdbE_OK;

    bool result = false;
    if (!DBVAL_ELE_IS_TYPE(pVals, fieldPos_, FieldType))
    {
      DBVAL_SET_BOOL(pResult, false);
      return PdbE_OK;
    }

    if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_STRING)
    {
      if constexpr (CompOp == TK_EQ)
      {
        if (DBVAL_ELE_GET_LEN(pVals, fieldPos_) == val_.size())
          result = strncmp(DBVAL_ELE_GET_STRING(pVals, fieldPos_), val_.c_str(), val_.size()) == 0;
      }
      else if constexpr (CompOp == TK_NE)
      {
        if (DBVAL_ELE_GET_LEN(pVals, fieldPos_) != val_.size())
          result = true;
        else 
          result = strncmp(DBVAL_ELE_GET_STRING(pVals, fieldPos_), val_.c_str(), val_.size()) != 0;
      }
    }
    else if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_BOOL)
    {
      if constexpr (CompOp == TK_EQ)
        result = DBVAL_ELE_GET_BOOL(pVals, fieldPos_) == val_;
      else
        result = DBVAL_ELE_GET_BOOL(pVals, fieldPos_) != val_;
    }
    else
    {
      T lv;
      if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_INT64)
        lv = DBVAL_ELE_GET_INT64(pVals, fieldPos_);
      else if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_DATETIME)
        lv = DBVAL_ELE_GET_DATETIME(pVals, fieldPos_);
      else if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_DOUBLE)
        lv = DBVAL_ELE_GET_DOUBLE(pVals, fieldPos_);

      if constexpr (CompOp == TK_LT)
        result = lv < val_;
      else if constexpr (CompOp == TK_LE)
        result = lv <= val_;
      else if constexpr (CompOp == TK_GT)
        result = lv > val_;
      else if constexpr (CompOp == TK_GE)
        result = lv >= val_;
      else if constexpr (CompOp == TK_EQ)
        result = lv == val_;
      else if constexpr (CompOp == TK_NE)
        result = lv != val_;
    }

    DBVAL_SET_BOOL(pResult, result);
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (FieldType == PDB_FIELD_TYPE::TYPE_BOOL
      || FieldType == PDB_FIELD_TYPE::TYPE_STRING)
    {
      return CompOp == TK_EQ || CompOp == TK_NE;
    }

    if (CompOp != TK_LT && CompOp != TK_LE
      && CompOp != TK_GT && CompOp != TK_GE
      && CompOp != TK_EQ && CompOp != TK_NE)
      return false;

    return (FieldType == PDB_VALUE_TYPE::VAL_INT64
      || FieldType == PDB_VALUE_TYPE::VAL_DOUBLE
      || FieldType == PDB_VALUE_TYPE::VAL_DATETIME);
  }
  bool IsConstValue() const override { return false; }


  bool IsDevIdCondition() const override
  {
    return fieldPos_ == PDB_DEVID_INDEX && FieldType == PDB_VALUE_TYPE::VAL_INT64;
  }

  bool IsTstampCondition() const override
  {
    return fieldPos_ == PDB_TSTAMP_INDEX && FieldType == PDB_VALUE_TYPE::VAL_DATETIME;
  }

  bool GetDevIdRange(int64_t* pMinDevId, int64_t* pMaxDevId) const override
  {
    if (!IsDevIdCondition())
      return false;

    if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_INT64)
    {
      int64_t minId = 0;
      int64_t maxId = INT64_MAX;

      if constexpr (CompOp == TK_LT || CompOp == TK_LE)
      {
        maxId = val_;
      }
      else if constexpr (CompOp == TK_GT || CompOp == TK_GE)
      {
        minId = val_;
      }
      else if constexpr (CompOp == TK_EQ)
      {
        minId = val_;
        maxId = val_;
      }

      if (pMinDevId != nullptr)
        *pMinDevId = minId;

      if (pMaxDevId != nullptr)
        *pMaxDevId = maxId;

      return true;
    }

    return false;
  }

  bool GetTstampRange(int64_t* pMinTstamp, int64_t* pMaxTstamp) const override
  {
    if (!IsTstampCondition())
      return false;

    if constexpr (FieldType == PDB_FIELD_TYPE::TYPE_DATETIME)
    {
      int64_t minTs = 0;
      int64_t maxTs = INT64_MAX;

      if constexpr (CompOp == TK_LT || CompOp == TK_LE)
      {
        maxTs = val_;
      }
      else if constexpr (CompOp == TK_GT || CompOp == TK_GE)
      {
        minTs = val_;
      }
      else if constexpr (CompOp == TK_EQ)
      {
        minTs = val_;
        maxTs = val_;
      }

      if (pMinTstamp != nullptr)
        *pMinTstamp = minTs;

      if (pMaxTstamp != nullptr)
        *pMaxTstamp = maxTs;

      return true;
    }

    return false;
  }

private:
  size_t fieldPos_;
  T val_;
};

class AndFunction : public ValueItem
{
public:
  AndFunction() { }

  virtual ~AndFunction()
  {
    for (size_t idx = 0; idx < condiVec_.size(); idx++)
    {
      delete condiVec_[idx];
    }
  }

  int32_t GetValueType() const override
  {
    return PDB_VALUE_TYPE::VAL_BOOL;
  }

  PdbErr_t GetValue(const DBVal* pVals, DBVal* pResult) const override
  {
    DBVal tmpRet;
    PdbErr_t retVal = PdbE_OK;
    DBVAL_SET_BOOL(&tmpRet, true);
    for (auto condiIt = condiVec_.begin(); condiIt != condiVec_.end(); condiIt++)
    {
      retVal = (*condiIt)->GetValue(pVals, &tmpRet);
      if (retVal != PdbE_OK)
        return retVal;

      if (!DBVAL_GET_BOOL(&tmpRet))
        break;
    }

    *pResult = tmpRet;
    return PdbE_OK;
  }

  bool IsValid() const override
  {
    if (condiVec_.size() == 0)
      return false;

    for (auto condiIt = condiVec_.begin(); condiIt != condiVec_.end(); condiIt++)
    {
      if (!(*condiIt)->IsValid())
        return false;

      if ((*condiIt)->GetValueType() != PDB_VALUE_TYPE::VAL_BOOL)
        return false;
    }

    return true;
  }

  bool IsConstValue() const override
  {
    for (auto condiIt = condiVec_.begin(); condiIt != condiVec_.end(); condiIt++)
    {
      if (!(*condiIt)->IsConstValue())
        return false;
    }

    return true;
  }

  void AddValueItem(ValueItem* pItem)
  {
    if (pItem != nullptr)
    {
      condiVec_.push_back(pItem);
    }
  }

private:
  std::vector<ValueItem*> condiVec_;
};

int GetFunctionId(const char* pName, size_t nameLen)
{
  if (pName == nullptr || nameLen < 2 || nameLen > 20)
    return 0;

  if (pName[0] == 'A' || pName[0] == 'a')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "AVG", (sizeof("AVG") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_AVG;
    if (StringTool::ComparyNoCase(pName, nameLen, "AVGIF", (sizeof("AVGIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_AVG_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "ABS", (sizeof("ABS") - 1)))
      return PDB_SQL_FUNC::FUNC_ABS;
    else if (StringTool::ComparyNoCase(pName, nameLen, "ADD", (sizeof("ADD") - 1)))
      return PDB_SQL_FUNC::FUNC_ADD;
  }
  else if (pName[0] == 'D' || pName[0] == 'd')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "DIV", (sizeof("DIV") - 1)))
      return PDB_SQL_FUNC::FUNC_DIV;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMEADD", (sizeof("DATETIMEADD") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMEADD;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMEDIFF", (sizeof("DATETIMEDIFF") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMEDIFF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMEFLOOR", (sizeof("DATETIMEFLOOR") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMEFLOOR;
    else if (StringTool::ComparyNoCase(pName, nameLen, "DATETIMECEIL", (sizeof("DATETIMECEIL") - 1)))
      return PDB_SQL_FUNC::FUNC_DATETIMECEIL;
  }
  else if (pName[0] == 'M' || pName[0] == 'm')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "MIN", (sizeof("MIN") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MIN;
    if (StringTool::ComparyNoCase(pName, nameLen, "MINIF", (sizeof("MINIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MIN_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MAX", (sizeof("MAX") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MAX;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MAXIF", (sizeof("MAXIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_MAX_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MUL", (sizeof("MUL") - 1)))
      return PDB_SQL_FUNC::FUNC_MUL;
    else if (StringTool::ComparyNoCase(pName, nameLen, "MOD", (sizeof("MOD") - 1)))
      return PDB_SQL_FUNC::FUNC_MOD;
  }
  else if (pName[0] == 'S' || pName[0] == 's')
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "SUM", (sizeof("SUM") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_SUM;
    if (StringTool::ComparyNoCase(pName, nameLen, "SUMIF", (sizeof("SUMIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_SUM_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "SUB", (sizeof("SUB") - 1)))
      return PDB_SQL_FUNC::FUNC_SUB;
  }
  else
  {
    if (StringTool::ComparyNoCase(pName, nameLen, "COUNT", (sizeof("COUNT") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_COUNT;
    if (StringTool::ComparyNoCase(pName, nameLen, "COUNTIF", (sizeof("COUNTIF") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_COUNT_IF;
    else if (StringTool::ComparyNoCase(pName, nameLen, "FIRST", (sizeof("FIRST") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_FIRST;
    else if (StringTool::ComparyNoCase(pName, nameLen, "LAST", (sizeof("LAST") - 1)))
      return PDB_SQL_FUNC::FUNC_AGG_LAST;
    else if (StringTool::ComparyNoCase(pName, nameLen, "NOW", (sizeof("NOW") - 1)))
      return PDB_SQL_FUNC::FUNC_NOW;
    else if (StringTool::ComparyNoCase(pName, nameLen, "IF", (sizeof("IF") - 1)))
      return PDB_SQL_FUNC::FUNC_IF;
  }

  return 0;
}

void DeleteValueVec(std::vector<ValueItem*>* pValVec)
{
  for (size_t idx = 0; idx < pValVec->size(); idx++)
  {
    delete (*pValVec)[idx];
  }
}

bool ConvertExprToValue(const TableInfo* pTabInfo, int64_t nowMillis,
  const ExprValueList* pArgList, std::vector<ValueItem*>* pValVec)
{
  if (pArgList == nullptr)
    return true;

  const std::vector<ExprValue*>* pExprVec = pArgList->GetValueList();
  for (auto argIt = pExprVec->begin(); argIt != pExprVec->end(); argIt++)
  {
    ValueItem* pTmpVal = BuildGeneralValueItem(pTabInfo, *argIt, nowMillis);
    if (pTmpVal == nullptr)
      break;

    pValVec->push_back(pTmpVal);
  }

  if (pValVec->size() != pExprVec->size())
  {
    DeleteValueVec(pValVec);
    return false;
  }

  return true;
}

template<int LeftType, int RightType>
ValueItem* Create2ParamOpFunctionStep(int op, ValueItem* pLeftValue, ValueItem* pRightValue)
{
  switch (op)
  {
  case PDB_SQL_FUNC::FUNC_ADD:
    return new AddFunction<LeftType, RightType>(pLeftValue, pRightValue);
  case PDB_SQL_FUNC::FUNC_SUB:
    return new SubFunction<LeftType, RightType>(pLeftValue, pRightValue);
  case PDB_SQL_FUNC::FUNC_MUL:
    return new MulFunction<LeftType, RightType>(pLeftValue, pRightValue);
  case PDB_SQL_FUNC::FUNC_DIV:
    return new DivFunction<LeftType, RightType>(pLeftValue, pRightValue);
  case PDB_SQL_FUNC::FUNC_MOD:
    return new ModFunction<LeftType, RightType>(pLeftValue, pRightValue);
  }

  return nullptr;
}

ValueItem* Create2ParamOpFunction(int op, const TableInfo* pTabInfo, int64_t nowMillis, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
    return nullptr;

  if (!ConvertExprToValue(pTabInfo, nowMillis, pArgList, &argVec))
    return nullptr;

  ValueItem* pResult = nullptr;

  do {
    if (argVec.size() != 2)
      break;

    int32_t leftType = argVec[0]->GetValueType();
    int32_t rightType = argVec[1]->GetValueType();

    if (leftType == PDB_VALUE_TYPE::VAL_INT64)
    {
      if (rightType == PDB_VALUE_TYPE::VAL_INT64)
        pResult = Create2ParamOpFunctionStep<PDB_VALUE_TYPE::VAL_INT64, PDB_VALUE_TYPE::VAL_INT64>(op, argVec[0], argVec[1]);
      else if (rightType == PDB_VALUE_TYPE::VAL_DOUBLE)
        pResult = Create2ParamOpFunctionStep<PDB_VALUE_TYPE::VAL_INT64, PDB_VALUE_TYPE::VAL_DOUBLE>(op, argVec[0], argVec[1]);
    }
    else if (leftType == PDB_VALUE_TYPE::VAL_DOUBLE)
    {
      if (rightType == PDB_VALUE_TYPE::VAL_INT64)
        pResult = Create2ParamOpFunctionStep<PDB_VALUE_TYPE::VAL_DOUBLE, PDB_VALUE_TYPE::VAL_INT64>(op, argVec[0], argVec[1]);
      else if (rightType == PDB_VALUE_TYPE::VAL_DOUBLE)
        pResult = Create2ParamOpFunctionStep<PDB_VALUE_TYPE::VAL_DOUBLE, PDB_VALUE_TYPE::VAL_DOUBLE>(op, argVec[0], argVec[1]);
    }

  } while (false);

  if (pResult == nullptr)
  {
    DeleteValueVec(&argVec);
  }

  return pResult;
}

ValueItem* CreateDateTimeFunction(int op, const TableInfo* pTabInfo, int64_t nowMillis, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
    return nullptr;

  if (!ConvertExprToValue(pTabInfo, nowMillis, pArgList, &argVec))
    return nullptr;

  ValueItem* pResult = nullptr;

  do {
    if (argVec.size() != 2)
      break;

    if (op == PDB_SQL_FUNC::FUNC_DATETIMEADD)
    {
      DateTimeAdd* pTimeAdd = new DateTimeAdd(argVec[0], argVec[1]);
      pTimeAdd->AdjustDateTime();
      pResult = pTimeAdd;
    }
    else if (op == PDB_SQL_FUNC::FUNC_DATETIMEDIFF)
    {
      DateTimeDiff* pTimeDiff = new DateTimeDiff(argVec[0], argVec[1]);
      pTimeDiff->AjustValueType();
      pResult = pTimeDiff;
    }
    else
    {
      DBVal tmpUnit;
      if (!argVec[1]->IsConstValue())
        break;

      PdbErr_t retVal = argVec[1]->GetValue(nullptr, &tmpUnit);
      if (retVal != PdbE_OK)
        break;

      delete argVec[1];
      argVec.erase((argVec.begin() + 1));

      if (!DBVAL_IS_STRING(&tmpUnit))
        break;

      int64_t millisOffset = 0;
      if (StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "s", (sizeof("s") - 1)),
        StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "second", (sizeof("second") - 1)))
      {
        millisOffset = MillisPerSecond;
      }
      else if (StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "m", (sizeof("m") - 1)),
        StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "minute", (sizeof("minute") - 1)))
      {
        millisOffset = MillisPerMinute;
      }
      else if (StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "h", (sizeof("h") - 1)),
        StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "hour", (sizeof("hour") - 1)))
      {
        millisOffset = MillisPerHour;
      }
      else if (StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "d", (sizeof("d") - 1)),
        StringTool::ComparyNoCase(DBVAL_GET_STRING(&tmpUnit), DBVAL_GET_LEN(&tmpUnit), "day", (sizeof("day") - 1)))
      {
        millisOffset = MillisPerDay;
      }
      else
      {
        break;
      }

      if (op == PDB_SQL_FUNC::FUNC_DATETIMEFLOOR)
      {
        DateTimeFloor* pTimeFloor = new DateTimeFloor(argVec[0], millisOffset);
        pTimeFloor->AjustDateTime();
        pResult = pTimeFloor;
      }
      else if (op == PDB_SQL_FUNC::FUNC_DATETIMECEIL)
      {
        DateTimeCeil* pTimeCeil = new DateTimeCeil(argVec[0], millisOffset);
        pTimeCeil->AjustDateTime();
        pResult = pTimeCeil;
      }
    }

  } while (false);

  if (pResult == nullptr)
  {
    DeleteValueVec(&argVec);
  }

  return pResult;
}

ValueItem* CreateNowFunction(const TableInfo* pTabInfo, int64_t nowMillis, const ExprValueList* pArgList)
{
  if (pArgList != nullptr)
  {
    const std::vector<ExprValue*>* pExprVec = pArgList->GetValueList();
    if (pExprVec->size() > 0)
      return nullptr;
  }

  return new ConstValue(nowMillis, true);
}

ValueItem* CreateAbsFunction(const TableInfo* pTabInfo, int64_t nowMillis, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
    return nullptr;

  if (!ConvertExprToValue(pTabInfo, nowMillis, pArgList, &argVec))
    return nullptr;

  if (argVec.size() == 1)
  {
    return new AbsFunction(argVec[0]);
  }

  DeleteValueVec(&argVec);
  return nullptr;
}

ValueItem* CreateIfFunction(const TableInfo* pTabInfo, int64_t nowMillis, const ExprValueList* pArgList)
{
  std::vector<ValueItem*> argVec;
  if (pArgList == nullptr)
    return nullptr;

  if (!ConvertExprToValue(pTabInfo, nowMillis, pArgList, &argVec))
    return nullptr;

  if (argVec.size() == 3)
  {
    if (argVec[0]->GetValueType() == PDB_VALUE_TYPE::VAL_BOOL)
    {
      IfFunction* pNew = new IfFunction(argVec[0], argVec[1], argVec[2]);
      pNew->AdjustDateTime();
      return pNew;
    }
  }

  DeleteValueVec(&argVec);
  return nullptr;
}

ValueItem* CreateFieldValue(const TableInfo* pTableInfo, const ExprValue* pExpr)
{
  if (pTableInfo == nullptr || pExpr == nullptr)
    return nullptr;

  DBVal fieldVal = pExpr->GetValue();
  if (!DBVAL_IS_STRING(&fieldVal))
    return nullptr;

  PdbErr_t retVal = PdbE_OK;
  size_t fieldPos = 0;
  int32_t fieldType = 0;
  std::string fieldName = std::string(DBVAL_GET_STRING(&fieldVal), DBVAL_GET_LEN(&fieldVal));
  retVal = pTableInfo->GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
  if (retVal != PdbE_OK)
    return nullptr;

  return new FieldValue(fieldPos, fieldType);
}

ValueItem* CreateIsNullFunction(const TableInfo* pTableInfo, const ExprValue* pExpr)
{
  ValueItem* pFieldValue = CreateFieldValue(pTableInfo, pExpr);
  if (pFieldValue == nullptr)
    return nullptr;

  return new IsNullFunction(pFieldValue);
}

ValueItem* CreateIsNotNullFunction(const TableInfo* pTableInfo, const ExprValue* pExpr)
{
  ValueItem* pFieldValue = CreateFieldValue(pTableInfo, pExpr);
  if (pFieldValue == nullptr)
    return nullptr;

  return new IsNotNullFunction(pFieldValue);
}

ValueItem* CreateLikeFunction(const TableInfo* pTableInfo, const ExprValue* pExpr)
{
  if (pTableInfo == nullptr || pExpr == nullptr)
    return nullptr;

  const ExprValue* pPatVal = pExpr->GetLeftParam();
  if (pPatVal == nullptr)
    return nullptr;

  if (pPatVal->GetValueType() != TK_STRING)
    return nullptr;

  DBVal patStr = pPatVal->GetValue();
  if (!DBVAL_IS_STRING(&patStr))
    return nullptr;

  ValueItem* pFieldValue = CreateFieldValue(pTableInfo, pExpr);
  if (pFieldValue == nullptr)
    return nullptr;

  return new LikeFunction(pFieldValue, DBVAL_GET_STRING(&patStr), DBVAL_GET_LEN(&patStr));
}

ValueItem* CreateInFunction(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMillis)
{
  if (pTableInfo == nullptr || pExpr == nullptr)
    return nullptr;

  if (pExpr->GetValueType() != TK_IN)
    return nullptr;

  bool errFlag = false;
  DBVal tmpVal;
  const ExprValueList* pArgList = pExpr->GetArgList();
  const std::vector<ExprValue*>* pValVec = pArgList->GetValueList();
  std::list<int64_t> intList;
  for (auto argIt = pValVec->begin(); argIt != pValVec->end(); argIt++)
  {
    ValueItem* pTmpVal = BuildGeneralValueItem(pTableInfo, (*argIt), nowMillis);
    if (pTmpVal == nullptr)
    {
      errFlag = true;
      break;
    }

    do {
      if (!pTmpVal->IsValid())
      {
        errFlag = true;
        break;
      }

      if (!pTmpVal->IsConstValue())
      {
        errFlag = true;
        break;
      }

      if (pTmpVal->GetValue(nullptr, &tmpVal) != PdbE_OK)
      {
        errFlag = true;
        break;
      }

      if (!DBVAL_IS_INT64(&tmpVal))
      {
        errFlag = true;
        break;
      }

      intList.push_back(DBVAL_GET_INT64(&tmpVal));
    } while (false);

    delete pTmpVal;

    if (errFlag)
      break;
  }

  if (errFlag)
    return nullptr;

  ValueItem* pFieldValue = CreateFieldValue(pTableInfo, pExpr);
  if (pFieldValue == nullptr)
    return nullptr;

  return new InFunction(pFieldValue, intList);
}

ValueItem* CreateNotInFunction(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMillis)
{
  if (pTableInfo == nullptr || pExpr == nullptr)
    return nullptr;

  if (pExpr->GetValueType() != TK_NOTIN)
    return nullptr;

  bool errFlag = false;
  DBVal tmpVal;
  const ExprValueList* pArgList = pExpr->GetArgList();
  const std::vector<ExprValue*>* pValVec = pArgList->GetValueList();
  std::list<int64_t> intList;
  for (auto argIt = pValVec->begin(); argIt != pValVec->end(); argIt++)
  {
    ValueItem* pTmpVal = BuildGeneralValueItem(pTableInfo, (*argIt), nowMillis);
    if (pTmpVal == nullptr)
    {
      errFlag = true;
      break;
    }

    do {
      if (!pTmpVal->IsValid())
      {
        errFlag = true;
        break;
      }

      if (!pTmpVal->IsConstValue())
      {
        errFlag = true;
        break;
      }

      if (pTmpVal->GetValue(nullptr, &tmpVal) != PdbE_OK)
      {
        errFlag = true;
        break;
      }

      if (!DBVAL_IS_INT64(&tmpVal))
      {
        errFlag = true;
        break;
      }

      intList.push_back(DBVAL_GET_INT64(&tmpVal));
    } while (false);

    delete pTmpVal;

    if (errFlag)
      break;
  }

  if (errFlag)
    return nullptr;

  ValueItem* pFieldValue = CreateFieldValue(pTableInfo, pExpr);
  if (pFieldValue == nullptr)
    return nullptr;

  return new NotInFunction(pFieldValue, intList);
}

ValueItem* CreateInFunction(size_t fieldPos, std::vector<ValueItem*>& argVec)
{
  if (argVec.size() == 0)
    return nullptr;

  PdbErr_t retVal = PdbE_OK;
  DBVal tmpVal;
  std::list<int64_t> argList;
  for (auto argIt = argVec.begin(); argIt != argVec.end(); argIt++)
  {
    if (!(*argIt)->IsConstValue())
      return nullptr;

    retVal = (*argIt)->GetValue(nullptr, &tmpVal);
    if (retVal != PdbE_OK)
      return nullptr;

    if (!DBVAL_IS_INT64(&tmpVal))
      return nullptr;

    argList.push_back(DBVAL_GET_INT64(&tmpVal));
  }

  for (auto argIt = argVec.begin(); argIt != argVec.end(); argIt++)
  {
    delete (*argIt);
  }

  FieldValue* pField = new FieldValue(fieldPos, PDB_VALUE_TYPE::VAL_INT64);
  return new InFunction(pField, argList);
}

ValueItem* CreateNotInFunction(size_t fieldPos, std::vector<ValueItem*>& argVec)
{
  if (argVec.size() == 0)
    return nullptr;

  PdbErr_t retVal = PdbE_OK;
  DBVal tmpVal;
  std::list<int64_t> argList;
  for (auto argIt = argVec.begin(); argIt != argVec.end(); argIt++)
  {
    if (!(*argIt)->IsConstValue())
      return nullptr;

    retVal = (*argIt)->GetValue(nullptr, &tmpVal);
    if (retVal != PdbE_OK)
      return nullptr;

    if (!DBVAL_IS_INT64(&tmpVal))
      return nullptr;

    argList.push_back(DBVAL_GET_INT64(&tmpVal));
  }

  for (auto argIt = argVec.begin(); argIt != argVec.end(); argIt++)
  {
    delete (*argIt);
  }

  FieldValue* pField = new FieldValue(fieldPos, PDB_VALUE_TYPE::VAL_INT64);
  return new NotInFunction(pField, argList);
}

ValueItem* CreateGeneralFunction(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMillis)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType = 0;
  size_t fieldPos = 0;
  char uniqueName[32];
  if (pTableInfo == nullptr || pExpr == nullptr)
    return nullptr;

  if (pExpr->GetValueType() != TK_FUNCTION)
    return nullptr;

  DBVal nameVal = pExpr->GetValue();
  if (!DBVAL_IS_STRING(&nameVal))
    return nullptr;

  if (DBVAL_GET_LEN(&nameVal) < 2 || DBVAL_GET_LEN(&nameVal) > 20)
    return nullptr;

  int funcId = GetFunctionId(DBVAL_GET_STRING(&nameVal), DBVAL_GET_LEN(&nameVal));
  if (funcId <= 0)
    return nullptr;

  ValueItem* pResultVal = nullptr;

  do {
    if (funcId == PDB_SQL_FUNC::FUNC_AGG_COUNT
      || funcId == PDB_SQL_FUNC::FUNC_AGG_FIRST
      || funcId == PDB_SQL_FUNC::FUNC_AGG_LAST
      || funcId == PDB_SQL_FUNC::FUNC_AGG_AVG
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MIN
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MAX
      || funcId == PDB_SQL_FUNC::FUNC_AGG_SUM
      || funcId == PDB_SQL_FUNC::FUNC_AGG_COUNT_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_AVG_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MIN_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_MAX_IF
      || funcId == PDB_SQL_FUNC::FUNC_AGG_SUM_IF)
    {
      sprintf(uniqueName, "agg_%llu", reinterpret_cast<uintptr_t>(pExpr));
      retVal = pTableInfo->GetFieldInfo(uniqueName, &fieldPos, &fieldType);
      if (retVal != PdbE_OK)
        return nullptr;

      return new FieldValue(fieldPos, fieldType);
    }

    switch (funcId)
    {
    case PDB_SQL_FUNC::FUNC_ADD:
    case PDB_SQL_FUNC::FUNC_SUB:
    case PDB_SQL_FUNC::FUNC_MUL:
    case PDB_SQL_FUNC::FUNC_DIV:
    case PDB_SQL_FUNC::FUNC_MOD:
      pResultVal = Create2ParamOpFunction(funcId, pTableInfo, nowMillis, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_DATETIMEADD:
    case PDB_SQL_FUNC::FUNC_DATETIMEDIFF:
    case PDB_SQL_FUNC::FUNC_DATETIMEFLOOR:
    case PDB_SQL_FUNC::FUNC_DATETIMECEIL:
      pResultVal = CreateDateTimeFunction(funcId, pTableInfo, nowMillis, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_IF:
      pResultVal = CreateIfFunction(pTableInfo, nowMillis, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_ABS:
      pResultVal = CreateAbsFunction(pTableInfo, nowMillis, pExpr->GetArgList());
      break;
    case PDB_SQL_FUNC::FUNC_NOW:
      pResultVal = CreateNowFunction(pTableInfo, nowMillis, pExpr->GetArgList());
      break;
    }

  } while (false);

  if (pResultVal != nullptr)
  {
    if (pResultVal->IsValid())
      return pResultVal;

    delete pResultVal;
  }

  return nullptr;
}

template<int FieldType, typename T>
ValueItem* CreateFieldCompare(int op, size_t fieldPos, T val)
{
  switch (op)
  {
  case TK_LT:
    return new FieldCompareFunction<FieldType, T, TK_LT>(fieldPos, val);
  case TK_LE:
    return new FieldCompareFunction<FieldType, T, TK_LE>(fieldPos, val);
  case TK_GT:
    return new FieldCompareFunction<FieldType, T, TK_GT>(fieldPos, val);
  case TK_GE:
    return new FieldCompareFunction<FieldType, T, TK_GE>(fieldPos, val);
  case TK_EQ:
    return new FieldCompareFunction<FieldType, T, TK_EQ>(fieldPos, val);
  case TK_NE:
    return new FieldCompareFunction<FieldType, T, TK_NE>(fieldPos, val);
  }

  return nullptr;
}

template<int LeftType, int RightType, typename T>
ValueItem* CreateValueCompareLeaf(int op, ValueItem* pLeft, ValueItem* pRight)
{
  switch (op)
  {
  case TK_LT:
    return new ValueCompareFunction<LeftType, RightType, T, TK_LT>(pLeft, pRight);
  case TK_LE:
    return new ValueCompareFunction<LeftType, RightType, T, TK_LE>(pLeft, pRight);
  case TK_GT:
    return new ValueCompareFunction<LeftType, RightType, T, TK_GT>(pLeft, pRight);
  case TK_GE:
    return new ValueCompareFunction<LeftType, RightType, T, TK_GE>(pLeft, pRight);
  case TK_EQ:
    return new ValueCompareFunction<LeftType, RightType, T, TK_EQ>(pLeft, pRight);
  case TK_NE:
    return new ValueCompareFunction<LeftType, RightType, T, TK_NE>(pLeft, pRight);
  }
  return nullptr;
}

ValueItem* CreateValueCompare(int op, ValueItem* pLeft, ValueItem* pRight)
{
  int leftType = pLeft->GetValueType();
  int rightType = pRight->GetValueType();

  if (leftType == rightType)
  {
    switch (leftType)
    {
    case PDB_FIELD_TYPE::TYPE_BOOL:
      return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_BOOL, PDB_FIELD_TYPE::TYPE_BOOL, bool>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_INT64:
      return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_INT64, PDB_FIELD_TYPE::TYPE_INT64, int64_t>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_DATETIME:
      return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_DATETIME, PDB_FIELD_TYPE::TYPE_DATETIME, int64_t>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_DOUBLE:
      return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_DOUBLE, PDB_FIELD_TYPE::TYPE_DOUBLE, double>(op, pLeft, pRight);
    case PDB_FIELD_TYPE::TYPE_STRING:
      return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_STRING, PDB_FIELD_TYPE::TYPE_STRING, std::string>(op, pLeft, pRight);
    }

    return nullptr;
  }
  else
  {
    if (leftType == PDB_FIELD_TYPE::TYPE_INT64)
    {
      if (rightType == PDB_FIELD_TYPE::TYPE_DATETIME)
        return CreateValueCompareLeaf< PDB_FIELD_TYPE::TYPE_INT64, PDB_FIELD_TYPE::TYPE_DATETIME, int64_t>(op, pLeft, pRight);
      else if (rightType == PDB_FIELD_TYPE::TYPE_DOUBLE)
        return CreateValueCompareLeaf< PDB_FIELD_TYPE::TYPE_INT64, PDB_FIELD_TYPE::TYPE_DOUBLE, double>(op, pLeft, pRight);
    }
    else if (leftType == PDB_FIELD_TYPE::TYPE_DATETIME)
    {
      if (rightType == PDB_FIELD_TYPE::TYPE_INT64)
        return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_DATETIME, PDB_FIELD_TYPE::TYPE_INT64, int64_t>(op, pLeft, pRight);
      else if (rightType == PDB_FIELD_TYPE::TYPE_DOUBLE)
        return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_DATETIME, PDB_FIELD_TYPE::TYPE_DOUBLE, double>(op, pLeft, pRight);
    }
    else if (leftType == PDB_FIELD_TYPE::TYPE_DOUBLE)
    {
      if (rightType == PDB_FIELD_TYPE::TYPE_INT64)
        return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_DOUBLE, PDB_FIELD_TYPE::TYPE_INT64, double>(op, pLeft, pRight);
      else if (rightType == PDB_FIELD_TYPE::TYPE_DATETIME)
        return CreateValueCompareLeaf<PDB_FIELD_TYPE::TYPE_DOUBLE, PDB_FIELD_TYPE::TYPE_DATETIME, double>(op, pLeft, pRight);
    }
  }

  return nullptr;
}

ValueItem* CreateOperator(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMillis)
{
  if (pExpr == nullptr)
    return nullptr;

  int op = pExpr->GetValueType();
  const ExprValue* pLeftExpr = pExpr->GetLeftParam();
  const ExprValue* pRightExpr = pExpr->GetRightParam();
  if (pLeftExpr == nullptr || pRightExpr == nullptr)
    return nullptr;

  PdbErr_t retVal = PdbE_OK;
  ValueItem* pLeftValue = nullptr;
  ValueItem* pRightValue = BuildGeneralValueItem(pTableInfo, pRightExpr, nowMillis);
  if (pRightValue == nullptr)
    return nullptr;

  if (!pRightValue->IsValid())
  {
    delete pRightValue;
    return nullptr;
  }

  ValueItem* pResultValue = nullptr;
  DBVal oldVal;
  DateTime dt;

  if (pLeftExpr->GetValueType() == TK_ID && pRightValue->IsConstValue())
  {
    do {
      DBVal fieldVal = pLeftExpr->GetValue();
      if (!DBVAL_IS_STRING(&fieldVal))
        break;

      PdbErr_t retVal = PdbE_OK;
      size_t fieldPos = 0;
      int32_t fieldType = 0;
      std::string fieldName = std::string(DBVAL_GET_STRING(&fieldVal), DBVAL_GET_LEN(&fieldVal));
      retVal = pTableInfo->GetFieldInfo(fieldName.c_str(), &fieldPos, &fieldType);
      if (retVal != PdbE_OK)
        break;

      DBVal rightVal;
      retVal = pRightValue->GetValue(nullptr, &rightVal);
      if (retVal != PdbE_OK)
        break;

      if (fieldType == PDB_VALUE_TYPE::VAL_DATETIME && DBVAL_IS_STRING(&rightVal))
      {
        if (dt.Parse(DBVAL_GET_STRING(&rightVal), DBVAL_GET_LEN(&rightVal)))
        {
          DBVAL_SET_DATETIME(&rightVal, dt.TotalMilliseconds());
        }
      }

      if (fieldType != DBVAL_GET_TYPE(&rightVal))
      {
        //表达式两边的类型不一样
        if (DBVAL_IS_INT64(&rightVal))
        {
          //int64 可以转成 datetime， double
          int64_t tmpI64 = DBVAL_GET_INT64(&rightVal);
          if (fieldType == PDB_FIELD_TYPE::TYPE_DATETIME)
          {
            if (tmpI64 >= MinMillis && tmpI64 < MaxMillis)
            {
              DBVAL_SET_DATETIME(&rightVal, tmpI64);
            }
          }
          else if (fieldType == PDB_FIELD_TYPE::TYPE_DOUBLE)
          {
            DBVAL_SET_DOUBLE(&rightVal, static_cast<double>(tmpI64));
          }
        }

        if (fieldType != DBVAL_GET_TYPE(&rightVal))
        {
          break;
        }
      }

      switch (fieldType)
      {
      case PDB_FIELD_TYPE::TYPE_BOOL:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_BOOL, bool>(op, fieldPos, DBVAL_GET_BOOL(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_INT64:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_INT64, int64_t>(op, fieldPos, DBVAL_GET_INT64(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_DATETIME:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_DATETIME, int64_t>(op, fieldPos, DBVAL_GET_DATETIME(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_DOUBLE:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_DOUBLE, double>(op, fieldPos, DBVAL_GET_DOUBLE(&rightVal));
        break;
      case PDB_FIELD_TYPE::TYPE_STRING:
        pResultValue = CreateFieldCompare<PDB_FIELD_TYPE::TYPE_STRING, std::string>(op, fieldPos, std::string(DBVAL_GET_STRING(&rightVal), DBVAL_GET_LEN(&rightVal)));
        break;
      }

    } while (false);

    delete pRightValue;
    return pResultValue;
  }
  else
  {
    do {
      pLeftValue = BuildGeneralValueItem(pTableInfo, pLeftExpr, nowMillis);
      if (pLeftValue == nullptr)
        break;

      if (!pLeftValue->IsValid())
        break;

      //1. 尝试 string 转 datetime
      //2. 尝试 int 转 datetime 或 double
      auto convertParamFunc = [&](ValueItem* pParam1, ValueItem* pParam2)->ValueItem* {
        if (pParam1->IsConstValue()) {
          if (pParam1->GetValue(nullptr, &oldVal) != PdbE_OK)
            return nullptr;

          if (DBVAL_IS_STRING(&oldVal) && pParam2->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME)
          {
            if (dt.Parse(DBVAL_GET_STRING(&oldVal), DBVAL_GET_LEN(&oldVal)))
            {
              return new ConstValue(dt.TotalMilliseconds(), true);
            }
          }
          else if (DBVAL_IS_INT64(&oldVal))
          {
            if (pParam2->GetValueType() == PDB_VALUE_TYPE::VAL_DATETIME)
            {
              if (DBVAL_GET_INT64(&oldVal) >= MinMillis && DBVAL_GET_INT64(&oldVal) < MaxMillis)
              {
                return new ConstValue(DBVAL_GET_INT64(&oldVal), true);
              }
            }
            else if (pParam2->GetValueType() == PDB_VALUE_TYPE::VAL_DOUBLE)
            {
              return new ConstValue(static_cast<double>(DBVAL_GET_INT64(&oldVal)));
            }
          }
        }
        return nullptr;
      };

      if (pLeftValue->GetValueType() != pRightValue->GetValueType())
      {
        ValueItem* pNewValue = convertParamFunc(pLeftValue, pRightValue);
        if (pNewValue != nullptr)
        {
          delete pLeftValue;
          pLeftValue = pNewValue;
        }
      }

      if (pLeftValue->GetValueType() != pRightValue->GetValueType())
      {
        ValueItem* pNewValue = convertParamFunc(pRightValue, pLeftValue);
        if (pNewValue != nullptr)
        {
          delete pRightValue;
          pRightValue = pNewValue;
        }
      }

      pResultValue = CreateValueCompare(op, pLeftValue, pRightValue);
    } while (false);

    if (pResultValue == nullptr)
    {
      if (pLeftValue != nullptr)
        delete pLeftValue;

      if (pRightValue != nullptr)
        delete pRightValue;
    }

    return pResultValue;
  }
}

PdbErr_t CreateAndFunctionStep(const TableInfo* pTableInfo, const ExprValue* pExpr,
  int64_t nowMillis, AndFunction* pAndFunc)
{
  PdbErr_t retVal = PdbE_OK;
  int opType = pExpr->GetValueType();
  if (opType == TK_AND)
  {
    const ExprValue* pLeftExpr = pExpr->GetLeftParam();
    const ExprValue* pRightExpr = pExpr->GetRightParam();
    if (pLeftExpr == nullptr || pRightExpr == nullptr)
      return PdbE_SQL_ERROR;

    retVal = CreateAndFunctionStep(pTableInfo, pLeftExpr, nowMillis, pAndFunc);
    if (retVal != PdbE_OK)
      return retVal;

    retVal = CreateAndFunctionStep(pTableInfo, pRightExpr, nowMillis, pAndFunc);
    if (retVal != PdbE_OK)
      return retVal;
  }
  else
  {
    ValueItem* pValue = BuildGeneralValueItem(pTableInfo, pExpr, nowMillis);
    if (pValue == nullptr)
      return PdbE_SQL_ERROR;

    pAndFunc->AddValueItem(pValue);
  }

  return PdbE_OK;
}

ValueItem* CreateAndFunction(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMillis)
{
  AndFunction* pAndFunc = new AndFunction();
  if (CreateAndFunctionStep(pTableInfo, pExpr, nowMillis, pAndFunc) != PdbE_OK)
  {
    delete pAndFunc;
    return nullptr;
  }

  return pAndFunc;
}

QueryField* CreateAggCount(size_t fieldPos)
{
  return new CountFunc(fieldPos);
}

template<bool IsFirst>
QueryField* CreateAggFirstOrLast(size_t fieldPos, int fieldType)
{
  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_BOOL:
    return new FirstOrLastValueFunc< PDB_FIELD_TYPE::TYPE_BOOL, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new FirstOrLastValueFunc< PDB_FIELD_TYPE::TYPE_INT64, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    return new FirstOrLastValueFunc< PDB_FIELD_TYPE::TYPE_DATETIME, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new FirstOrLastValueFunc< PDB_FIELD_TYPE::TYPE_DOUBLE, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_STRING:
    return new FirstOrLastValueFunc< PDB_FIELD_TYPE::TYPE_STRING, IsFirst>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_BLOB:
    return new FirstOrLastValueFunc< PDB_FIELD_TYPE::TYPE_BLOB, IsFirst>(fieldPos);
  }

  return nullptr;
}

QueryField* CreateAggAvg(size_t fieldPos, int fieldType)
{
  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new AvgFunc< PDB_FIELD_TYPE::TYPE_INT64, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new AvgFunc<PDB_FIELD_TYPE::TYPE_DOUBLE, double>(fieldPos);
  }

  return nullptr;
}

QueryField* CreateAggSum(size_t fieldPos, int fieldType)
{
  switch (fieldType)
  {
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_INT64, int64_t>(fieldPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new SumFunc<PDB_FIELD_TYPE::TYPE_DOUBLE, double>(fieldPos);
    break;
  }

  return nullptr;
}

template<int CompareType, bool IsMax>
QueryField* CreateAggExtremeLeaf(int targetType, size_t comparePos, size_t targetPos)
{
  switch (targetType)
  {
  case PDB_FIELD_TYPE::TYPE_BOOL:
    return new ExtremeValueFunc< CompareType, PDB_FIELD_TYPE::TYPE_BOOL, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_INT64:
    return new ExtremeValueFunc< CompareType, PDB_FIELD_TYPE::TYPE_INT64, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    return new ExtremeValueFunc< CompareType, PDB_FIELD_TYPE::TYPE_DATETIME, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return new ExtremeValueFunc< CompareType, PDB_FIELD_TYPE::TYPE_DOUBLE, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_STRING:
    return new ExtremeValueFunc< CompareType, PDB_FIELD_TYPE::TYPE_STRING, IsMax>(comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_BLOB:
    return new ExtremeValueFunc< CompareType, PDB_FIELD_TYPE::TYPE_BLOB, IsMax>(comparePos, targetPos);
  }

  return nullptr;
}

template<bool IsMax>
QueryField* CreateAggExtreme(int compareType, int targetType, size_t comparePos, size_t targetPos)
{
  switch (compareType)
  {
  case PDB_FIELD_TYPE::TYPE_INT64:
    return CreateAggExtremeLeaf< PDB_FIELD_TYPE::TYPE_INT64, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DATETIME:
    return CreateAggExtremeLeaf< PDB_FIELD_TYPE::TYPE_DATETIME, IsMax>(targetType, comparePos, targetPos);
  case PDB_FIELD_TYPE::TYPE_DOUBLE:
    return CreateAggExtremeLeaf< PDB_FIELD_TYPE::TYPE_DOUBLE, IsMax>(targetType, comparePos, targetPos);
  }

  return nullptr;
}


ValueItem* BuildGeneralValueItem(const TableInfo* pTableInfo, const ExprValue* pExpr, int64_t nowMillis)
{
  PdbErr_t retVal = PdbE_OK;

  if (pExpr == nullptr)
    return nullptr;

  ValueItem* pNewValue = nullptr;
  DBVal exprVal = pExpr->GetValue();
  int exprType = pExpr->GetValueType();
  switch (exprType)
  {
  case TK_ID:
    pNewValue = CreateFieldValue(pTableInfo, pExpr);
    break;
  case TK_TRUE:
  case TK_FALSE:
  case TK_INTEGER:
  case TK_DOUBLE:
  case TK_STRING:
  case TK_BLOB:
  case TK_TIMEVAL:
    pNewValue = new ConstValue(exprVal);
    break;
  case TK_FUNCTION:
    pNewValue = CreateGeneralFunction(pTableInfo, pExpr, nowMillis);
    break;
  case TK_LT:
  case TK_LE:
  case TK_GT:
  case TK_GE:
  case TK_EQ:
  case TK_NE:
    pNewValue = CreateOperator(pTableInfo, pExpr, nowMillis);
    break;
  case TK_AND:
    pNewValue = CreateAndFunction(pTableInfo, pExpr, nowMillis);
    break;
  case TK_LIKE:
    pNewValue = CreateLikeFunction(pTableInfo, pExpr);
    break;
  case TK_ISNOTNULL:
    pNewValue = CreateIsNotNullFunction(pTableInfo, pExpr);
    break;
  case TK_ISNULL:
    pNewValue = CreateIsNullFunction(pTableInfo, pExpr);
    break;
  case TK_IN:
    pNewValue = CreateInFunction(pTableInfo, pExpr, nowMillis);
    break;
  case TK_NOTIN:
    pNewValue = CreateNotInFunction(pTableInfo, pExpr, nowMillis);
    break;
  }

  if (pNewValue != nullptr)
  {
    if (!pNewValue->IsValid())
    {
      delete pNewValue;
      pNewValue = nullptr;
    }
  }

  return pNewValue;
}

QueryField* BuildTargetGroupStep(int funcId, const TableInfo* pTableInfo, int64_t nowMillis,
  const ExprValue* pParam1, const ExprValue* pParam2)
{
  PdbErr_t retVal = PdbE_OK;
  int32_t fieldType1, fieldType2 = 0;
  size_t fieldPos1, fieldPos2 = 0;

  if (pParam1 != nullptr)
  {
    if (pParam1->GetValueType() == TK_ID)
    {
      DBVal nameVal1 = pParam1->GetValue();
      if (!DBVAL_IS_STRING(&nameVal1))
        return nullptr;

      std::string fieldName1 = std::string(DBVAL_GET_STRING(&nameVal1), DBVAL_GET_LEN(&nameVal1));
      retVal = pTableInfo->GetFieldInfo(fieldName1.c_str(), &fieldPos1, &fieldType1);
      if (retVal != PdbE_OK)
        return nullptr;
    }
  }

  if (pParam2 != nullptr)
  {
    if (pParam2->GetValueType() != TK_ID)
      return nullptr;

    DBVal nameVal2 = pParam2->GetValue();
    if (!DBVAL_IS_STRING(&nameVal2))
      return nullptr;

    std::string fieldName2 = std::string(DBVAL_GET_STRING(&nameVal2), DBVAL_GET_LEN(&nameVal2));
    retVal = pTableInfo->GetFieldInfo(fieldName2.c_str(), &fieldPos2, &fieldType2);
    if (retVal != PdbE_OK)
      return nullptr;
  }

  if (pParam1 == nullptr && pParam2 == nullptr)
  {
    if (funcId == PDB_SQL_FUNC::FUNC_AGG_COUNT)
      return new CountFunc(PDB_DEVID_INDEX);
    else
      return nullptr;
  }
  else if (pParam1 != nullptr && pParam2 == nullptr)
  {
    if (pParam1->GetValueType() == TK_STAR)
    {
      if (funcId == PDB_SQL_FUNC::FUNC_AGG_COUNT)
        return new CountFunc(PDB_DEVID_INDEX);
      else
        return nullptr;
    }

    if (pParam1->GetValueType() != TK_ID)
      return nullptr;

    switch (funcId)
    {
    case PDB_SQL_FUNC::FUNC_AGG_COUNT:
      return CreateAggCount(fieldPos1);
    case PDB_SQL_FUNC::FUNC_AGG_FIRST:
      return CreateAggFirstOrLast<true>(fieldPos1, fieldType1);
    case PDB_SQL_FUNC::FUNC_AGG_LAST:
      return CreateAggFirstOrLast<false>(fieldPos1, fieldType1);
    case PDB_SQL_FUNC::FUNC_AGG_AVG:
      return CreateAggAvg(fieldPos1, fieldType1);
    case PDB_SQL_FUNC::FUNC_AGG_MIN:
      return CreateAggExtreme<false>(fieldType1, fieldType1, fieldPos1, fieldPos1);
    case PDB_SQL_FUNC::FUNC_AGG_MAX:
      return CreateAggExtreme<true>(fieldType1, fieldType1, fieldPos1, fieldPos1);
    case PDB_SQL_FUNC::FUNC_AGG_SUM:
      return CreateAggSum(fieldPos1, fieldType1);
    }
  }
  else if (pParam1 != nullptr && pParam2 != nullptr)
  {
    if (pParam1->GetValueType() != TK_ID)
      return nullptr;

    if (funcId == PDB_SQL_FUNC::FUNC_AGG_MIN)
    {
      return CreateAggExtreme<false>(fieldType1, fieldType2, fieldPos1, fieldPos2);
    }
    else if (funcId == PDB_SQL_FUNC::FUNC_AGG_MAX)
    {
      return CreateAggExtreme<true>(fieldType1, fieldType2, fieldPos1, fieldPos2);
    }
  }

  return nullptr;
}

PdbErr_t BuildTargetGroupItem(const TableInfo* pTableInfo, const ExprValue* pExpr,
  TableInfo* pGroupInfo, std::vector<QueryField*>& fieldVec, int64_t nowMillis)
{
  PdbErr_t retVal = PdbE_OK;
  char uniqueName[32];
  if (pTableInfo == nullptr || pExpr == nullptr || pGroupInfo == nullptr)
    return PdbE_INVALID_PARAM;

  sprintf(uniqueName, "agg_%llu", reinterpret_cast<uintptr_t>(pExpr));

  if (pExpr->GetValueType() == TK_FUNCTION)
  {
    DBVal nameVal = pExpr->GetValue();
    if (!DBVAL_IS_STRING(&nameVal))
      return PdbE_SQL_ERROR;

    if (DBVAL_GET_LEN(&nameVal) < 2 || DBVAL_GET_LEN(&nameVal) > 20)
      return PdbE_SQL_ERROR;

    int functionId = GetFunctionId(DBVAL_GET_STRING(&nameVal), DBVAL_GET_LEN(&nameVal));
    if (functionId <= 0)
      return PdbE_SQL_ERROR;

    const ExprValue* pParam1 = nullptr;
    const ExprValue* pParam2 = nullptr;
    const ExprValueList* pArgList = pExpr->GetArgList();

    if (functionId == PDB_SQL_FUNC::FUNC_AGG_COUNT
      || functionId == PDB_SQL_FUNC::FUNC_AGG_FIRST
      || functionId == PDB_SQL_FUNC::FUNC_AGG_LAST
      || functionId == PDB_SQL_FUNC::FUNC_AGG_AVG
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MIN
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MAX
      || functionId == PDB_SQL_FUNC::FUNC_AGG_SUM)
    {
      if (pArgList != nullptr)
      {
        const std::vector<ExprValue*>* pArgVec = pArgList->GetValueList();
        if (pArgVec != nullptr)
        {
          if (pArgVec->size() > 2)
            return PdbE_SQL_ERROR;

          if (pArgVec->size() >= 1)
            pParam1 = pArgVec->at(0);

          if (pArgVec->size() == 2)
            pParam2 = pArgVec->at(1);
        }
      }

      QueryField* pAggFunc = BuildTargetGroupStep(functionId, pTableInfo, nowMillis, pParam1, pParam2);
      if (pAggFunc == nullptr)
        return PdbE_SQL_ERROR;

      pGroupInfo->AddField(uniqueName, pAggFunc->FieldType());
      fieldVec.push_back(pAggFunc);
      return PdbE_OK;
    }

    if (functionId == PDB_SQL_FUNC::FUNC_AGG_COUNT_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_AVG_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MIN_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MAX_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_SUM_IF)
    {
      if (pArgList == nullptr)
        return PdbE_SQL_ERROR;

      const std::vector<ExprValue*>* pArgVec = pArgList->GetValueList();
      if (pArgVec == nullptr)
        return PdbE_SQL_ERROR;

      if (pArgVec->size() < 1 || pArgVec->size() > 3)
        return PdbE_SQL_ERROR;

      if (pArgVec->size() >= 2)
        pParam1 = pArgVec->at(1);

      if (pArgVec->size() == 3)
        pParam2 = pArgVec->at(2);

      ValueItem* pCondi = BuildGeneralValueItem(pTableInfo, pArgVec->at(0), nowMillis);
      if (pCondi == nullptr)
        return PdbE_SQL_ERROR;

      int subFunc = 0;
      switch (functionId)
      {
      case PDB_SQL_FUNC::FUNC_AGG_COUNT_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_COUNT; break;
      case PDB_SQL_FUNC::FUNC_AGG_AVG_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_AVG; break;
      case PDB_SQL_FUNC::FUNC_AGG_MIN_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_MIN; break;
      case PDB_SQL_FUNC::FUNC_AGG_MAX_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_MAX; break;
      case PDB_SQL_FUNC::FUNC_AGG_SUM_IF: subFunc = PDB_SQL_FUNC::FUNC_AGG_SUM; break;
      }

      QueryField* pSubAggFunc = BuildTargetGroupStep(subFunc, pTableInfo, nowMillis, pParam1, pParam2);
      if (pSubAggFunc == nullptr)
      {
        delete pCondi;
        return PdbE_SQL_ERROR;
      }

      QueryField* pAggFunc = new AggIfExtendFunc(pCondi, pSubAggFunc, true);
      pGroupInfo->AddField(uniqueName, pAggFunc->FieldType());
      fieldVec.push_back(pAggFunc);
      return PdbE_OK;
    }

  }

  const ExprValue* pLeftParam = pExpr->GetLeftParam();
  if (pLeftParam != nullptr)
  {
    retVal = BuildTargetGroupItem(pTableInfo, pLeftParam, pGroupInfo, fieldVec, nowMillis);
    if (retVal != PdbE_OK)
      return retVal;
  }

  const ExprValue* pRightParam = pExpr->GetRightParam();
  if (pRightParam != nullptr)
  {
    retVal = BuildTargetGroupItem(pTableInfo, pRightParam, pGroupInfo, fieldVec, nowMillis);
    if (retVal != PdbE_OK)
      return retVal;
  }

  const ExprValueList* pArgList = pExpr->GetArgList();
  if (pArgList != nullptr)
  {
    const std::vector<ExprValue*>* pValVec = pArgList->GetValueList();
    for (auto argIt = pValVec->begin(); argIt != pValVec->end(); argIt++)
    {
      retVal = BuildTargetGroupItem(pTableInfo, (*argIt), pGroupInfo, fieldVec, nowMillis);
      if (retVal != PdbE_OK)
        return retVal;
    }
  }

  return PdbE_OK;
}

bool IncludeAggFunction(const ExprValue* pExpr)
{
  if (pExpr == nullptr)
    return false;

  bool haveAgg = false;

  if (pExpr->GetValueType() == TK_FUNCTION)
  {
    DBVal nameVal = pExpr->GetValue();
    if (!DBVAL_IS_STRING(&nameVal))
      return false;

    if (DBVAL_GET_LEN(&nameVal) < 2 || DBVAL_GET_LEN(&nameVal) > 20)
      return false;

    int functionId = GetFunctionId(DBVAL_GET_STRING(&nameVal), DBVAL_GET_LEN(&nameVal));
    if (functionId <= 0)
      return false;

    if (functionId == PDB_SQL_FUNC::FUNC_AGG_COUNT
      || functionId == PDB_SQL_FUNC::FUNC_AGG_FIRST
      || functionId == PDB_SQL_FUNC::FUNC_AGG_LAST
      || functionId == PDB_SQL_FUNC::FUNC_AGG_AVG
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MIN
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MAX
      || functionId == PDB_SQL_FUNC::FUNC_AGG_SUM
      || functionId == PDB_SQL_FUNC::FUNC_AGG_COUNT_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_AVG_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MIN_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_MAX_IF
      || functionId == PDB_SQL_FUNC::FUNC_AGG_SUM_IF
      )
    {
      return true;
    }
  }

  const ExprValue* pLeftParam = pExpr->GetLeftParam();
  if (pLeftParam != nullptr)
  {
    haveAgg = IncludeAggFunction(pLeftParam);
    if (haveAgg)
      return true;
  }

  const ExprValue* pRightParam = pExpr->GetRightParam();
  if (pRightParam != nullptr)
  {
    haveAgg = IncludeAggFunction(pRightParam);
    if (haveAgg)
      return true;
  }

  const ExprValueList* pArgList = pExpr->GetArgList();
  if (pArgList != nullptr)
  {
    const std::vector<ExprValue*>* pArgVec = pArgList->GetValueList();
    for (auto argIt = pArgVec->begin(); argIt != pArgVec->end(); argIt++)
    {
      haveAgg = IncludeAggFunction(*argIt);
      if (haveAgg)
        return true;
    }
  }

  return false;
}
