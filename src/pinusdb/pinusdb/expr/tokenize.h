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

#include "internal.h"
#include "expr/pdb_db_int.h"
#include "expr/sql_parser.h"
#include "expr/insert_sql.h"
#include "util/arena.h"

class Tokenize
{
public:

  static void InitTokenize();

  static PdbErr_t RunParser(Arena* pArena, SQLParser *pParse, const char* pSql, size_t sqlLen);

private:
  static PdbErr_t StrToBlob(const char* pSourceStr, size_t sourceLen, uint8_t* pDest, size_t* pDestLen);

  static PdbErr_t GetStr(const char* pSourceStr, size_t sourceLen, char* pDest, size_t* pDestLen);

  static int GetToken(const unsigned char* z, size_t maxLen, int* tokenType, bool* needEscape);

  static int GetKeywordType(const char* z, int n);


private:
  static std::unordered_map<uint64_t, int> keywordMap_;

};

