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
  static PdbErr_t RunInsertParser(Arena* pArena, InsertSql* pInsertSql, const char* pSql, size_t sqlLen);

private:
  static PdbErr_t StrToBlob(const char* pSourceStr, size_t sourceLen, uint8_t* pDest, size_t* pDestLen);

  static PdbErr_t GetStr(const char* pSourceStr, size_t sourceLen, char* pDest, size_t* pDestLen);

  static int GetToken(const unsigned char* z, size_t maxLen, int* tokenType, bool* needEscape);

  static int GetKeywordType(const char* z, int n);


private:
  static std::unordered_map<uint64_t, int> keywordMap_;

};

