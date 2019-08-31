#pragma once

#include "internal.h"

#include "table/table_info.h"
#include "expr/expr_item.h"
#include "query/result_filter.h"

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
  PdbErr_t Query(IResultFilter* pFilter);

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

