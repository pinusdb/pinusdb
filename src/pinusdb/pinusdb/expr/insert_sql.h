#pragma once

#include "internal.h"
#include "table/db_value.h"
#include "table/table_info.h"

typedef struct _SqlToken
{
  int32_t type_;
  int32_t len_;
  const char* str_;
}SqlToken;

class IInsertObj
{
public:
  IInsertObj() {}
  virtual ~IInsertObj() {}

  virtual bool IsEnd() const = 0;
  virtual PdbErr_t ParseMeta() = 0;
  virtual std::string GetTableName() const = 0;
  virtual PdbErr_t InitTableInfo(const TableInfo* pTabInfo) = 0;
  virtual PdbErr_t GetNextRec(int* pTypes, DBVal* pVals, size_t valCnt) = 0;
};

class InsertSql : public IInsertObj
{
public:
  InsertSql();
  virtual ~InsertSql();

  PdbErr_t AddToken(int32_t type, int32_t len, const char* pStr);
  
  virtual bool IsEnd() const { return curIter_ == tokenList_.end(); }
  virtual PdbErr_t ParseMeta();
  virtual std::string GetTableName() const { return tabName_; }
  virtual PdbErr_t InitTableInfo(const TableInfo* pTabInfo);
  virtual PdbErr_t GetNextRec(int* pTypes, DBVal* pVals, size_t valCnt);

private:
  std::string tabName_;
  std::vector<std::string> colNameVec_;
  std::vector<size_t> posVec_;

  std::list<SqlToken> tokenList_;
  std::list<SqlToken>::iterator curIter_;
};

class InsertTable : public IInsertObj
{
public:
  InsertTable(size_t fieldCnt, size_t recCnt);
  virtual ~InsertTable();

  PdbErr_t AddVal(const DBVal* pVal);

  virtual bool IsEnd() const { return valIter_ == valList_.end(); }
  virtual PdbErr_t ParseMeta();
  virtual std::string GetTableName() const { return tabName_; }
  virtual PdbErr_t InitTableInfo(const TableInfo* pTabInfo);
  virtual PdbErr_t GetNextRec(int* pTypes, DBVal* pVals, size_t valCnt);

private:
  size_t fieldCnt_;
  size_t recCnt_;
  std::string tabName_;
  std::vector<std::string> colNameVec_;
  std::vector<size_t> posVec_;

  std::list<DBVal> valList_;
  std::list<DBVal>::const_iterator valIter_;

};