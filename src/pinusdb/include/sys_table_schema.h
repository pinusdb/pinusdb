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

#define SYSTAB_SYSCOLUMN_NAME          "sys_column"

#define SYSCOL_SYSCOLUMN_TABNAME       "tabname"
#define SYSCOL_SYSCOLUMN_COLUMNNAME    "colname"
#define SYSCOL_SYSCOLUMN_DATATYPE      "datatype"
#define SYSCOL_SYSCOLUMN_ISKEY         "iskey"

///////////////////////////////////////////////////////////////////
#define SYSTAB_CONNECTION_NAME         "sys_connection"

#define SYSCOL_CONNECTION_HOSTNAME     "host"
#define SYSCOL_CONNECTION_PORTNAME     "port"
#define SYSCOL_CONNECTION_USERNAME     "user"
#define SYSCOL_CONNECTION_ROLENAME     "userrole"
#define SYSCOL_CONNECTION_CONNTIMENAME "conntime"

///////////////////////////////////////////////////////////////////
#define SYSTAB_SYSDEV_NAME             "sys_dev"

#define SYSCOL_SYSDEV_TABNAME          "tabname"
#define SYSCOL_SYSDEV_DEVID            "devid"
#define SYSCOL_SYSDEV_DEVNAME          "devname"
#define SYSCOL_SYSDEV_EXPAND           "expand"

//////////////////////////////////////////////////////////////////
#define SYSTAB_SYSCFG_NAME       "sys_config"

#define SYSCOL_SYSCFG_NAME       "name"
#define SYSCOL_SYSCFG_VALUE      "value"

//////////////////////////////////////////////////////////////////
#define SYSTAB_SYSTABLE_NAME           "sys_table"

#define SYSCOL_SYSTABLE_TABNAME        "tabname"
#define SYSCOL_SYSTABLE_PARTS          "parts"

/////////////////////////////////////////////////////////////////
#define SYSTAB_SYSDATAFILE_NAME            "sys_datafile"

#define SYSCOL_SYSDATAFILE_TABNAME         "tabname"
#define SYSCOL_SYSDATAFILE_FILEDATE        "filedate"
#define SYSCOL_SYSDATAFILE_PARTTYPE        "filetype"

/////////////////////////////////////////////////////////////////
#define SYSTAB_SYSUSER_NAME            "sys_user"

#define SYSCOL_SYSUSER_USERNAME        "username"
#define SYSCOL_SYSUSER_PASSWORD        "pwd"
#define SYSCOL_SYSUSER_ROLE            "userrole"

