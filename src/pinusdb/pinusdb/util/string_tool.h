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

#pragma once

#include <string>
#include <stdint.h>

#define CRC64_TO_CRC32(crc64) static_cast<uint32_t>((crc64) & 0xFFFFFFFF)

class StringTool
{
public:
  static bool ValidUserName(const char* pUserName, size_t nameLen);
  static bool ValidTableName(const char* pTabName, size_t nameLen);
  static bool ValidColumnName(const char* pColName, size_t nameLen);

  static bool ComparyNoCase(const char* pStr1, const char* pStr2);
  static bool ComparyNoCase(const char* pStr1, size_t len1, const char* pStr2, size_t len2);
  static bool ComparyNoCase(const std::string& str1, const char* pStr2, size_t len2);

  static bool Utf8LikeCompare(const char* zPattern, const char* zString, size_t strLen);

  static bool StartWith(const char* pStr, const char* pStartPart);
  static bool StartWithNoCase(const char* pStr, const char* pStartPart);

  static std::string ConvertGbkToUtf8(const std::string& strGbk);

  /////////////////////////////////////////////////////////////////////////////////////////////////

  static uint64_t CRC64(const char* pStr);
  static uint64_t CRC64(const void* pData, size_t len);
  static uint64_t CRC64(const void* pData, size_t len, size_t offset);
  static uint64_t CRC64(const void* pData, size_t len, size_t offset, uint64_t crc);

  static uint64_t CRC64NoCase(const char* pStr);
  static uint64_t CRC64NoCase(const char* pStr, size_t len);
  
  static uint32_t CRC32(const char* pStr);
  static uint32_t CRC32(const void* pData, size_t len);
  static uint32_t CRC32(const void* pData, size_t len, size_t offset);


  static bool StrToInt64(const char* pStr, size_t len, int64_t* pVal);
  static bool StrToDouble(const char* pStr, size_t len, double* pVal);

  static bool ValidName(const char* pName, size_t nameLen);

};
