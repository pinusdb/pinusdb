cmake_minimum_required(VERSION 3.8)

include_directories(
  .
  ../pinusdb
  ../pinusdb/include
)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_definitions(-std=c++11 -Wall)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
  include_directories(${BOOST_INC_PATH})
  link_directories(${BOOST_LIB_PATH})
  add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
ELSE ()

ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

add_library(pdb_csdk SHARED pdb_api.cpp pdb_client.cpp pdb_datatable.cpp pdb_error_msg.cpp ../pinusdb/util/arena.cpp ../pinusdb/table/db_obj.cpp ../pinusdb/table/field_info.cpp ../pinusdb/server/proto_header.cpp ../pinusdb/util/string_tool.cpp ../pinusdb/table/table_info.cpp)