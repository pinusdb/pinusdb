cmake_minimum_required(VERSION 3.8)

include_directories(../pinusdb/include ../pdb_csdk)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_definitions(-std=c++11 -Wall)
  add_executable(pdbcli main.cpp)
  target_link_libraries(pdbcli -lpthread -lreadline -lpdb_csdk)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
  link_directories(${PDBCSDK_LIB_PATH})
  add_executable(pdbcli main.cpp)
  target_link_libraries(pdbcli pdb_csdk)
ELSE ()

ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

