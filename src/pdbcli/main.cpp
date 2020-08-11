#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <mutex>
#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include "pdb_api.h"

void print_table(void* pTable);
PdbErr_t create_handle(const char* pHost, int port, const char* pUser, const char* pPwd, int* pHandle);

bool ChangeSaPwd(const char* pHost, int port, const char* pUser, const char* pPwd);
std::string read_command();

void print_error_msg(PdbErr_t ret);

bool ReadConnInfo(std::string& hostStr, std::string& userStr, std::string& pwdStr, int& port);

#ifdef _WIN32
std::string convert_gbk_to_utf8(const std::string& strGbk);
std::string convert_utf8_to_gbk(const std::string& strUtf8);
#endif

std::string convert_datetime_to_str(int64_t dt);

int main(int argc, char* argv[])
{
  std::string hostStr = "";
  std::string userStr = "";
  std::string pwdStr = "";
  int port = 0;

  do {
    std::cout << "PinusDB Command Client" << std::endl;
    if (!ReadConnInfo(hostStr, userStr, pwdStr, port))
    {
      std::cout << "connection failed..." << std::endl;
      std::cout << "press enter key to continue... "<< std::endl;
      fgetc(stdin);
      return 0;
    }

    if (userStr.compare("sa") == 0 && pwdStr.compare("pinusdb") == 0)
    {
      //must change default password
      std::cout << "must change password" << std::endl;
      if (!ChangeSaPwd(hostStr.c_str(), port, userStr.c_str(), pwdStr.c_str()))
      {
        std::cout << "press enter key to continue... "<< std::endl;
        fgetc(stdin);
        return 0;
      }
    }
    else
    {
      break;
    }

  } while (true);

  PdbErr_t retVal = PdbE_OK;

  while (true)
  {
    std::string cmd = read_command();

    if (cmd.compare("exit") == 0)
    {
      return 0;
    }
    else if (cmd.compare("clear") == 0)
    {
      //system("cls");
    }
    else
    {
      int32_t handle = 0;
      retVal = create_handle(hostStr.c_str(), port, userStr.c_str(), pwdStr.c_str(), &handle);
      if (retVal != PdbE_OK)
      {
        print_error_msg(retVal);
        continue;
      }

      size_t bgPos = cmd.find_first_not_of(' ');
      if (bgPos != std::string::npos)
      {
        std::string cmdType = cmd.substr(bgPos, 6);
        std::transform(cmdType.begin(), cmdType.end(), cmdType.begin(), ::tolower);

        if (cmdType.compare("insert") == 0)
        {
          retVal = pdb_execute_insert(handle, cmd.c_str(), nullptr, nullptr);
        }
        else if (cmdType.compare("select") == 0)
        {
          void* pTable = nullptr;
          retVal = pdb_execute_query(handle, cmd.c_str(), &pTable);
          if (retVal == PdbE_OK)
          {
            print_table(pTable);

            pdb_table_free(pTable);
          }
        }
        else
        {
          retVal = pdb_execute_non_query(handle, cmd.c_str());
        }

      }

      pdb_disconnect(handle);

      if (retVal != PdbE_OK)
        print_error_msg(retVal);

    }

  }

  return 0;
}

void SetStdinEcho(bool enable = true)
{
#ifdef _WIN32
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode;
  GetConsoleMode(hStdin, &mode);

  if (!enable)
    mode &= ~ENABLE_ECHO_INPUT;
  else
    mode |= ENABLE_ECHO_INPUT;

  SetConsoleMode(hStdin, mode);
#else
  struct termios mode;
  tcgetattr(STDIN_FILENO, &mode);
  if (!enable)
    mode.c_lflag &= ~(ICANON | ECHO);
  else
    mode.c_lflag |= (ICANON | ECHO);

  tcsetattr(STDIN_FILENO, TCSANOW, &mode);
#endif
}

bool ReadConnInfo(std::string& hostStr, std::string& userStr, std::string& pwdStr, int& port)
{
  void* pTable = nullptr;
  const char* pVerStr = nullptr;
  size_t verLen = 0;
  char buf[128] = { 0 };

  std::cout << "Server IP:" << std::flush;
  std::cin.getline(buf, 100);

  hostStr = buf;

  std::cout << "Server Port:" << std::flush;
  std::cin.getline(buf, 100);

  port = atoi(buf);

  std::cout << "User Name:" << std::flush;
  std::cin.getline(buf, 100);

  userStr = buf;

  std::cout << "Password:" << std::flush;
  SetStdinEcho(false);
  std::cin.getline(buf, 100);
  SetStdinEcho(true);
  std::cout << std::endl;

  pwdStr = buf;

  int handle = 0;

  PdbErr_t retVal = create_handle(hostStr.c_str(), port, userStr.c_str(), pwdStr.c_str(), &handle);
  if (retVal == PdbE_OK)
  {
    std::cout << "Welcome to PinusDB. Command end with ;" << std::endl;
    retVal = pdb_execute_query(handle, "SELECT VERSION()", &pTable);
    if (retVal == PdbE_OK)
    {
      if (pdb_table_get_string_by_colidx(pTable, 0, 0, &pVerStr, &verLen) == PdbE_OK)
      {
        std::cout << "Server version: " << std::string(pVerStr, verLen).c_str() << std::endl;
      }
      pdb_table_free(pTable);
    }

    pdb_disconnect(handle);
    return true;
  }

  return false;
}

bool ChangeSaPwd(const char* pHost, int port, const char* pUser, const char* pPwd)
{
  std::string newPwd;
  char buf[1024] = { 0 };
  do {
    std::cout << "New Password:" << std::flush;
    SetStdinEcho(false);
    std::cin.getline(buf, 100);
    SetStdinEcho(true);
    std::cout << std::endl;

    newPwd = buf;
    if (newPwd.size() < 6)
    {
      std::cout << "BAD PASSEORD: The password is shorter than 6 characters" << std::endl;
      continue;
    }

    if (newPwd.compare("pinusdb") == 0)
    {
      std::cout << "BAD PASSEORD: Don't use the default password" << std::endl;
      continue;
    }

    std::cout << "Retype new password:" << std::flush;
    SetStdinEcho(false);
    std::cin.getline(buf, 100);
    SetStdinEcho(true);
    std::cout << std::endl;

    if (newPwd.compare(buf) != 0)
    {
      std::cout << "Sorry, passwords do not match." << std::endl;
      continue;
    }

    break;
  } while (true);

  int handle = 0;
  PdbErr_t retVal = create_handle(pHost, port, pUser, pPwd, &handle);
  if (retVal != PdbE_OK)
  {
    std::cout << "Connection failed:" << retVal << std::endl;
    return false;
  }

  do {
    sprintf(buf, "SET PASSWORD FOR %s = PASSWORD('%s')", pUser, newPwd.c_str());

    retVal = pdb_execute_non_query(handle, buf);
    if (retVal == PdbE_OK)
    {
      std::cout << "Change password success" << std::endl;
    }
    else
    {
      std::cout << "Change password failed:" << retVal << std::endl;
    }
  } while (false);

  pdb_disconnect(handle);
  return retVal == PdbE_OK;
}

void print_error_msg(PdbErr_t ret)
{
  if (ret != PdbE_OK)
  {
    const char* pErrMsg = pdb_get_error_msg(ret);
    std::cout << "Error :" << ret << "," << pErrMsg << std::endl;
  }
}


#define PRINT_LINE(lineLen) do { std::cout.fill('-'); std::cout.width(lineLen); std::cout<< "-" << std::endl; std::cout.fill(' '); } while(false)
#define PRINT_VAL_LEFT(val) do { std::cout.width(11); std::cout.setf(std::ios::left); std::cout << (val) << "|"; std::cout.unsetf(std::ios::left); } while(false)
#define PRINT_VAL_RIGHT(val) do { std::cout.width(11); std::cout.setf(std::ios::right); std::cout << (val) << "|"; std::cout.unsetf(std::ios::right); } while(false)

//print table
void print_table(void* pTable)
{
  PdbErr_t retVal = PdbE_OK;

  size_t rowCnt = 0;
  size_t colCnt = 0;
  std::vector<int> colTypeVec; //column type

  //get rows count
  retVal = pdb_table_get_row_count(pTable, &rowCnt);
  if (retVal != PdbE_OK)
  {
    std::cout << "Error: Get table row count failed, " << retVal << std::endl;
    return;
  }

  //get columns count
  retVal = pdb_table_get_column_count(pTable, &colCnt);
  if (retVal != PdbE_OK)
  {
    std::cout << "Error: Get table column count failed, " << retVal << std::endl;
    return;
  }

  //get column info
  size_t lineLen = colCnt * 12;
  PRINT_LINE(lineLen);
  for (size_t i = 0; i < colCnt; i++)
  {
    ColumnInfo colInfo;
    retVal = pdb_table_get_column_info(pTable, i, &colInfo);
    if (retVal != PdbE_OK)
    {
      std::cout << "" << std::endl;
      return;
    }

    colTypeVec.push_back(colInfo.colType_);

    PRINT_VAL_LEFT(colInfo.colName_);
  }
  std::cout << std::endl;
  PRINT_LINE(lineLen);

  bool boolVal = false;
  int64_t int64Val = 0;
  double doubleVal = 0;
  const char* pStrVal = nullptr;
  const uint8_t* pBlobVal = nullptr;
  size_t len = 0;
  std::string strVal;

  bool isNull = false;

  //print data
  for (size_t rowIdx = 0; rowIdx < rowCnt; rowIdx++)
  {
    for (size_t colIdx = 0; colIdx < colCnt; colIdx++)
    {
      retVal = pdb_table_val_is_null_by_colidx(pTable, rowIdx, colIdx, &isNull);
      if (retVal != PdbE_OK)
      {
        std::cout << "ERROR: get data failed, "<< retVal << std::endl;
        return;
      }

      if (isNull)
      {
        PRINT_VAL_RIGHT("[null]");
        continue;
      }

      switch (colTypeVec[colIdx])
      {
      case PDB_FIELD_TYPE::TYPE_BOOL:
      {
        retVal = pdb_table_get_bool_by_colidx(pTable, rowIdx, colIdx, &boolVal);
        if (retVal == PdbE_OK)
          PRINT_VAL_RIGHT((boolVal ? "true" : "false"));
        break;
      }
      case PDB_FIELD_TYPE::TYPE_INT64:
      {
        retVal = pdb_table_get_bigint_by_colidx(pTable, rowIdx, colIdx, &int64Val);
        if (retVal == PdbE_OK)
          PRINT_VAL_RIGHT(int64Val);
        break;
      }
      case PDB_FIELD_TYPE::TYPE_DATETIME:
      {
        retVal = pdb_table_get_datetime_by_colidx(pTable, rowIdx, colIdx, &int64Val);
        if (retVal == PdbE_OK)
        {
          strVal = convert_datetime_to_str(int64Val).c_str();
          PRINT_VAL_RIGHT(strVal.c_str());
        }
        break;
      }
      case PDB_FIELD_TYPE::TYPE_DOUBLE:
      {
        retVal = pdb_table_get_double_by_colidx(pTable, rowIdx, colIdx, &doubleVal);
        if (retVal == PdbE_OK)
          PRINT_VAL_RIGHT(doubleVal);
        break;
      }
      case PDB_FIELD_TYPE::TYPE_STRING:
      {
        retVal = pdb_table_get_string_by_colidx(pTable, rowIdx, colIdx, &pStrVal, &len);
        if (retVal == PdbE_OK)
        {
#ifdef _WIN32
          strVal = convert_utf8_to_gbk(std::string(pStrVal, len));
#else
          strVal = std::string(pStrVal, len);
#endif
          PRINT_VAL_LEFT(strVal.c_str());
        }
        break;
      }
      case PDB_FIELD_TYPE::TYPE_BLOB:
      {
        retVal = pdb_table_get_blob_by_colidx(pTable, rowIdx, colIdx, &pBlobVal, &len);
        if (retVal == PdbE_OK)
          PRINT_VAL_LEFT("[blob]");
        break;
      }
      }

      if (retVal != PdbE_OK)
      {
        std::cout << "ERROR: get data failed, " << retVal << std::endl;
        return;
      }
    }

    std::cout << std::endl;
  }

}

#ifdef _WIN32
std::string read_command()
{
  std::string cmdStr;

  char buf[1024] = { 0 };
  std::cout << "pinusdb> " << std::flush;

  while (true)
  {
    std::cin.getline(buf, 1000);
    cmdStr = cmdStr + " " + buf;

    size_t bgpos = cmdStr.find_first_not_of(' ');
    size_t edpos = cmdStr.find_last_not_of(' ');

    if (bgpos != std::string::npos && edpos != std::string::npos)
    {
      std::string tmpStr = cmdStr.substr(bgpos, 5);
      std::transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), ::tolower);
      if (tmpStr.compare("exit") == 0 || tmpStr.compare("exit;") == 0)
        return "exit";
      if (tmpStr.compare("clear") == 0 || tmpStr.compare("clear;") == 0
        || tmpStr.compare("cls") == 0 || tmpStr.compare("cls;") == 0)
        return "clear";
    }
    else
    {
      return "";
    }

    if (edpos != std::string::npos)
    {
      if (cmdStr[edpos] == ';')
      {
        //return cmdStr;
        return convert_gbk_to_utf8(cmdStr.substr(0, edpos));
      }
    }
  }
}

#else

std::string read_command()
{
  std::string cmdStr = "";
  std::cout << "pinusdb> " << std::flush;

  while (true)
  {
    char* pStr = readline("");
    if (pStr != nullptr && strlen(pStr) > 0)
    {
      if (cmdStr.size() > 0)
        cmdStr += " ";

      cmdStr += pStr;
      free(pStr);

      size_t bgpos = cmdStr.find_first_not_of(' ');
      size_t edpos = cmdStr.find_last_not_of(' ');

      if (bgpos != std::string::npos && edpos != std::string::npos)
      {
        std::string tmpStr = cmdStr.substr(bgpos, 5);
        std::transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), ::tolower);
        if (tmpStr.compare("exit") == 0 || tmpStr.compare("exit;") == 0)
          return "exit";
        if (tmpStr.compare("clear") == 0 || tmpStr.compare("clear;") == 0
          || tmpStr.compare("cls") == 0 || tmpStr.compare("cls;") == 0)
          return "clear";
      }
      else
      {
        return "";
      }

      if (edpos != std::string::npos)
      {
        if (cmdStr[edpos] == ';')
        {
          add_history(cmdStr.c_str());
          return cmdStr;
        }
      }
    }
    else
    {
      return "";
    }
  }
}
#endif


//create handle
PdbErr_t create_handle(const char* pHost, int port, const char* pUser, const char* pPwd, int* pHandle)
{
  PdbErr_t retVal = PdbE_OK;

  retVal = pdb_connect(pHost, port, pHandle);
  if (retVal != PdbE_OK)
    return retVal;

  retVal = pdb_login(*pHandle, pUser, pPwd);
  if (retVal != PdbE_OK)
  {
    pdb_disconnect(*pHandle);
    return retVal;
  }

  return PdbE_OK;
}



#ifdef _WIN32
std::string convert_gbk_to_utf8(const std::string& strGbk)
{
  size_t len = MultiByteToWideChar(CP_ACP, 0, strGbk.c_str(), -1, NULL, 0);
  wchar_t* wstr = new wchar_t[len + 1];
  memset(wstr, 0, len + 1);
  MultiByteToWideChar(CP_ACP, 0, strGbk.c_str(), -1, wstr, len);
  len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
  char* pStr = new char[len + 1];
  memset(pStr, 0, len + 1);
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, pStr, len, NULL, NULL);
  std::string result(pStr);
  delete []pStr;
  delete []wstr;
  return result;
}

std::string convert_utf8_to_gbk(const std::string& strUtf8)
{
  size_t len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
  wchar_t* wstr = new wchar_t[len + 1];
  memset(wstr, 0, len + 1);
  MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wstr, len);
  len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
  char* pStr = new char[len + 1];
  memset(pStr, 0, len + 1);
  WideCharToMultiByte(CP_ACP, 0, wstr, -1, pStr, len, NULL, NULL);
  std::string result(pStr);
  delete []pStr;
  delete []wstr;
  return result;
}
#endif

std::string convert_datetime_to_str(int64_t dt)
{
  time_t t = (dt / 1000);
  struct tm tmptm = { 0 };
#ifdef _WIN32
  errno_t err = localtime_s(&tmptm, &t);
  if (err)
  {
    return std::string("[time error]");
  }
#else
  if (localtime_r(&t, &tmptm) == NULL)
    return "";

#endif

  char tmpBuf[64] = { 0 };

  sprintf(tmpBuf, "%d-%d-%d %d:%d:%d.%03d",
    (tmptm.tm_year + 1900), (tmptm.tm_mon + 1),
    tmptm.tm_mday, tmptm.tm_hour, tmptm.tm_min,
    tmptm.tm_sec, static_cast<int>(dt % 1000));

  return std::string(tmpBuf);
}
