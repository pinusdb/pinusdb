#pragma once

#include "internal.h"
#include "util/ker_list.h"
#include "query/result_filter.h"

typedef struct _ConnInfo
{
  int role_;
  int remotePort_;
  int64_t connTime_;
  char remoteHost_[IP_ADDR_STR_LEN];
  char loginName_[PDB_USER_NAME_LEN];
}ConnInfo;

class QueryParam;
class DataTable;

class ServerConnection
{
public:
  ServerConnection();
  ~ServerConnection();

  bool AddConnection(uint64_t connKey, const char* pRemoteHost, int remotePort);
  void ConnectionLogin(uint64_t connKey, const char* pLoginName, int role);

  void DelConnection(uint64_t connKey);

  PdbErr_t QueryConn(IResultFilter* pFilter);

private:
  enum {
    kConnCntPerBlk = 64,  //一个内存块中ConnInfo对象的个数, 靠近 8K
    kMaxBlkCnt = 16,     //内存块的最大个数，也就是最多能存储 256 * 16 = 4096 个连接对象
  };

  ConnInfo* MallocConnInfo();

private:
  std::mutex connMutex_;
  std::unordered_map<uint64_t, ConnInfo*> connMap_;

  struct list_head freeConnList_;

  int blkCnt_;                //内存块数量
  void* ppBlk_[kMaxBlkCnt];   //内存块数组

};


