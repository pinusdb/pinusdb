#pragma once

#include <string>
#include <unordered_map>
#include "pdb.h"
#include "pdb_error.h"

class PdbErrorMsg
{
public:
  static PdbErrorMsg* GetInstance();

  ~PdbErrorMsg();

  const char* GetMsgInfo(PdbErr_t errCode);

private:

  PdbErrorMsg();

private:
  
  static PdbErrorMsg* instance_;

  std::string unknownMsg_;
  std::unordered_map<PdbErr_t, std::string> errMsgMap_;

};
