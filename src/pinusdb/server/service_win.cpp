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

#ifdef _WIN32

#include "internal.h"
#include <winsvc.h>
#include <string.h>
#include <stdio.h>
#include "util/dbg.h"

#include "server/server_pdb.h"
#include "server/server_connection.h"
#include "table/mutex_manager.h"
#include "server/user_config.h"
#include "server/sys_config.h"
#include "server/table_config.h"
#include "db/page_pool.h"
#include "db/table_set.h"
#include "commitlog/commit_log_list.h"

#define SERVICE_NAME                 TEXT("PinusDB")
#define SERVICE_DISPLAY_NAME         TEXT("PinusDB Service")
#define SERVICE_START_TYPE           SERVICE_DEMAND_START

#define SERVICE_ACCOUNT              TEXT("NT AUTHORITY\\LocalService")
#define SERVICE_PASSWORD             NULL

SERVICE_STATUS_HANDLE hStatusHandle;
SERVICE_STATUS serviceStatus;
HANDLE         stopEvent; //服务停止句柄

ServerPDB* pServer = nullptr;
ServerConnection* pGlbServerConnction = nullptr;
MutexManager* pGlbMutexManager = nullptr;
UserConfig* pGlbUser = nullptr;
SysConfig* pGlbSysCfg = nullptr;
TableConfig* pGlbTabCfg = nullptr;
PagePool* pGlbPagePool = nullptr;
TableSet* pGlbTableSet = nullptr;
CommitLogList* pGlbCommitLog = nullptr;
bool glbCancelCompTask = false;
bool glbRunning = true; 
bool glbRepStates = false;

void ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

DWORD WINAPI SvcCtrlHandlerEx(DWORD dwCtrl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);

void ServiceMain()
{
  //初始化全局变量
  pGlbServerConnction = new ServerConnection();
  pGlbMutexManager = new MutexManager();
  pServer = new ServerPDB();
  pGlbUser = new UserConfig();
  pGlbSysCfg = new SysConfig();
  pGlbTabCfg = new TableConfig();
  pGlbPagePool = new PagePool();
  pGlbTableSet = new TableSet();
  pGlbCommitLog = new CommitLogList();

  //init
  pServer->LoadSocketLib();
  if (!pServer->Start())
  {
    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    return;
  }

  ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

  WaitForSingleObject(stopEvent, INFINITE);
  glbRunning = false;
  glbCancelCompTask = true;
  pServer->Stop();

  //释放全局变量
  delete pGlbCommitLog;
  delete pGlbTableSet;
  delete pGlbPagePool;
  delete pGlbTabCfg;
  delete pGlbSysCfg;
  delete pGlbUser;
  delete pServer;
  delete pGlbMutexManager;
  delete pGlbServerConnction;
}

void WINAPI ServerMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
  serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  serviceStatus.dwCurrentState = SERVICE_START_PENDING;
  serviceStatus.dwControlsAccepted = 0; //执行ReportSvcStatus时设置

  //注册服务控制
  hStatusHandle = RegisterServiceCtrlHandlerEx(SERVICE_NAME, SvcCtrlHandlerEx, NULL);
  if (hStatusHandle == NULL)
  {
    return;
  }

  serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  serviceStatus.dwServiceSpecificExitCode = 0;

  ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

  stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  if (NULL == stopEvent)
  {
    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    return;
  }

  PDB_SEH_BEGIN(true);
  ServiceMain();
  PDB_SEH_END("main", return);

  ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);

  return;
}

void ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
  static DWORD dwCheckPoint = 1;

  serviceStatus.dwCurrentState = dwCurrentState;
  serviceStatus.dwWin32ExitCode = dwWin32ExitCode;
  serviceStatus.dwWaitHint = dwWaitHint;

  if (dwCurrentState == SERVICE_RUNNING)
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PRESHUTDOWN; 
  else
    serviceStatus.dwControlsAccepted = 0;

  if ((dwCurrentState == SERVICE_RUNNING)
    || (dwCurrentState == SERVICE_STOPPED))
    serviceStatus.dwCheckPoint = 0;
  else
    serviceStatus.dwCheckPoint = dwCheckPoint++;

  SetServiceStatus(hStatusHandle, &serviceStatus);
}

DWORD WINAPI SvcCtrlHandlerEx(DWORD dwCtrl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
  switch (dwCtrl)
  {
  case SERVICE_CONTROL_PRESHUTDOWN:
  case SERVICE_CONTROL_STOP:
    SetEvent(stopEvent);
    ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 10000);
    return NO_ERROR;

  case SERVICE_CONTROL_INTERROGATE:
    ReportSvcStatus(serviceStatus.dwCurrentState, NO_ERROR, 0);
    return NO_ERROR;
  }

  return NO_ERROR;
}

int main(int argc, char* argv[])
{
  SC_HANDLE schSCManager = NULL;
  SC_HANDLE schService = NULL;

  SERVICE_DESCRIPTION sd;
  sd.lpDescription = "松果时序数据库";
  TCHAR szFilePath[MAX_PATH];
  ::GetModuleFileName(NULL, szFilePath, MAX_PATH);

  if ((argc > 1) && ((*argv[1] == '-') || (*argv[1] == '/')))
  {
    if (strcmp("install", (argv[1] + 1)) == 0)
    {
      schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
      if (NULL == schSCManager)
      {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return 1;
      }

      schService = CreateService(
        schSCManager,
        SERVICE_NAME,
        SERVICE_DISPLAY_NAME,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        szFilePath,
        NULL,
        NULL,
        TEXT(""),
        NULL,
        NULL);

      if (NULL == schService)
      {
        printf("Install Service failed (%d)\n", GetLastError());
      }
      else
      {
        ::ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &sd);

        CloseServiceHandle(schService);
        printf("Install Service PinusDB successfully\n");
      }

      CloseServiceHandle(schSCManager);

    }
    else if (strcmp("remove", (argv[1] + 1)) == 0)
    {
      schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
      if (NULL == schSCManager)
      {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return 1;
      }

      schService = OpenService(schSCManager, SERVICE_NAME, SC_MANAGER_ALL_ACCESS);
      if (NULL == schService)
      {
        CloseServiceHandle(schSCManager);
        printf("OpenService failed (%d)\n", GetLastError());
        return 1;
      }

      if (!DeleteService(schService))
      {
        printf("Delete PinusDB Service failed (%d)\n", GetLastError());
      }
      else
      {
        printf("Delete PinusDB Service successfully\n");
      }

      CloseServiceHandle(schService);
      CloseServiceHandle(schSCManager);
    }
  }
  else
  {
    SERVICE_TABLE_ENTRY dispatchTable[] = {
      { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServerMain },
      { NULL, NULL }
    };
    if (!StartServiceCtrlDispatcher(dispatchTable))
    {
      //ServerReportEvent
    }
  }
  return 0;
}

#endif