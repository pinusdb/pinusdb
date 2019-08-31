#pragma once

#include "server/server_iocp.h"

class ServerPDB : public ServerIOCP
{
public:
  ServerPDB();
  virtual ~ServerPDB();

  virtual bool Start();
  virtual void Stop();

protected:
  virtual IOCPEvent* NewEvent(const char* pRemoteIp, int remotePort);
  virtual void RemoveEvent(IOCPEvent* pEvent);

  bool InitLog();
  bool InitUser();
  bool InitTable();
  bool InitCommitLog();

protected:
  bool isInit_;

};

