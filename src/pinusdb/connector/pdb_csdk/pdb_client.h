#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <stdint.h>
#include "pdb.h"
#include "pdb_error.h"
#include "util/arena.h"

#include "boost/asio.hpp"
#include "pdb_datatable.h"

class ErrorInfo
{
public:
  ErrorInfo(int32_t idx, PdbErr_t errCode, int32_t errPos)
  {
    this->idx_ = idx;
    this->errCode_ = errCode;
    this->errPos_ = errPos;
  }

  ~ErrorInfo() { }

  ErrorInfo(const ErrorInfo& errInfo)
  {
    this->idx_ = errInfo.idx_;
    this->errCode_ = errInfo.errCode_;
    this->errPos_ = errInfo.errPos_;
  }

  ErrorInfo& operator=(const ErrorInfo& errInfo)
  {
    this->idx_ = errInfo.idx_;
    this->errCode_ = errInfo.errCode_;
    this->errPos_ = errInfo.errPos_;

    return *this;
  }

  int32_t idx_;
  PdbErr_t errCode_;
  int32_t errPos_;
};

class DBClient
{
public:
  DBClient();
  ~DBClient();

  PdbErr_t Connect(const char* pHost, int32_t port);
  PdbErr_t Login(const char* pName, const char* pPwd);
  PdbErr_t Disconnect();

  PdbErr_t ExecuteInsert(const char* pSql, size_t* pSucessCnt, size_t* pPosition);
  PdbErr_t ExecuteQuery(const char* pSql, PDBDataTable* pTable);
  PdbErr_t ExecuteNonQuery(const char* pSql);

private:

  PdbErr_t DecodePacket(uint8_t* pPacket, size_t packetLen, PDBDataTable* pTable);
  PdbErr_t InitColumnInfo(const DBObj* pObj, PDBDataTable* pTable);
  PdbErr_t MakeRequestPacket(Arena& arena, const char* pSql, 
    uint32_t method, uint8_t** ppPacketBuf, size_t* pPacketLen);

  PdbErr_t Request(const uint8_t* pBuf, size_t len);
  PdbErr_t Recv(Arena& arena, uint8_t** ppDataBuf, size_t* pDataLen);

private:
  bool connected_;
  std::string host_;
  int32_t port_;

  bool logined_;

  std::mutex cliMutex_;

  boost::asio::io_service ioService_;
  boost::asio::ip::tcp::socket socket_;
};

