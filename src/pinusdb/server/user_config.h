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
#include "table/table_info.h"
#include "query/iquery.h"

class UserInfo
{
public:
  UserInfo(const char* pUserName, uint32_t pwd,
    int32_t role, const char* pRoleName);

  const std::string& GetName() const;
  int32_t GetRole() const;
  const std::string& GetRoleName() const;
  uint32_t GetPwd() const;

  void SetRole(int32_t role);
  void SetPwd(uint32_t pwd);

private:
  //no copy
  UserInfo(const UserInfo& cpy);
  const UserInfo& operator=(const UserInfo& cpy);

private:
  std::string userName_;
  std::string roleName_;
  uint32_t pwd_;
  int32_t role_;
};

class UserConfig
{
public:
  UserConfig();
  ~UserConfig();

  PdbErr_t Load();

  PdbErr_t AddUser(const char* pUserName, uint32_t pwd, const char* pRoleName);
  PdbErr_t Login(const char* pUserName, uint32_t pwd, int32_t* pRole);
  PdbErr_t Query(IQuery* pQuery);

  PdbErr_t ChangePwd(const std::string& userName, uint32_t pwd);
  PdbErr_t ChangeRole(const char* pUserName, const char* pRoleName);
  PdbErr_t DropUser(const std::string& userName);

private:
  bool _SaveUser();

private:
  std::mutex userMutex_;
  std::string cfgPath_;
  std::unordered_map<uint64_t, UserInfo*> userMap_;
};

