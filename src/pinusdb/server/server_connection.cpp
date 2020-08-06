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

#include "server/server_connection.h"
#include "util/date_time.h"
#include "util/log_util.h"
#include "expr/sql_parser.h"

inline void SetRemoteInfo(ConnInfo* pConnInfo, const char* pRemoteHost, int remotePort)
{
  strncpy(pConnInfo->remoteHost_, pRemoteHost, IP_ADDR_STR_LEN);
  pConnInfo->remoteHost_[IP_ADDR_STR_LEN - 1] = '\0';
  pConnInfo->remotePort_ = remotePort;
  pConnInfo->connTime_ = DateTime::NowMilliseconds();
  pConnInfo->role_ = 0;
  pConnInfo->loginName_[0] = '\0';
}

inline void SetLoginInfo(ConnInfo* pConnInfo, const char* pLoginName, int role)
{
  strncpy(pConnInfo->loginName_, pLoginName, PDB_USER_NAME_LEN);
  pConnInfo->loginName_[PDB_USER_NAME_LEN - 1] = '\0';

  pConnInfo->role_ = role;
}

ServerConnection::ServerConnection()
{
  INIT_LIST_HEAD(&freeConnList_);

  blkCnt_ = 0;

  for (int i = 0; i < kMaxBlkCnt; i++)
  {
    ppBlk_[i] = nullptr;
  }
}

ServerConnection::~ServerConnection()
{
  for (int i = 0; i < blkCnt_; i++)
  {
    free(ppBlk_[i]);
  }

  connMap_.clear();
}


bool ServerConnection::AddConnection(uint64_t connKey, const char* pRemoteHost, int remotePort)
{
  std::unique_lock<std::mutex> connLock(connMutex_);
  auto connIt = connMap_.find(connKey);
  if (connIt != connMap_.end())
  {
    SetRemoteInfo(connIt->second, pRemoteHost, remotePort);
    return true;
  }
  else
  {
    ConnInfo* pNewConn = MallocConnInfo();
    if (pNewConn != nullptr)
    {
      SetRemoteInfo(pNewConn, pRemoteHost, remotePort);
      connMap_.insert(std::pair<uint64_t, ConnInfo*>(connKey, pNewConn));
      return true;
    }
  }

  return false;
}

void ServerConnection::ConnectionLogin(uint64_t connKey, const char* pLoginName, int role)
{
  std::unique_lock<std::mutex> connLock(connMutex_);
  auto connIt = connMap_.find(connKey);
  if (connIt != connMap_.end())
  {
    SetLoginInfo(connIt->second, pLoginName, role);
  }
}

void ServerConnection::DelConnection(uint64_t connKey)
{
  struct list_head* pNode = nullptr;
  std::unique_lock<std::mutex> connLock(connMutex_);
  auto connIt = connMap_.find(connKey);
  if (connIt != connMap_.end())
  {
    pNode = (struct list_head*)(connIt->second);

    INIT_NODE(pNode);
    list_add(pNode, (&freeConnList_));

    connMap_.erase(connIt);
  }
}

PdbErr_t ServerConnection::QueryConn(IQuery* pQuery)
{
  PdbErr_t retVal = PdbE_OK;

  const int valCnt = 5;
  DBVal vals[valCnt];

  std::unique_lock<std::mutex> connLock(connMutex_);
  for (auto connIt = connMap_.begin(); connIt != connMap_.end(); connIt++)
  {
    DBVAL_ELE_SET_STRING(vals, 0, connIt->second->remoteHost_, strlen(connIt->second->remoteHost_));
    DBVAL_ELE_SET_INT64(vals, 1, connIt->second->remotePort_);

    std::string roleName = "";
    switch (connIt->second->role_)
    {
    case PDB_ROLE::ROLE_READ_ONLY:
      roleName = PDB_USER_ROLE_READONLY_STR;
      break;
    case PDB_ROLE::ROLE_WRITE_ONLY:
      roleName = PDB_USER_ROLE_WRITEONLY_STR;
      break;
    case PDB_ROLE::ROLE_READ_WRITE:
      roleName = PDB_USER_ROLE_READWRITE_STR;
      break;
    case PDB_ROLE::ROLE_ADMIN:
      roleName = PDB_USER_ROLE_ADMIN_STR;
      break;
    }

    if (roleName.size() > 0)
    {
      DBVAL_ELE_SET_STRING(vals, 2, connIt->second->loginName_, strlen(connIt->second->loginName_));
      DBVAL_ELE_SET_STRING(vals, 3, roleName.c_str(), roleName.size());
    }
    else
    {
      DBVAL_ELE_SET_STRING(vals, 2, "", 0);
      DBVAL_ELE_SET_STRING(vals, 3, "", 0);
    }

    DBVAL_ELE_SET_DATETIME(vals, 4, connIt->second->connTime_);
    
    retVal = pQuery->AppendData(vals, valCnt, nullptr);
    if (retVal != PdbE_OK)
      break;

    if (pQuery->GetIsFullFlag())
      break;

  }

  return retVal;
}

ConnInfo* ServerConnection::MallocConnInfo()
{
  struct list_head* pNode = nullptr;

  if (list_empty(&freeConnList_) && blkCnt_ < kMaxBlkCnt)
  {
    void* pNewBlk = malloc(sizeof(ConnInfo) * kConnCntPerBlk);
    ppBlk_[blkCnt_++] = pNewBlk;

    for (int i = 0; i < kConnCntPerBlk; i++)
    {
      pNode = (struct list_head*)(((uint8_t*)pNewBlk) + sizeof(ConnInfo) * i);
      INIT_NODE(pNode);
      list_add(pNode, (&freeConnList_));
    }
  }

  if (!list_empty(&freeConnList_))
  {
    struct list_head* pTmp = freeConnList_.next;
    list_del(pTmp);
    return (ConnInfo*)pTmp;
  }

  LOG_ERROR("failed to MallocConnInfo, too many connections");

  return nullptr;
}
