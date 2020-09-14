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

#ifndef _WIN32
#include "internal.h"
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <mutex>
#include <list>
#include <thread>
#include <chrono>
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

static void SignalHandler(int sig)
{
  (void)sig;
  glbRunning = false;
  glbCancelCompTask = true;
}

static int LockPid(int pidFd)
{
  struct flock fl;
  fl.l_type = F_WRLCK;
  fl.l_start = 0;
  fl.l_whence = SEEK_SET;
  fl.l_len = 0;
  if (fcntl(pidFd, F_SETLK, &fl) < 0)
  {
    return -1;
  }

  return 0;
}

static int SavePid(int pidFd)
{
  char buf[32];

  sprintf(buf, "%ld", static_cast<long>(getpid()));
  ftruncate64(pidFd, 0);
  lseek(pidFd, 0, SEEK_SET);
  write(pidFd, buf, strlen(buf));

  return 0;
}

static int StopByPidFd(int pidFd)
{
  char buf[32];
  memset(buf, 0, sizeof(buf));
  if (read(pidFd, buf, (sizeof(buf) - 1)) > 0)
  {
    int64_t pid = 0;
    if (StringTool::StrToInt64(buf, strlen(buf), &pid))
    {
      //fprintf(stdout, "will be kill %ld\n", pid);
      if (kill(static_cast<pid_t>(pid), SIGTERM) == 0)
      {
        //fprintf(stderr, "kill success\n");
        return 0;
      }
    }
  }

  return -1;
}

static int RunWithDaemon()
{
  if (sigignore(SIGHUP) == -1) {
    perror("Failed to ignore SIGHUP");
  }

  switch (fork()) {
  case -1:
    return (-1);
  case 0:
    break;
  default:
    _exit(EXIT_SUCCESS);
  }

  if (setsid() == -1)
    return -1;

  int fd = 0;
  if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
    if (dup2(fd, STDIN_FILENO) < 0) {
      perror("dup2 stdin");
      return (-1);
    }

    if (dup2(fd, STDOUT_FILENO) < 0) {
      perror("dup2 stdout");
      return (-1);
    }

    if (dup2(fd, STDERR_FILENO) < 0) {
      perror("dup2 stderr");
      return (-1);
    }

    if (fd > STDERR_FILENO) {
      if (close(fd) < 0) {
        perror("close");
        return (-1);
      }
    }
  }

  return 0;
}

int main(int argc, char* argv[])
{
  const char* pidfile = "/var/run/pinusdb.pid";
  int pidfd = -1;

  bool start = true;

  if (argc > 2)
  {
    fprintf(stderr, "parameter error\n");
    exit(1);
  }

  if (argc == 2)
  {
    if (StringTool::ComparyNoCase(argv[1], "start"))
      start = true;
    else if (StringTool::ComparyNoCase(argv[1], "stop"))
      start = false;
    else {
      fprintf(stderr, "parameter error\n");
      exit(1);
    }
  }

  pidfd = open(pidfile, (O_RDWR | O_CREAT), (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
  if (pidfd < 0) {
    fprintf(stderr, "open pid file failed, errno:%d\n", errno);
    return -1;
  }

  if (!start)
  {
    //stop...
    if (LockPid(pidfd) < 0)
    {
      //stop it
      StopByPidFd(pidfd);
    }
    else {
      fprintf(stdout, "lock pid success\n");
    }
    return 0;
  }
  else
  {
    if (RunWithDaemon() == -1)
    {
      fprintf(stderr, "failed to deamon\n");
      exit(EXIT_FAILURE);
    }

    if (LockPid(pidfd) < 0)
    {
      exit(EXIT_FAILURE);
    }

    if (SavePid(pidfd) < 0)
    {
      exit(EXIT_FAILURE);
    }

    //init global variables
    pGlbServerConnction = new ServerConnection();
    pGlbMutexManager = new MutexManager();
    pServer = new ServerPDB();
    pGlbUser = new UserConfig();
    pGlbSysCfg = new SysConfig();
    pGlbTabCfg = new TableConfig();
    pGlbPagePool = new PagePool();
    pGlbTableSet = new TableSet();
    pGlbCommitLog = new CommitLogList();

    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTERM, SignalHandler);

    pServer->Start();
    pServer->Stop();

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

  close(pidfd);
  exit(EXIT_SUCCESS);
}

#endif