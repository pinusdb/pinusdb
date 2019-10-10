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

#include "server/server_connection.h"
#include "table/mutex_manager.h"
#include "server/user_config.h"
#include "server/sys_config.h"
#include "server/table_config.h"
#include "db/page_pool.h"
#include "db/table_set.h"
#include "commitlog/commit_log_list.h"

extern ServerConnection* pGlbServerConnction;
extern MutexManager* pGlbMutexManager;
extern UserConfig* pGlbUser;
extern SysConfig* pGlbSysCfg;
extern TableConfig* pGlbTabCfg;
extern PagePool* pGlbPagePool;
extern TableSet* pGlbTableSet;
extern CommitLogList* pGlbCommitLog;
extern bool glbCancelCompTask;
extern bool glbRunning;
