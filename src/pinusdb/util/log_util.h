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

#include <stdio.h>
#include "spdlog/spdlog.h"

namespace spd = spdlog;

#define LOG_LEVEL_MIN_VAL        1

#define LOG_LEVEL_DEBUG          1
#define LOG_LEVEL_INFO           2
#define LOG_LEVEL_WARNING        3
#define LOG_LEVEL_ERROR          4

#define LOG_LEVEL_MAX_VAL        4


#define LOG_LEVEL_DEBUG_STR      "debug"
#define LOG_LEVEL_INFO_STR       "info"
#define LOG_LEVEL_WARNING_STR    "warning"
#define LOG_LEVEL_ERROR_STR      "error"

#ifdef NO_LOG

#define LOG_ERROR(FORMAT, ...) do { } while(0)
#define LOG_WARNING(FORMAT, ...) do { } while(0)
#define LOG_INFO(FORMAT, ...) do { } while(0)
#define LOG_DEBUG(FORMAT, ...) do { } while(0)

#else


#define LOG_ERROR(FORMAT, ...)   do { spd::get("pdb")->error(FORMAT, ## __VA_ARGS__); } while(0)
#define LOG_WARNING(FORMAT, ...) do { spd::get("pdb")->warn(FORMAT, ## __VA_ARGS__); } while(0)
#define LOG_INFO(FORMAT, ...)    do { spd::get("pdb")->info(FORMAT, ## __VA_ARGS__); } while(0)
#define LOG_DEBUG(FORMAT, ...)   do { spd::get("pdb")->debug(FORMAT, ## __VA_ARGS__); } while(0)

#endif

#define CHINESE_LOG
#ifdef CHINESE_LOG

//port/env_windows.cpp
#define LOGFMT_READ_NORMAL_FILE_SETFILEPOINTEREX_FAILED_3PARAM  \
"读普通文件({})设置文件指针失败,偏移({}),错误码({})"

#define LOGFMT_READ_NORMAL_FILE_FAILED_4PARAM  \
"读普通文件({})失败,偏移({}),字节数({}),错误码({})"

#define LOGFMT_READ_NORMAL_FILE_DATA_LESS_4PARAM  \
"读普通文件({})失败,偏移({}),期望字节({}),实际字节({})"

#else


//port/env_windows.cpp
#define LOGFMT_READ_NORMAL_FILE_SETFILEPOINTEREX_FAILED_3PARAM  \
"read normal file ({}) SetFilePointerEx failed, offset({}), err({})"


#define LOGFMT_READ_NORMAL_FILE_FAILED_4PARAM  \
"read normal file ({}) failed, offset({}),bytes({}),err({})"

#define LOGFMT_READ_NORMAL_FILE_DATA_LESS_4PARAM  \
"read normal file ({}) failed, offset({}),need bytes({}), read bytes({})"

#endif


