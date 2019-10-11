cmake_minimum_required(VERSION 3.8)

include_directories(
  .
  ./include
  ../deps
  ../deps/zlib
)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_definitions(-std=c++11 -Wall)
  set(CMAKE_BUILD_TYPE "Debug")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	include_directories(${BOOST_INC_PATH})
	link_directories(${BOOST_LIB_PATH} ${ZLIB_LIB_PATH})
	add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
ELSE ()

ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

file(GLOB SRC_LIST "commitlog/*.cpp" "db/*.cpp" "expr/*.cpp" "port/*.cpp" "query/*.cpp" "server/*.cpp" "storage/*.cpp" "table/*.cpp" "util/*.cpp")

#add_executable(pinusdb ${SRC_LIST})

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_executable(pinusdbd ${SRC_LIST})
  target_link_libraries(pinusdbd -lpthread -lz libboost_system.a libboost_filesystem.a)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
  add_executable(pinusdb ${SRC_LIST})
  target_link_libraries(pinusdb ws2_32.lib wsock32.lib dbghelp.lib zlib.lib)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
