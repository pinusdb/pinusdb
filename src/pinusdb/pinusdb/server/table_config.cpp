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

#include "server/table_config.h"
#include "boost/filesystem.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/foreach.hpp"
#include "util/date_time.h"
#include "util/string_tool.h"
#include "util/log_util.h"
#include "global_variable.h"
#include "sys_table_schema.h"

const char* sysTabNames[] = {
  SYSTAB_SYSCFG_NAME,
  SYSTAB_SYSUSER_NAME,
  SYSTAB_SYSTABLE_NAME,
  SYSTAB_SYSCOLUMN_NAME,
  SYSTAB_SYSDEV_NAME,
  SYSTAB_SYSDATAFILE_NAME,
  SYSTAB_CONNECTION_NAME,
  nullptr
};

////////////////////////////////////////////////////////////////

bool DataPartComp(const PartItem& partA, const PartItem& partB)
{
  return partA.GetPartDate() < partB.GetPartDate();
}

PartItem::PartItem(const char* partDateStr, int partDate, int partType)
{
  partDateStr_ = partDateStr;
  partDate_ = partDate;
  partType_ = partType;
}

PartItem::PartItem(const PartItem& part)
{
  partDateStr_ = part.partDateStr_;
  partDate_ = part.partDate_;
  partType_ = part.partType_;
}

PartItem& PartItem::operator=(const PartItem& part)
{
  partDateStr_ = part.partDateStr_;
  partDate_ = part.partDate_;
  partType_ = part.partType_;

  return *this;
}

//////////////////////////////////////////////////////

TableItem::TableItem(const char* pTabName)
{
  tabName_ = pTabName;
}

TableItem::~TableItem() { }

void TableItem::AddPartItem(const PartItem& partItem)
{
  partVec_.push_back(partItem);
  std::sort(partVec_.begin(), partVec_.end(), DataPartComp);
}

void TableItem::DelPartItem(int partDate)
{
  for (auto partIt = partVec_.begin(); partIt != partVec_.end(); partIt++)
  {
    if (partIt->GetPartDate() == partDate)
    {
      partVec_.erase(partIt);
      return;
    }
  }
}

////////////////////////////////////////////////////

TableConfig::TableConfig()
{
}

TableConfig::~TableConfig()
{
  for (auto tabIt = tabVec_.begin(); tabIt != tabVec_.end(); tabIt++)
  {
    delete *tabIt;
  }
  tabVec_.clear();
}

bool TableConfig::LoadTableConfig()
{
  std::string cfgPath = pGlbSysCfg->GetTablePath() + "/table.json";
  if (!FileTool::FileExists(cfgPath.c_str()))
  {
    _SaveConfig();
  }

  try {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(cfgPath.c_str(), pt);
    if (pt.count("tables") > 0)
    {
      boost::property_tree::ptree items;
      items = pt.get_child("tables");
    
      for (boost::property_tree::ptree::iterator it = items.begin();
        it != items.end(); it++)
      {
        std::string name = it->second.get<std::string>(SYSCOL_SYSTABLE_TABNAME);
        boost::property_tree::ptree dataParts = it->second.get_child(SYSCOL_SYSTABLE_PARTS);

        TableItem* pTabItem = new TableItem(name.c_str());
        if (pTabItem == nullptr)
          return false;

        for (boost::property_tree::ptree::iterator partIt = dataParts.begin();
          partIt != dataParts.end(); partIt++)
        {
          std::string partDateStr = partIt->second.get<std::string>(SYSCOL_SYSDATAFILE_FILEDATE);
          std::string partTypeStr = partIt->second.get<std::string>(SYSCOL_SYSDATAFILE_PARTTYPE);
          int partType = 0;
          int partDate = 0;
          if (StringTool::ComparyNoCase(partTypeStr.c_str(), PDB_PART_TYPE_NORMAL_STR))
          {
            partType = PDB_PART_TYPE_NORMAL_VAL;
          }
          else if (StringTool::ComparyNoCase(partTypeStr.c_str(), PDB_PART_TYPE_COMPRESS_STR))
          {
            partType = PDB_PART_TYPE_COMPRESS_VAL;
          }
          else
          {
            delete pTabItem;
            return false;
          }

          if (!DateTime::ParseDate(partDateStr.c_str(), partDateStr.size(), &partDate))
          {
            return false;
          }
          
          PartItem partItem(partDateStr.c_str(), partDate, partType);
          pTabItem->AddPartItem(partItem);
        }

        tabVec_.push_back(pTabItem);
      }
    }
  }
  catch (boost::property_tree::ptree_error pt_error)
  {
    LOG_ERROR("failed to load table config , {}", pt_error.what());
    return false;
  }

  return true;
}

PdbErr_t TableConfig::AddTable(const char* pTabName)
{
  std::unique_lock<std::mutex> tabLock(tabMutex_);
  TableItem* pTabItem = new TableItem(pTabName);
  if (pTabItem == nullptr)
    return PdbE_NOMEM;

  tabVec_.push_back(pTabItem);
  _SaveConfig();
  return PdbE_OK;
}

PdbErr_t TableConfig::DropTable(const char* pTabName)
{
  std::unique_lock<std::mutex> tabLock(tabMutex_);

  for (auto tabIt = tabVec_.begin(); tabIt != tabVec_.end(); tabIt++)
  {
    if (StringTool::ComparyNoCase((*tabIt)->GetTabName().c_str(), pTabName))
    {
      TableItem* pItem = *tabIt;
      tabVec_.erase(tabIt);
      delete pItem;
      _SaveConfig();
      return PdbE_OK;
    }
  }

  return PdbE_TABLE_NOT_FOUND;
}

PdbErr_t TableConfig::AddFilePart(const char* pTabName, 
  const char* pPartDateStr, int partDate, int partType)
{
  PartItem partItem(pPartDateStr, partDate, partType);
  std::unique_lock<std::mutex> tabLock(tabMutex_);

  for (auto tabIt = tabVec_.begin(); tabIt != tabVec_.end(); tabIt++)
  {
    if (StringTool::ComparyNoCase((*tabIt)->GetTabName().c_str(), pTabName))
    {
      (*tabIt)->AddPartItem(partItem);
      _SaveConfig();
      return PdbE_OK;
    }
  }

  return PdbE_TABLE_NOT_FOUND;
}

PdbErr_t TableConfig::DelFilePart(const char* pTabName, int partDate)
{
  std::unique_lock<std::mutex> tabLock(tabMutex_);
  for (auto tabIt = tabVec_.begin(); tabIt != tabVec_.end(); tabIt++)
  {
    if (StringTool::ComparyNoCase((*tabIt)->GetTabName().c_str(), pTabName))
    {
      (*tabIt)->DelPartItem(partDate);
      _SaveConfig();
      return PdbE_OK;
    }
  }

  return PdbE_OK;
}

PdbErr_t TableConfig::SetFilePart(const char* pTabName,
  const char* pPartDateStr, int partDate, int partType)
{
  PartItem partItem(pPartDateStr, partDate, partType);
  std::unique_lock<std::mutex> tabLock(tabMutex_);

  for (auto tabIt = tabVec_.begin(); tabIt != tabVec_.end(); tabIt++)
  {
    if (StringTool::ComparyNoCase((*tabIt)->GetTabName().c_str(), pTabName))
    {
      (*tabIt)->DelPartItem(partDate);
      (*tabIt)->AddPartItem(partItem);
      _SaveConfig();
      return PdbE_OK;
    }
  }

  return PdbE_TABLE_NOT_FOUND;
}

const std::vector<TableItem*>& TableConfig::GetTables()
{
  return tabVec_;
}

PdbErr_t TableConfig::QueryTable(IResultFilter* pFilter)
{
  PdbErr_t retVal = PdbE_OK;
  const int valCnt = 1;
  DBVal vals[valCnt];

  size_t sysTabIdx = 0;
  while (sysTabNames[sysTabIdx] != nullptr)
  {
    DBVAL_ELE_SET_STRING(vals, 0, sysTabNames[sysTabIdx], strlen(sysTabNames[sysTabIdx]));

    retVal = pFilter->AppendData(vals, valCnt, nullptr);
    if (retVal != PdbE_OK)
      return retVal;

    if (pFilter->GetIsFullFlag())
      return PdbE_OK;

    sysTabIdx++;
  }

  std::unique_lock<std::mutex> tabLock(tabMutex_);
  for (auto tabIt = tabVec_.begin(); tabIt != tabVec_.end(); tabIt++)
  {
    std::string tabName = (*tabIt)->GetTabName();

    DBVAL_ELE_SET_STRING(vals, 0, tabName.c_str(), tabName.size());

    retVal = pFilter->AppendData(vals, valCnt, nullptr);
    if (retVal != PdbE_OK)
      return retVal;

    if (pFilter->GetIsFullFlag())
      return PdbE_OK;
  }

  return retVal;
}

PdbErr_t TableConfig::QueryPart(IResultFilter* pFilter)
{
  PdbErr_t retVal = PdbE_OK;
  const int valCnt = 3;
  DBVal vals[valCnt];

  std::unique_lock<std::mutex> tabLock(tabMutex_);
  for (auto tabIt = tabVec_.begin(); tabIt != tabVec_.end(); tabIt++)
  {
    std::string tabName = (*tabIt)->GetTabName();

    const std::vector<PartItem>& partVec = (*tabIt)->GetPartVec();
    for (auto partIt = partVec.begin(); partIt != partVec.end(); partIt++)
    {
      std::string partDateStr = partIt->GetPartDateStr();
      int partType = partIt->GetPartType();

      DBVAL_ELE_SET_STRING(vals, 0, tabName.c_str(), tabName.size());
      DBVAL_ELE_SET_STRING(vals, 1, partDateStr.c_str(), partDateStr.size());
      if (partType == PDB_PART_TYPE_NORMAL_VAL)
      {
        DBVAL_ELE_SET_STRING(vals, 2, PDB_PART_TYPE_NORMAL_STR, strlen(PDB_PART_TYPE_NORMAL_STR));
      }
      else if (partType == PDB_PART_TYPE_COMPRESS_VAL)
      {
        DBVAL_ELE_SET_STRING(vals, 2, PDB_PART_TYPE_COMPRESS_STR, strlen(PDB_PART_TYPE_COMPRESS_STR));
      }
      else
      {
        DBVAL_ELE_SET_STRING(vals, 2, nullptr, 0);
      }

      retVal = pFilter->AppendData(vals, valCnt, nullptr);
      if (retVal != PdbE_OK)
        return retVal;

      if (pFilter->GetIsFullFlag())
        return PdbE_OK;
    }
  }

  return retVal;
}

bool TableConfig::_SaveConfig()
{
  std::string cfgPath = pGlbSysCfg->GetTablePath() + "/table.json";

  try {
    boost::property_tree::ptree pt;
    boost::property_tree::ptree tablept;

    for (auto tabIt = tabVec_.begin(); tabIt != tabVec_.end(); tabIt++)
    {
      std::string tabName = (*tabIt)->GetTabName();

      boost::property_tree::ptree tabItem;
      tabItem.put(SYSCOL_SYSTABLE_TABNAME, tabName);

      boost::property_tree::ptree tabParts;
      const std::vector<PartItem>& partVec = (*tabIt)->GetPartVec();
      for (auto partIt = partVec.begin(); partIt != partVec.end(); partIt++)
      {
        boost::property_tree::ptree partItem;
        partItem.put(SYSCOL_SYSDATAFILE_FILEDATE, partIt->GetPartDateStr());
        if (PDB_PART_TYPE_NORMAL_VAL == partIt->GetPartType())
          partItem.put(SYSCOL_SYSDATAFILE_PARTTYPE, PDB_PART_TYPE_NORMAL_STR);
        else if (PDB_PART_TYPE_COMPRESS_VAL == partIt->GetPartType())
          partItem.put(SYSCOL_SYSDATAFILE_PARTTYPE, PDB_PART_TYPE_COMPRESS_STR);

        tabParts.push_back(std::make_pair("", partItem));
      }

      tabItem.put_child(SYSCOL_SYSTABLE_PARTS, tabParts);
      tablept.push_back(std::make_pair("", tabItem));
    }

    pt.put_child("tables", tablept);
    boost::property_tree::write_json(cfgPath, pt);
  }
  catch (boost::property_tree::ptree_error pt_error)
  {
    return false;
  }

  return true;
}

