#include "pdb_client.h"
#include "server/proto_header.h"
#include "util/coding.h"

using namespace boost::asio;

DBClient::DBClient()
  : socket_(ioService_)
{
  connected_ = false;
  port_ = 0;

  logined_ = false;
}

DBClient::~DBClient()
{
  if (connected_)
  {
    Disconnect();
  }
}

PdbErr_t DBClient::Connect(const char* pHost, int32_t port)
{
  std::unique_lock<std::mutex> clilock(cliMutex_);

  if (connected_)
  {
    socket_.close();
    connected_ = false;
    host_ = "";
    port_ = 0;
  }

  if (port < 0 || port > 65535)
  {
    return PdbE_INVALID_PARAM;
  }

  ip::tcp::endpoint endp(ip::address_v4::from_string(pHost), static_cast<unsigned short>(port));
  boost::system::error_code errCode;
  socket_.connect(endp, errCode);
  if (errCode)
  {
    return PdbE_NET_ERROR;
  }

  this->host_ = pHost;
  this->port_ = port;

  connected_ = true;
  return PdbE_OK;
}

PdbErr_t DBClient::Login(const char* pName, const char* pPwd)
{
  std::unique_lock<std::mutex> clilock(cliMutex_);

  PdbErr_t retVal = PdbE_OK;
  Arena arena;
  ProtoHeader proHdr;
  size_t packetLen = ProtoHeader::kProtoHeadLength + PDB_USER_NAME_LEN 
    + PDB_PWD_CRC32_LEN;

  if (pName == nullptr || pPwd == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  if (!connected_)
  {
    return PdbE_NET_ERROR;
  }

  size_t nameLen = strlen(pName);
  if (nameLen >= PDB_USER_NAME_LEN)
  {
    return PdbE_INVALID_PARAM;
  }

  char* pPacketBuf = arena.Allocate(packetLen);
  if (pPacketBuf == nullptr)
  {
    return PdbE_NOMEM;
  }

  proHdr.Load(pPacketBuf);


  //拷贝登录名
  strncpy((char*)(pPacketBuf + ProtoHeader::kProtoHeadLength), pName, PDB_USER_NAME_LEN);
  //设置密码
  uint32_t pwd = StringTool::CRC32(pPwd);
  Coding::FixedEncode32((pPacketBuf + ProtoHeader::kProtoHeadLength + PDB_USER_NAME_LEN), pwd);

  uint32_t bodyCrc = StringTool::CRC32((pPacketBuf + ProtoHeader::kProtoHeadLength),
    (PDB_USER_NAME_LEN + PDB_PWD_CRC32_LEN));

  //设置报文头
  proHdr.SetVersion(ProtoHeader::kProtoVersion);
  proHdr.SetMethod(METHOD_CMD_LOGIN_REQ);
  proHdr.SetBodyLen((uint32_t)(PDB_USER_NAME_LEN + PDB_PWD_CRC32_LEN));
  proHdr.SetReturnVal(PdbE_OK);
  proHdr.SetRecordCnt(0);
  proHdr.SetBodyCrc(bodyCrc);
  proHdr.UpdateHeadCrc();

  retVal = Request(pPacketBuf, packetLen);
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  char* pResBuf = nullptr;
  size_t resLen = 0;
  retVal = Recv(arena, &pResBuf, &resLen);
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  ProtoHeader resHdr;
  resHdr.Load(pResBuf);
  if (resHdr.GetMethod() != METHOD_CMD_LOGIN_REP)
  {
    return PdbE_PACKET_ERROR;
  }

  retVal = resHdr.GetReturnVal();
  if (retVal == PdbE_OK)
  {
    logined_ = true;
  }

  return retVal;
}

PdbErr_t DBClient::Disconnect()
{
  std::unique_lock<std::mutex> clilock(cliMutex_);

  if (connected_)
  {
    socket_.close();
    connected_ = false;
    host_ = "";
    port_ = 0;
    logined_ = false;
  }

  return PdbE_OK;
}

PdbErr_t DBClient::ExecuteInsert(const char* pSql, size_t* pSucessCnt, size_t* pPosition)
{
  PdbErr_t retVal = PdbE_OK;
  Arena arena;
  char* pPacketBuf = nullptr;
  size_t packetLen = 0;

  if (pSql == nullptr)
    return PdbE_INVALID_PARAM;

  if (pSucessCnt != nullptr)
    *pSucessCnt = 0;

  if (pPosition != nullptr)
    *pPosition = 0;

  std::unique_lock<std::mutex> cliLock(cliMutex_);
  if (!connected_)
    return PdbE_NET_ERROR;

  if (!logined_)
    return PdbE_NOT_LOGIN;

  retVal = MakeRequestPacket(arena, pSql, METHOD_CMD_INSERT_REQ, &pPacketBuf, &packetLen);
  if (retVal != PdbE_OK)
    return retVal;

  retVal = Request(pPacketBuf, packetLen);
  if (retVal != PdbE_OK)
    return retVal;

  retVal = Recv(arena, &pPacketBuf, &packetLen);
  if (retVal != PdbE_OK)
    return retVal;

  ProtoHeader proHdr;
  proHdr.Load(pPacketBuf);
  if (proHdr.GetMethod() != METHOD_CMD_INSERT_REP)
    return PdbE_PACKET_ERROR;

  if (pSucessCnt != nullptr)
    *pSucessCnt = proHdr.GetRecordCnt();

  if (pPosition != nullptr)
    *pPosition = proHdr.GetErrPos();

  return proHdr.GetReturnVal();
}

PdbErr_t DBClient::ExecuteQuery(const char* pSql, PDBDataTable* pTable)
{
  PdbErr_t retVal = PdbE_OK;
  Arena arena;

  if (pSql == nullptr || pTable == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  std::unique_lock<std::mutex> cliLock(cliMutex_);
  if (!connected_)
    return PdbE_NET_ERROR;

  if (!logined_)
    return PdbE_NOT_LOGIN;

  char* pPacketBuf = nullptr;
  size_t packetLen = 0;

  retVal = MakeRequestPacket(arena, pSql, METHOD_CMD_QUERY_REQ, &pPacketBuf, &packetLen);
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  retVal = Request(pPacketBuf, packetLen);
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  retVal = Recv(arena, &pPacketBuf, &packetLen);
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  ProtoHeader proHdr;
  proHdr.Load(pPacketBuf);
  if (proHdr.GetMethod() != METHOD_CMD_QUERY_REP)
  {
    return PdbE_PACKET_ERROR;
  }

  retVal = DecodePacket(pPacketBuf, packetLen, pTable);
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  return retVal;
}

PdbErr_t DBClient::ExecuteNonQuery(const char* pSql)
{
  PdbErr_t retVal = PdbE_OK;
  Arena arena;

  if (pSql == nullptr)
  {
    return PdbE_INVALID_HANDLE;
  }

  std::unique_lock<std::mutex> cliLock(cliMutex_);
  if (!connected_)
  {
    return PdbE_NET_ERROR;
  }

  if (!logined_)
  {
    return PdbE_NOT_LOGIN;
  }

  char* pPacketBuf = nullptr;
  size_t packetLen = 0;

  retVal = MakeRequestPacket(arena, pSql, METHOD_CMD_NONQUERY_REQ, &pPacketBuf, &packetLen);
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  retVal = Request(pPacketBuf, packetLen);
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  retVal = Recv(arena, &pPacketBuf, &packetLen);
  if (retVal != PdbE_OK)
  {
    return retVal;
  }

  ProtoHeader proHdr;
  proHdr.Load(pPacketBuf);
  if (proHdr.GetMethod() != METHOD_CMD_NONQUERY_REP)
  {
    return PdbE_PACKET_ERROR;
  }

  retVal = proHdr.GetReturnVal();

  return retVal;
}

PdbErr_t DBClient::DecodePacket(char* pPacket, size_t packetLen, PDBDataTable* pTable)
{
  ProtoHeader proHdr;
  proHdr.Load(pPacket);

  PdbErr_t retVal = PdbE_OK;

  retVal = (PdbErr_t)proHdr.GetReturnVal();
  if (retVal != PdbE_OK)
    return retVal;

  size_t offset = ProtoHeader::kProtoHeadLength;
  size_t recLen = 0;
  size_t fieldCnt = proHdr.GetFieldCnt();

  DBObj tmpObj(nullptr);

  do {
    //初始化列信息
    retVal = tmpObj.ParseTrans(fieldCnt, (pPacket + offset), 
      (pPacket + packetLen), &recLen);
    if (retVal != PdbE_OK)
      break;

    retVal = InitColumnInfo(&tmpObj, pTable);
    if (retVal != PdbE_OK)
      break;

    offset += recLen;

    while (offset < packetLen)
    {
      retVal = tmpObj.ParseTrans(fieldCnt, (pPacket + offset), (pPacket + packetLen), &recLen);
      if (retVal != PdbE_OK)
        break;

      retVal = pTable->AddRow(&tmpObj);
      if (retVal != PdbE_OK)
        break;

      offset += recLen;

    }

  } while (false);

  return retVal;
}


PdbErr_t DBClient::InitColumnInfo(const DBObj* pObj, PDBDataTable* pTable)
{
  PdbErr_t retVal = PdbE_OK;

  if (pObj == nullptr || pTable == nullptr)
    return PdbE_INVALID_PARAM;

  size_t fieldCnt = pObj->GetFieldCnt();

  for (size_t i = 0; i < fieldCnt; i++)
  {
    const DBVal* pVal = pObj->GetFieldValue(i);
    if (pVal == nullptr)
      return PdbE_PACKET_ERROR;

    if (!DBVAL_IS_STRING(pVal))
      return PdbE_PACKET_ERROR;

    const char* pStr = DBVAL_GET_STRING(pVal);
    size_t strLen = DBVAL_GET_LEN(pVal);

    if (StringTool::StartWithNoCase(pStr, "bool;"))
      retVal = pTable->AddColumn((pStr + 5), (strLen - 5), PDB_FIELD_TYPE::TYPE_BOOL);
    else if (StringTool::StartWithNoCase(pStr, "bigint;"))
      retVal = pTable->AddColumn((pStr + 7), (strLen - 7), PDB_FIELD_TYPE::TYPE_INT64);
    else if (StringTool::StartWithNoCase(pStr, "double;"))
      retVal = pTable->AddColumn((pStr + 7), (strLen - 7), PDB_FIELD_TYPE::TYPE_DOUBLE);
    else if (StringTool::StartWithNoCase(pStr, "string;"))
      retVal = pTable->AddColumn((pStr + 7), (strLen - 7), PDB_FIELD_TYPE::TYPE_STRING);
    else if (StringTool::StartWithNoCase(pStr, "blob;"))
      retVal = pTable->AddColumn((pStr + 5), (strLen - 5), PDB_FIELD_TYPE::TYPE_BLOB);
    else if (StringTool::StartWithNoCase(pStr, "datetime;"))
      retVal = pTable->AddColumn((pStr + 9), (strLen - 9), PDB_FIELD_TYPE::TYPE_DATETIME);
    else
      return PdbE_PACKET_ERROR;

    if (retVal != PdbE_OK)
      return retVal;
  }
  
  return PdbE_OK;
}

PdbErr_t DBClient::MakeRequestPacket(Arena& arena, const char* pSql,
  uint32_t method, char** ppPacketBuf, size_t* pPacketLen)
{
  int32_t bodyLen = 0;
  int32_t totalLen = 0;

  if (pSql == nullptr || ppPacketBuf == nullptr || pPacketLen == nullptr)
  {
    return PdbE_INVALID_PARAM;
  }

  bodyLen = static_cast<int32_t>(strlen(pSql));
  totalLen = ProtoHeader::kProtoHeadLength + bodyLen;

  char* pTmpBuf = arena.Allocate(totalLen);
  if (pTmpBuf == nullptr)
    return PdbE_NOMEM;

  memcpy((pTmpBuf + ProtoHeader::kProtoHeadLength), pSql, bodyLen);
  uint32_t bodyCrc = StringTool::CRC32((pTmpBuf + ProtoHeader::kProtoHeadLength), bodyLen);

  ProtoHeader proHdr;
  proHdr.Load(pTmpBuf);
  proHdr.SetVersion(ProtoHeader::kProtoVersion);
  proHdr.SetMethod(method);
  proHdr.SetBodyLen((uint32_t)bodyLen);
  proHdr.SetReturnVal(PdbE_OK);
  proHdr.SetRecordCnt(1);
  proHdr.SetBodyCrc(bodyCrc);
  proHdr.UpdateHeadCrc();

  *ppPacketBuf = pTmpBuf;
  *pPacketLen = totalLen;
  return PdbE_OK;
}

PdbErr_t DBClient::Request(const char* pBuf, size_t len)
{
  if (!connected_)
  {
    return PdbE_NET_ERROR;
  }

  size_t sendLen = socket_.send(buffer(pBuf, len));

  if (sendLen != len)
    return PdbE_NET_ERROR;

  return PdbE_OK;
}

PdbErr_t DBClient::Recv(Arena& arena, char** ppDataBuf, size_t* pDataLen)
{
  boost::system::error_code errCode;
  size_t totalLen = 0;
  PdbErr_t retVal = PdbE_OK;

  char tmpHead[ProtoHeader::kProtoHeadLength] = { 0 };

  while (totalLen < ProtoHeader::kProtoHeadLength)
  {
    size_t tmpLen = ProtoHeader::kProtoHeadLength - totalLen;
    size_t recvLen = socket_.read_some(buffer((tmpHead + totalLen), tmpLen), errCode);
    if (errCode)
      return PdbE_NET_ERROR;

    totalLen += recvLen;
  }

  ProtoHeader proHdr;
  proHdr.Load(tmpHead);
  size_t bodyLen = static_cast<size_t>(proHdr.GetBodyLen());

  char* pTmpBuf = arena.Allocate((bodyLen + ProtoHeader::kProtoHeadLength));
  if (pTmpBuf == nullptr)
    return PdbE_NOMEM;

  memcpy(pTmpBuf, tmpHead, ProtoHeader::kProtoHeadLength);
  totalLen = 0;

  while (totalLen < bodyLen)
  {
    size_t tmpLen = bodyLen - totalLen;
    char* pRecv = pTmpBuf + ProtoHeader::kProtoHeadLength + totalLen;
    size_t recvLen = socket_.read_some(buffer(pRecv, tmpLen), errCode);
    if (errCode)
    {
      retVal = PdbE_NET_ERROR;
      break;
    }

    totalLen += recvLen;
  }

  if (retVal == PdbE_OK)
  {
    *ppDataBuf = pTmpBuf;
    *pDataLen = (bodyLen + ProtoHeader::kProtoHeadLength);
  }

  return retVal;
}


