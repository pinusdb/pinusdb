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

#include "db_obj.h"
#include "pdb_error.h"
#include "util/coding.h"
#include <vector>

void DBObj::Clear()
{
  fieldVec_.clear();
}

size_t DBObj::GetFieldCnt() const
{
  return fieldVec_.size();
}
const DBVal* DBObj::GetFieldValue(size_t idx) const
{
  assert(idx < fieldVec_.size());
  return (&(fieldVec_[idx]));
}

void DBObj::AppendNullVal()
{
  DBVal tmpVal;
  DBVAL_SET_NULL(&tmpVal);
  fieldVec_.push_back(tmpVal);
}

void DBObj::AppendVal(bool val)
{
  DBVal tmpVal;
  DBVAL_SET_BOOL(&tmpVal, val);
  fieldVec_.push_back(tmpVal);
}

void DBObj::AppendVal(int8_t val)
{
  DBVal tmpVal;
  DBVAL_SET_INT8(&tmpVal, val);
  fieldVec_.push_back(tmpVal);
}

void DBObj::AppendVal(int16_t val)
{
  DBVal tmpVal;
  DBVAL_SET_INT16(&tmpVal, val);
  fieldVec_.push_back(tmpVal);
}

void DBObj::AppendVal(int32_t val)
{
  DBVal tmpVal;
  DBVAL_SET_INT32(&tmpVal, val);
  fieldVec_.push_back(tmpVal);
}

void DBObj::AppendVal(int64_t val)
{
  DBVal tmpVal;
  DBVAL_SET_INT64(&tmpVal, val);
  fieldVec_.push_back(tmpVal);
}

void DBObj::AppendVal(float val)
{
  DBVal tmpVal;
  DBVAL_SET_FLOAT(&tmpVal, val);
  fieldVec_.push_back(tmpVal);
}

void DBObj::AppendVal(double val)
{
  DBVal tmpVal;
  DBVAL_SET_DOUBLE(&tmpVal, val);
  fieldVec_.push_back(tmpVal);
}
PdbErr_t DBObj::AppendStrVal(const char* pVal, size_t len)
{
  DBVal tmpVal;

  if (len > 0)
  {
    if (pArena_ == nullptr)
    {
      DBVAL_SET_STRING(&tmpVal, pVal, len);
    }
    else
    {
      char* pTmp = pArena_->Allocate((len + 1));
      if (pTmp == nullptr)
        return PdbE_NOMEM;

      memcpy(pTmp, pVal, len);
      pTmp[len] = '\0';
      DBVAL_SET_STRING(&tmpVal, pTmp, len);
    }
  }
  else
  {
    DBVAL_SET_STRING(&tmpVal, nullptr, 0);
  }

  fieldVec_.push_back(tmpVal);
  return PdbE_OK;
}
PdbErr_t DBObj::AppendBlobVal(const uint8_t* pVal, size_t len)
{
  DBVal tmpVal;

  if (len > 0)
  {
    if (pArena_ != nullptr)
    {
      PdbByte* pTmp = (PdbByte*)pArena_->Allocate(len);
      if (pTmp == nullptr)
        return PdbE_NOMEM;

      memcpy(pTmp, pVal, len);
      DBVAL_SET_BLOB(&tmpVal, pTmp, len);
    }
    else
    {
      DBVAL_SET_BLOB(&tmpVal, pVal, len);
    }
  }
  else
  {
    DBVAL_SET_BLOB(&tmpVal, nullptr, 0);
  }

  fieldVec_.push_back(tmpVal);
  return PdbE_OK;

}
void DBObj::AppendDateTime(int64_t val)
{
  DBVal tmpVal;
  DBVAL_SET_DATETIME(&tmpVal, val);
  fieldVec_.push_back(tmpVal);
}

PdbErr_t DBObj::AppendVal(const DBVal* pVal)
{
  if (DBVAL_IS_STRING(pVal))
  {
    return AppendStrVal(DBVAL_GET_STRING(pVal), DBVAL_GET_LEN(pVal));
  }
  else if (DBVAL_IS_BLOB(pVal))
  {
    return AppendBlobVal(DBVAL_GET_BLOB(pVal), DBVAL_GET_LEN(pVal));
  }
  else
  {
    fieldVec_.push_back(*pVal);
  }

  return PdbE_OK;
}

PdbErr_t DBObj::ParseTrans(size_t fieldCnt, const char* pData, 
  const char* pLimit, size_t* pObjLen)
{
  PdbErr_t retVal = PdbE_OK;
  const char* pTmp = pData;
  uint64_t v64 = 0;
  uint32_t v32 = 0;
  uint32_t vType = 0;

  Clear();

  for (size_t i = 0; i < fieldCnt; i++)
  {
    vType = *pTmp++;
    switch (vType)
    {
    case PDB_VALUE_TYPE::VAL_NULL:
      AppendNullVal();
      break;
    case PDB_VALUE_TYPE::VAL_BOOL:
      AppendVal((*pTmp == PDB_BOOL_TRUE));
      pTmp++;
      break;
    case PDB_VALUE_TYPE::VAL_INT8:
      pTmp = Coding::VarintDecode64(pTmp, pLimit, &v64);
      AppendVal(static_cast<int8_t>(Coding::ZigzagDecode64(v64)));
      break;
    case PDB_VALUE_TYPE::VAL_INT16:
      pTmp = Coding::VarintDecode64(pTmp, pLimit, &v64);
      AppendVal(static_cast<int16_t>(Coding::ZigzagDecode64(v64)));
      break;
    case PDB_VALUE_TYPE::VAL_INT32:
      pTmp = Coding::VarintDecode64(pTmp, pLimit, &v64);
      AppendVal(static_cast<int32_t>(Coding::ZigzagDecode64(v64)));
      break;
    case PDB_VALUE_TYPE::VAL_INT64:
      pTmp = Coding::VarintDecode64(pTmp, pLimit, &v64);
      AppendVal(Coding::ZigzagDecode64(v64));
      break;
    case PDB_VALUE_TYPE::VAL_DATETIME:
      pTmp = Coding::VarintDecode64(pTmp, pLimit, &v64);
      AppendDateTime(Coding::ZigzagDecode64(v64));
      break;
    case PDB_VALUE_TYPE::VAL_FLOAT:
      v32 = Coding::FixedDecode32(pTmp);
      AppendVal(Coding::DecodeFloat(v32));
      pTmp += sizeof(float);
      break;
    case PDB_VALUE_TYPE::VAL_DOUBLE:
      v64 = Coding::FixedDecode64(pTmp);
      AppendVal(Coding::DecodeDouble(v64));
      pTmp += sizeof(double);
      break;
    case PDB_VALUE_TYPE::VAL_STRING:
      pTmp = Coding::VarintDecode32(pTmp, pLimit, &v32);
      retVal = AppendStrVal(pTmp, v32);
      pTmp += v32;
      break;
    case PDB_VALUE_TYPE::VAL_BLOB:
      pTmp = Coding::VarintDecode32(pTmp, pLimit, &v32);
      retVal = AppendBlobVal((const uint8_t*)pTmp, v32);
      pTmp += v32;
      break;
    default:
      return PdbE_PACKET_ERROR;
    }

    if (retVal != PdbE_OK)
      return retVal;

    if (pTmp == nullptr)
      return PdbE_PACKET_ERROR;
  }

  if (pObjLen != nullptr)
    *pObjLen = (pTmp - pData);

  return PdbE_OK;
}
