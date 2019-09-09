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

#include "internal.h"

#define METHOD_ERROR_REP               0x0         //错误 回复报文
                                       
#define METHOD_CMD_LOGIN_REQ           0x010001    //登录 请求报文
#define METHOD_CMD_LOGIN_REP           0x010002    //登录 回复报文
                                       
#define METHOD_CMD_QUERY_REQ           0x010003    //查询 请求报文（一条执行语句）
#define METHOD_CMD_QUERY_REP           0x010004    //查询 回复报文（正常回复执行结果表，出错返回错误码）
                                       
#define METHOD_CMD_INSERT_REQ          0x010005    //插入 请求报文（一条或多条执行语句）
#define METHOD_CMD_INSERT_REP          0x010006    //插入 回复报文（出错返回错误码）
                                       
#define METHOD_CMD_NONQUERY_REQ        0x010007    //执行其他功能 请求报文（一条执行语句）
#define METHOD_CMD_NONQUERY_REP        0x010008    //执行其他功能 回复报文（回复执行结果）

#define METHOD_CMD_INSERT_TABLE_REQ   0x010009    //插入 请求报文(二进制格式)
#define METHOD_CMD_INSERT_TABLE_REP   0x01000A    //插入 回复报文(二进制格式)

/*
** |------4-----|------4-----|------4-----|------4-----|
** |   Version  |   Method   |  BodyLen   | ReturnVal  |
** |---------------------------------------------------|
** |  fieldCnt  |    recCnt  |  errPos    |            |
** |---------------------------------------------------|
** |            |            |            |            |
** |---------------------------------------------------|
** |            |            |  BodyCrc   |  HeadCrc   |
** |---------------------------------------------------|
**/
class ProtoHeader
{
public:
  ProtoHeader();
  ~ProtoHeader();

  void Load(uint8_t* pHead);

  uint32_t GetVersion();
  uint32_t GetMethod();
  uint32_t GetBodyLen();
  uint32_t GetReturnVal();
  uint32_t GetFieldCnt();
  uint32_t GetRecordCnt();
  uint32_t GetErrPos();
  uint32_t GetBodyCrc();
  uint32_t GetHeadCrc();

  void SetVersion(uint32_t version);
  void SetMethod(uint32_t methodId);
  void SetBodyLen(uint32_t bodyLen);
  void SetReturnVal(uint32_t retVal);
  void SetRecordCnt(uint32_t recordCnt);
  void SetFieldCnt(uint32_t fieldCnt);
  void SetErrPos(uint32_t errPos);
  void SetBodyCrc(uint32_t bodyCrc);

  void UpdateHeadCrc();

  void InitHeader(int32_t methodId, int32_t bodyLen, int32_t retVal, int32_t bodyCrc);

  enum {
    kProtoHeadCalcCrcLen = 60,    //CRC覆盖60字节
    kProtoHeadLength = 64,        //报文头64字节
    kProtoVersion = 0x00010000,   //协议版本号
  };

private:
  uint8_t* pHead_;
};
