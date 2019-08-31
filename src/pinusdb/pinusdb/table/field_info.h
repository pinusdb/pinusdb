#pragma once

#include <stdint.h>
#include <string>
#include "internal.h"

class FieldInfo
{
public:
  FieldInfo();
  ~FieldInfo();

  static PdbErr_t ValidFieldName(const char* pName, size_t nameLen);

  FieldInfo(const FieldInfo& cpy);
  void operator=(const FieldInfo& cpy);

  PdbErr_t SetFieldInfo(const char* pName, int32_t fieldType, bool isKey);
  const char* GetFieldName() const { return fieldName_.c_str(); }
  int32_t GetFieldType() const { return fieldType_; }
  bool GetFieldIsKey() const { return isKey_; }

private:
  bool isKey_;
  int32_t fieldType_;
  std::string fieldName_;
};
