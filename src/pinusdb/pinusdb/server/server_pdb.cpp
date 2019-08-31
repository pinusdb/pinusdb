#include "internal.h"
#include "server/server_pdb.h"
#include "server/event_handle.h"
#include "util/log_util.h"
#include "expr/tokenize.h"
#include "db/page_pool.h"
#include "db/db_impl.h"
#include "util/date_time.h"
#include "boost/filesystem.hpp"
#include "global_variable.h"

namespace bfs = boost::filesystem;

ServerPDB::ServerPDB()
{
  this->isInit_ = false;
}

ServerPDB::~ServerPDB()
{

}

bool ServerPDB::Start()
{
  try {
    if (isInit_)
      return true;

    //³õÊ¼»¯SQLÓï¾ä·Ö´Ê
    Tokenize::InitTokenize();
    DateTime::InitTimeZone();

    if (!pGlbSysCfg->LoadConfig())
      return false;

    if (!InitLog())
      return false;

    if (!pGlbPagePool->InitPool())
      return false;

    if (!InitUser())
      return false;

    if (!InitTable())
      return false;

    if (!InitCommitLog())
      return false;

    PdbErr_t retVal = PdbE_OK;

    DBImpl* pDB = DBImpl::GetInstance();
    retVal = pDB->Start();
    if (retVal != PdbE_OK)
    {
      LOG_ERROR("start database instance error {}", retVal);
      return false;
    }

    if (!StartIocp(pGlbSysCfg->GetAddress().c_str(), pGlbSysCfg->GetPort()))
    {
      LOG_ERROR("failed to start IOCP");
      return false;
    }

    isInit_ = true;
  }
  catch (std::exception ex)
  {
    return false;
  }

  return true;
}

void ServerPDB::Stop()
{
  LOG_INFO("stoping database service ...");

  DBImpl* pDB = DBImpl::GetInstance();

  pDB->StopTask();

  StopIocp();

  LOG_INFO("beging flush page cache to disk ...");
  pDB->Stop();
  LOG_INFO("flush page cache to disk finished");
  LOG_INFO("normal cessation of database service");
}

IOCPEvent* ServerPDB::NewEvent(const char* pRemoteIp, int remotePort)
{
  EventHandle* pNewEvent = new EventHandle(pRemoteIp, remotePort);
  if (pNewEvent == nullptr)
    return nullptr;

  if (!pGlbServerConnction->AddConnection((uint64_t)pNewEvent, pRemoteIp, remotePort))
  {
    delete pNewEvent;
    return nullptr;
  }

  LOG_INFO("create connection, remote:({}:{})", pRemoteIp, remotePort);
  return pNewEvent;
}

void ServerPDB::RemoveEvent(IOCPEvent* pEvent)
{
  pGlbServerConnction->DelConnection((uint64_t)pEvent);
}

bool ServerPDB::InitLog()
{
  bfs::path logPath = pGlbSysCfg->GetSysLogPath();

  if (!bfs::exists(logPath))
  {
    bfs::create_directories(logPath);
  }
  else
  {
    if (!bfs::is_directory(logPath))
      return false;
  }

  size_t logSize = PDB_MB_BYTES(5);
  int maxFile = 100;
  std::string filePath = logPath.string() + "/pdb.log";

  spd::rotating_logger_mt("pdb", filePath, logSize, maxFile);

  LOG_INFO("init system log successful");
  return true;
}

bool ServerPDB::InitUser()
{
  PdbErr_t retVal = pGlbUser->Load();
  if (retVal != PdbE_OK)
  {
    LOG_ERROR("failed to init user info, ret:{}", retVal);
    return false;
  }

  return true;
}

bool ServerPDB::InitTable()
{
  if (!pGlbTabCfg->LoadTableConfig())
  {
    LOG_ERROR("failed to load table config");
    return false;
  }

  return true;
}

bool ServerPDB::InitCommitLog()
{
  std::string commitLogPath = pGlbSysCfg->GetCommitLogPath();
  if (!pGlbCommitLog->Init(commitLogPath.c_str(), false))
  {
    LOG_ERROR("failed to load datalog");
    return false;
  }

  return true;
}