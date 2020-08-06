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

#include "server/sys_config.h"
#include "boost/filesystem.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/ini_parser.hpp"
#include "boost/foreach.hpp"
#include "util/string_tool.h"
#include "util/log_util.h"
#include "port/os_file.h"
#include "global_variable.h"

#define SYSCFG_ADDRESS_NAME           "address"
#define SYSCFG_PORT_NAME              "port"
#define SYSCFG_CACHE_SIZE_NAME        "cacheSize"
#define SYSCFG_WRITE_CACHE_SIZE_NAME  "writeCache"
#define SYSCFG_QUERY_TIMEOUT_NAME     "queryTimeOut"
#define SYSCFG_INSERT_VALID_DAY_NAME  "insertValidDay"
#define SYSCFG_TABLEPATH_NAME         "tabPath"
#define SYSCFG_NORMAL_DATAPATH_NAME   "normalDataPath"
#define SYSCFG_COMPRESS_DATAPATH_NAME "compressDataPath"
#define SYSCFG_COMMITLOGPATH_NAME     "commitLogPath"
#define SYSCFG_SYSLOGPATH_NAME        "sysLogPath"
#define SYSCFG_LOG_LEVEL_NAME         "logLevel"
#define SYSCFG_COMPRESSFLAG_NAME      "compressFlag"

#define SYSCFG_MAJORVER_NAME        "majorVersion"
#define SYSCFG_MINORVER_NAME        "minorVersion"
#define SYSCFG_BUILDVER_NAME        "buildVersion"

#define SYSCFG_DEV_CNT_NAME         "devCnt"

SysConfig::SysConfig()
{
  port_ = 8105;
  logLevel_ = LOG_LEVEL_INFO;
  queryTimeOut_ = 300;
  cacheSize_ = 0;
  writeCache_ = 0;
  insertValidDay_ = 1;
  compressFlag_ = true;
}

SysConfig::~SysConfig()
{
}


bool SysConfig::LoadConfig()
{
#ifdef _WIN32
  TCHAR szFilePath[MAX_PATH];
  ::GetModuleFileName(NULL, szFilePath, MAX_PATH);

  boost::filesystem::path filePath(szFilePath);
  std::string cfgPath = filePath.parent_path().string();
  cfgPath += "/config.ini";
#else
  std::string cfgPath = "/etc/pinusdb/config.ini";
#endif

  if (!boost::filesystem::exists(cfgPath.c_str()))
    return false;

  try {
    int64_t tmpInt64 = 0;
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(cfgPath.c_str(), pt);

    for (boost::property_tree::ptree::iterator secIt = pt.begin(); secIt != pt.end(); secIt++)
    {
      std::string secName = secIt->first;
      if (StringTool::ComparyNoCase(secName.c_str(), "server"))
      {
        for (boost::property_tree::ptree::iterator it = secIt->second.begin(); it != secIt->second.end(); it++)
        {
          std::string name = it->first;
          std::string value = it->second.data();

          if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_ADDRESS_NAME))
          {
            this->address_ = value;
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_PORT_NAME))
          {
            if (!StringTool::StrToInt64(value.c_str(), value.size(), &tmpInt64))
              return false;

            this->port_ = static_cast<int>(tmpInt64);
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_QUERY_TIMEOUT_NAME))
          {
            if (!StringTool::StrToInt64(value.c_str(), value.size(), &tmpInt64))
              return false;

            this->queryTimeOut_ = static_cast<int32_t>(tmpInt64);
            if (this->queryTimeOut_ < 0)
              return false;
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_CACHE_SIZE_NAME))
          {
            if (!StringTool::StrToInt64(value.c_str(), value.size(), &tmpInt64))
              return false;

            this->cacheSize_ = static_cast<int32_t>(tmpInt64);
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_WRITE_CACHE_SIZE_NAME))
          {
            if (!StringTool::StrToInt64(value.c_str(), value.size(), &tmpInt64))
              return false;

            this->writeCache_ = static_cast<int32_t>(tmpInt64);
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_INSERT_VALID_DAY_NAME))
          {
            if (!StringTool::StrToInt64(value.c_str(), value.size(), &tmpInt64))
              return false;

            this->insertValidDay_ = static_cast<int32_t>(tmpInt64);
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_LOG_LEVEL_NAME))
          {
            if (StringTool::ComparyNoCase(value.c_str(), LOG_LEVEL_DEBUG_STR))
              this->logLevel_ = LOG_LEVEL_DEBUG;
            else if (StringTool::ComparyNoCase(value.c_str(), LOG_LEVEL_INFO_STR))
              this->logLevel_ = LOG_LEVEL_INFO;
            else if (StringTool::ComparyNoCase(value.c_str(), LOG_LEVEL_WARNING_STR))
              this->logLevel_ = LOG_LEVEL_WARNING;
            else if (StringTool::ComparyNoCase(value.c_str(), LOG_LEVEL_ERROR_STR))
              this->logLevel_ = LOG_LEVEL_ERROR;
            else
              return false;
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_SYSLOGPATH_NAME))
          {
            this->sysLogPath_ = value;
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_NORMAL_DATAPATH_NAME))
          {
            this->normalDataPath_ = value;
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_COMPRESS_DATAPATH_NAME))
          {
            this->compressDataPath_ = value;
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_COMPRESSFLAG_NAME))
          {
            this->compressFlag_ = StringTool::ComparyNoCase(value.c_str(), "true");
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_TABLEPATH_NAME))
          {
            this->tabPath_ = value;
          }
          else if (StringTool::ComparyNoCase(name.c_str(), SYSCFG_COMMITLOGPATH_NAME))
          {
            this->commitLogPath_ = value;
          }

        }
      }
      else
      {
        return false;
      }
    }
  }
  catch (std::runtime_error err)
  {
    return false;
  }

  if (sysLogPath_.size() == 0)
    return false;

  if (normalDataPath_.size() == 0)
    return false;

  if (commitLogPath_.size() == 0)
    return false;

  if (tabPath_.size() == 0)
    return false;

  if (compressDataPath_.size() == 0)
    return false;

  if (!FileTool::PathExists(sysLogPath_.c_str()))
    return false;

  if (!FileTool::PathExists(normalDataPath_.c_str()))
    return false;

  if (!FileTool::PathExists(commitLogPath_.c_str()))
    return false;

  if (!FileTool::PathExists(tabPath_.c_str()))
    return false;

  if (!FileTool::PathExists(compressDataPath_.c_str()))
    return false;

  return true;
}

#define APPEND_DATA_CFG   do {                          \
  retVal = pQuery->AppendData(vals, valCnt, nullptr);  \
  if (retVal != PdbE_OK) return retVal;                 \
  if (pQuery->GetIsFullFlag()) return PdbE_OK;         \
}while(false)

PdbErr_t SysConfig::Query(IQuery* pQuery)
{
  PdbErr_t retVal = PdbE_OK;
  const int valCnt = 2;
  DBVal vals[valCnt];
  char dataBuf[16];

  std::unique_lock<std::mutex> cfgLock(sysCfgMutex_);

  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_ADDRESS_NAME, strlen(SYSCFG_ADDRESS_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, address_.c_str(), address_.size());
  APPEND_DATA_CFG;
  //////////////////////////////////////////////////////////////////////

  sprintf(dataBuf, "%d", port_);
  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_PORT_NAME, strlen(SYSCFG_PORT_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, dataBuf, strlen(dataBuf));
  APPEND_DATA_CFG;
  //////////////////////////////////////////////////////////////////////

  sprintf(dataBuf, "%d", cacheSize_);
  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_CACHE_SIZE_NAME, strlen(SYSCFG_CACHE_SIZE_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, dataBuf, strlen(dataBuf));
  APPEND_DATA_CFG;
  //////////////////////////////////////////////////////////////////////

  sprintf(dataBuf, "%d", writeCache_);
  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_WRITE_CACHE_SIZE_NAME, strlen(SYSCFG_WRITE_CACHE_SIZE_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, dataBuf, strlen(dataBuf));
  APPEND_DATA_CFG;

  //////////////////////////////////////////////////////////////////////

  sprintf(dataBuf, "%d", queryTimeOut_);
  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_QUERY_TIMEOUT_NAME, strlen(SYSCFG_QUERY_TIMEOUT_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, dataBuf, strlen(dataBuf));
  APPEND_DATA_CFG;
  //////////////////////////////////////////////////////////////////////

  sprintf(dataBuf, "%d", insertValidDay_);
  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_INSERT_VALID_DAY_NAME, strlen(SYSCFG_INSERT_VALID_DAY_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, dataBuf, strlen(dataBuf));
  APPEND_DATA_CFG;
  //////////////////////////////////////////////////////////////////////

  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_LOG_LEVEL_NAME, strlen(SYSCFG_LOG_LEVEL_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, nullptr, 0);
  switch (logLevel_)
  {
  case LOG_LEVEL_DEBUG:
    DBVAL_ELE_SET_STRING(vals, 1, LOG_LEVEL_DEBUG_STR, strlen(LOG_LEVEL_DEBUG_STR));
    break;
  case LOG_LEVEL_INFO:
    DBVAL_ELE_SET_STRING(vals, 1, LOG_LEVEL_INFO_STR, strlen(LOG_LEVEL_INFO_STR));
    break;
  case LOG_LEVEL_WARNING:
    DBVAL_ELE_SET_STRING(vals, 1, LOG_LEVEL_WARNING_STR, strlen(LOG_LEVEL_WARNING_STR));
    break;
  case LOG_LEVEL_ERROR:
    DBVAL_ELE_SET_STRING(vals, 1, LOG_LEVEL_ERROR_STR, strlen(LOG_LEVEL_ERROR_STR));
    break;
  }
  APPEND_DATA_CFG;
  //////////////////////////////////////////////////////////////////////

  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_SYSLOGPATH_NAME, strlen(SYSCFG_SYSLOGPATH_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, sysLogPath_.c_str(), sysLogPath_.size());
  APPEND_DATA_CFG;
  //////////////////////////////////////////////////////////////////////

  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_TABLEPATH_NAME, strlen(SYSCFG_TABLEPATH_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, tabPath_.c_str(), tabPath_.size());
  APPEND_DATA_CFG;
  //////////////////////////////////////////////////////////////////////

  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_COMMITLOGPATH_NAME, strlen(SYSCFG_COMMITLOGPATH_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, commitLogPath_.c_str(), commitLogPath_.size());
  APPEND_DATA_CFG;
  //////////////////////////////////////////////////////////////////////

  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_NORMAL_DATAPATH_NAME, strlen(SYSCFG_NORMAL_DATAPATH_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, normalDataPath_.c_str(), normalDataPath_.size());
  APPEND_DATA_CFG;
  //////////////////////////////////////////////////////////////////////

  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_COMPRESS_DATAPATH_NAME, strlen(SYSCFG_COMPRESS_DATAPATH_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, compressDataPath_.c_str(), compressDataPath_.size());
  APPEND_DATA_CFG;

  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_COMPRESSFLAG_NAME, strlen(SYSCFG_COMPRESSFLAG_NAME));
  if (compressFlag_)
    DBVAL_ELE_SET_STRING(vals, 1, "true", 4);
  else
    DBVAL_ELE_SET_STRING(vals, 1, "false", 5);
  APPEND_DATA_CFG;

  sprintf(dataBuf, "%d", PDB_MAJOR_VER_VAL);
  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_MAJORVER_NAME, strlen(SYSCFG_MAJORVER_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, dataBuf, strlen(dataBuf));
  APPEND_DATA_CFG;

  sprintf(dataBuf, "%d", PDB_MINOR_VER_VAL);
  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_MINORVER_NAME, strlen(SYSCFG_MINORVER_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, dataBuf, strlen(dataBuf));
  APPEND_DATA_CFG;

  sprintf(dataBuf, "%d", PDB_BUILD_VER_VAL);
  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_BUILDVER_NAME, strlen(SYSCFG_BUILDVER_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, dataBuf, strlen(dataBuf));
  APPEND_DATA_CFG;

  sprintf(dataBuf, "%d", PDB_SYS_DEV_CNT);
  DBVAL_ELE_SET_STRING(vals, 0, SYSCFG_DEV_CNT_NAME, strlen(SYSCFG_DEV_CNT_NAME));
  DBVAL_ELE_SET_STRING(vals, 1, dataBuf, strlen(dataBuf));
  APPEND_DATA_CFG;

  return PdbE_OK;
}