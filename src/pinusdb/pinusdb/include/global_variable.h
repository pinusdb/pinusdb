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
