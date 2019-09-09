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

