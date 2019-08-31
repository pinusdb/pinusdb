#include "table/field_info.h"
#include "util/string_tool.h"
#include "pdb_error.h"

FieldInfo::FieldInfo()
{
  this->isKey_ = false;
  this->fieldType_ = PDB_FIELD_TYPE::TYPE_INT64;
}
FieldInfo::~FieldInfo()
{
}

PdbErr_t FieldInfo::ValidFieldName(const char* pName, size_t nameLen)
{
  if (nameLen <= 0 || nameLen >= PDB_FILED_NAME_LEN)
    return PdbE_INVALID_FIELD_NAME;

  if (!StringTool::ValidName(pName, nameLen))
    return PdbE_INVALID_FIELD_NAME;

  return PdbE_OK;
}

FieldInfo::FieldInfo(const FieldInfo& cpy)
{
  this->fieldType_ = cpy.fieldType_;
  this->fieldName_ = cpy.fieldName_;
  this->isKey_ = cpy.isKey_;
}
void FieldInfo::operator=(const FieldInfo& cpy)
{
  this->fieldType_ = cpy.fieldType_;
  this->fieldName_ = cpy.fieldName_;
  this->isKey_ = cpy.isKey_;
}

PdbErr_t FieldInfo::SetFieldInfo(const char* pName, int32_t fieldType, bool isKey)
{
  size_t nameLen = strlen(pName);
  PdbErr_t retVal = ValidFieldName(pName, nameLen);
  if (retVal != PdbE_OK)
    return retVal;

  if (!PDB_TYPE_IS_VALID(fieldType))
    return PdbE_INVALID_FIELD_TYPE;

  this->isKey_ = isKey;
  this->fieldType_ = fieldType;
  this->fieldName_ = pName;
  return PdbE_OK;
}
