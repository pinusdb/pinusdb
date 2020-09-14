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

#include "internal.h"

#include "server/user_config.h"
#include "pdb_error.h"
#include "boost/filesystem.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/foreach.hpp"
#include "util/string_tool.h"
#include "util/log_util.h"
#include "table/db_value.h"
#include "global_variable.h"
#include "sys_table_schema.h"

UserInfo::UserInfo(const char* pUserName, uint32_t pwd, 
  int32_t role, const char* pRoleName)
{
  this->userName_ = pUserName;
  this->roleName_ = pRoleName;
  this->pwd_ = pwd;
  this->role_ = role;
}

const std::string& UserInfo::GetName() const
{
  return userName_;
}

int32_t UserInfo::GetRole() const
{
  return this->role_;
}

const std::string& UserInfo::GetRoleName() const
{
  return this->roleName_;
}

uint32_t UserInfo::GetPwd() const
{
  return pwd_;
}

void UserInfo::SetRole(int32_t role)
{
  this->role_ = role;
  switch (role)
  {
  case PDB_ROLE::ROLE_READ_ONLY:
    this->roleName_ = PDB_USER_ROLE_READONLY_STR;
    break;
  case PDB_ROLE::ROLE_WRITE_ONLY:
    this->roleName_ = PDB_USER_ROLE_WRITEONLY_STR;
    break;
  case PDB_ROLE::ROLE_READ_WRITE:
    this->roleName_ = PDB_USER_ROLE_READWRITE_STR;
    break;
  case PDB_ROLE::ROLE_ADMIN:
    this->roleName_ = PDB_USER_ROLE_ADMIN_STR;
    break;
  default:
    assert(false);
  }
}
void UserInfo::SetPwd(uint32_t pwd)
{
  this->pwd_ = pwd;
}

UserConfig::UserConfig()
{}

UserConfig::~UserConfig()
{
  for (auto userIt = userMap_.begin(); userIt != userMap_.end(); userIt++)
  {
    delete userIt->second;
  }
  userMap_.clear();
}

PdbErr_t UserConfig::Load()
{
  cfgPath_ = pGlbSysCfg->GetTablePath() + "/user.json";
  if (!FileTool::FileExists(cfgPath_.c_str()))
  {
    AddUser("sa", 1350059811, PDB_USER_ROLE_ADMIN_STR);
    for (auto userIt = userMap_.begin(); userIt != userMap_.end(); userIt++)
    {
      delete userIt->second;
    }
    userMap_.clear();
  }

  try
  {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(cfgPath_, pt);
    if (pt.count("users") > 0)
    {
      boost::property_tree::ptree items;
      items = pt.get_child("users");

      for (boost::property_tree::ptree::iterator it = items.begin();
        it != items.end(); it++)
      {
        std::string name = it->second.get<std::string>(SYSCOL_SYSUSER_USERNAME);
        std::string roleName = it->second.get<std::string>(SYSCOL_SYSUSER_ROLE);
        uint32_t pwd = it->second.get<uint32_t>(SYSCOL_SYSUSER_PASSWORD);
        int32_t roleVal = 0;

        if (!StringTool::ValidUserName(name.c_str(), name.size()))
        {
          LOG_ERROR("invalid user({}) name", name.c_str());
          return PdbE_INVALID_USER_NAME;
        }

        if (StringTool::ComparyNoCase(roleName.c_str(), PDB_USER_ROLE_READONLY_STR))
          roleVal = PDB_ROLE::ROLE_READ_ONLY;
        else if (StringTool::ComparyNoCase(roleName.c_str(), PDB_USER_ROLE_WRITEONLY_STR))
          roleVal = PDB_ROLE::ROLE_WRITE_ONLY;
        else if (StringTool::ComparyNoCase(roleName.c_str(), PDB_USER_ROLE_READWRITE_STR))
          roleVal = PDB_ROLE::ROLE_READ_WRITE;
        else if (StringTool::ComparyNoCase(roleName.c_str(), PDB_USER_ROLE_ADMIN_STR))
          roleVal = PDB_ROLE::ROLE_ADMIN;
        else
        {
          LOG_ERROR("invalid user role({})", roleName.c_str());
          return PdbE_INVALID_USER_ROLE;
        }

        uint64_t userNameCrc = StringTool::CRC64NoCase(name.c_str());

        auto userIt = userMap_.find(userNameCrc);
        if (userIt != userMap_.end())
        {
          LOG_ERROR("user({}) exists", name.c_str());
          return PdbE_USER_EXIST;
        }

        UserInfo* pUser = new UserInfo(name.c_str(), pwd, roleVal, roleName.c_str());
        if (pUser == nullptr)
        {
          return PdbE_NOMEM;
        }

        std::pair<uint64_t, UserInfo*> userPair(userNameCrc, pUser);
        userMap_.insert(userPair);
      }
    }
  }
  catch (boost::property_tree::ptree_error pt_error)
  {
    LOG_ERROR("failed to load user config file");
    return PdbE_USER_CFG_ERROR;
  }

  return PdbE_OK;
}

PdbErr_t UserConfig::AddUser(const char* pUserName, uint32_t pwd, const char* pRoleName)
{
  int roleVal = 0;
  std::unique_lock<std::mutex> userLock(this->userMutex_);
  if (!StringTool::ValidUserName(pUserName, strlen(pUserName)))
  {
    LOG_ERROR("invalid user({}) name", pUserName);
    return PdbE_INVALID_USER_NAME;
  }

  if (StringTool::ComparyNoCase(pRoleName, PDB_USER_ROLE_READONLY_STR))
    roleVal = PDB_ROLE::ROLE_READ_ONLY;
  else if (StringTool::ComparyNoCase(pRoleName, PDB_USER_ROLE_WRITEONLY_STR))
    roleVal = PDB_ROLE::ROLE_WRITE_ONLY;
  else if (StringTool::ComparyNoCase(pRoleName, PDB_USER_ROLE_READWRITE_STR))
    roleVal = PDB_ROLE::ROLE_READ_WRITE;
  else if (StringTool::ComparyNoCase(pRoleName, PDB_USER_ROLE_ADMIN_STR))
    roleVal = PDB_ROLE::ROLE_ADMIN;
  else
  {
    LOG_ERROR("invalid user role({})", pRoleName);
    return PdbE_INVALID_USER_ROLE;
  }

  uint64_t userNameCrc = StringTool::CRC64NoCase(pUserName);
  auto userIt = userMap_.find(userNameCrc);
  if (userIt != userMap_.end())
  {
    LOG_ERROR("user({}) exists");
    return PdbE_USER_EXIST;
  }

  UserInfo* pUserInfo = new UserInfo(pUserName, pwd, roleVal, pRoleName);
  if (pUserInfo == nullptr)
  {
    LOG_ERROR("allocate memory failed");
    return PdbE_NOMEM;
  }

  userMap_.insert(std::pair<uint64_t, UserInfo*>(userNameCrc, pUserInfo));
  _SaveUser();

  return PdbE_OK;
}

PdbErr_t UserConfig::Login(const char* pUserName, uint32_t pwd, int32_t* pRole)
{
  std::unique_lock<std::mutex> userLock(this->userMutex_);
  uint64_t userNameCrc = StringTool::CRC64NoCase(pUserName);

  auto userIt = userMap_.find(userNameCrc);
  if (userIt != userMap_.end())
  {
    if (pwd == userIt->second->GetPwd())
    {
      if (pRole != nullptr)
      {
        *pRole = userIt->second->GetRole();
      }

      return PdbE_OK;
    }

    return PdbE_PASSWORD_ERROR;
  }

  return PdbE_USER_NOT_FOUND;
}

PdbErr_t UserConfig::Query(IQuery* pQuery)
{
  PdbErr_t retVal = PdbE_OK;
  const int valCnt = 2;
  DBVal vals[valCnt];

  std::unique_lock<std::mutex> userLock(userMutex_);
  for (auto userIt = userMap_.begin(); userIt != userMap_.end(); userIt++)
  {
    const std::string& userName = userIt->second->GetName();
    const std::string& roleName = userIt->second->GetRoleName();
    
    DBVAL_ELE_SET_STRING(vals, 0, userName.c_str(), userName.size());
    DBVAL_ELE_SET_STRING(vals, 1, roleName.c_str(), roleName.size());

    retVal = pQuery->AppendSingle(vals, valCnt, nullptr);
    if (retVal != PdbE_OK)
      break;

    if (pQuery->GetIsFullFlag())
      break;
  }

  return retVal;
}

PdbErr_t UserConfig::ChangePwd(const std::string& userName, uint32_t pwd)
{
  uint64_t userNameCrc = StringTool::CRC64NoCase(userName.c_str());
  std::unique_lock<std::mutex> userLock(this->userMutex_);

  auto userIt = userMap_.find(userNameCrc);
  if (userIt != userMap_.end())
  {
    userIt->second->SetPwd(pwd);
    _SaveUser();
    return PdbE_OK;
  }

  return PdbE_USER_NOT_FOUND;
}

PdbErr_t UserConfig::ChangeRole(const char* pUserName, const char* pRoleName)
{
  if (StringTool::ComparyNoCase(pUserName, "sa"))
  {
    LOG_INFO("denied change user(sa) role");
    return PdbE_OPERATION_DENIED;
  }

  uint64_t userNameCrc = StringTool::CRC64NoCase(pUserName);
  int roleVal = 0;
  if (StringTool::ComparyNoCase(pRoleName, PDB_USER_ROLE_READONLY_STR))
    roleVal = PDB_ROLE::ROLE_READ_ONLY;
  else if (StringTool::ComparyNoCase(pRoleName, PDB_USER_ROLE_WRITEONLY_STR))
    roleVal = PDB_ROLE::ROLE_WRITE_ONLY;
  else if (StringTool::ComparyNoCase(pRoleName, PDB_USER_ROLE_READWRITE_STR))
    roleVal = PDB_ROLE::ROLE_READ_WRITE;
  else if (StringTool::ComparyNoCase(pRoleName, PDB_USER_ROLE_ADMIN_STR))
    roleVal = PDB_ROLE::ROLE_ADMIN;
  else
    return PdbE_INVALID_PARAM;

  std::unique_lock<std::mutex> userLock(userMutex_);
  auto userIt = userMap_.find(userNameCrc);
  if (userIt != userMap_.end())
  {
    userIt->second->SetRole(roleVal);
    _SaveUser();
    return PdbE_OK;
  }

  return PdbE_USER_NOT_FOUND;
}

PdbErr_t UserConfig::DropUser(const std::string& userName)
{
  if (StringTool::ComparyNoCase(userName.c_str(), "sa"))
  {
    return PdbE_OPERATION_DENIED;
  }

  uint64_t userNameCrc = StringTool::CRC64NoCase(userName.c_str());
  std::unique_lock<std::mutex> userLock(this->userMutex_);
  auto userIt = userMap_.find(userNameCrc);
  if (userIt != userMap_.end())
  {
    delete userIt->second;
    userMap_.erase(userIt);
    _SaveUser();
    return PdbE_OK;
  }
  return PdbE_USER_NOT_FOUND;
}

bool UserConfig::_SaveUser()
{
  try
  {
    boost::property_tree::ptree pt;
    boost::property_tree::ptree userpt;

    for (auto userIt = userMap_.begin(); userIt != userMap_.end(); userIt++)
    {
      boost::property_tree::ptree userInfo;
      userInfo.put(SYSCOL_SYSUSER_USERNAME, userIt->second->GetName());
      userInfo.put(SYSCOL_SYSUSER_PASSWORD, userIt->second->GetPwd());
      userInfo.put(SYSCOL_SYSUSER_ROLE, userIt->second->GetRoleName());

      userpt.push_back(std::make_pair("", userInfo));
    }

    pt.put_child("users", userpt);

    boost::property_tree::write_json(cfgPath_, pt);
  }
  catch (boost::property_tree::ptree_error pt_error)
  {
    return false;
  }
  return true;
}


