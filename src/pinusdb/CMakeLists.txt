cmake_minimum_required(VERSION 3.8)

include_directories(
  .
  ./include
  ../deps
  ../deps/zlib
)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(CMAKE_CXX_STANDARD 17)
  add_definitions(-Wall -O2)
  link_directories(/usr/lib64)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
  add_compile_options("/std:c++17")
	include_directories(${BOOST_INC_PATH})
	link_directories(${BOOST_LIB_PATH} ${ZLIB_LIB_PATH})
	add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
ELSE ()

ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

file(GLOB SRC_LIST "commitlog/*.cpp" "db/*.cpp" "expr/*.cpp" "port/*.cpp" "query/*.cpp" "server/*.cpp" "storage/*.cpp" "table/*.cpp" "util/*.cpp")

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_executable(pinusdbd ${SRC_LIST})
  target_link_libraries(pinusdbd -lpthread -lz libboost_system.a libboost_filesystem.a)
  install(TARGETS pinusdbd RUNTIME DESTINATION "/var/pinusdb/bin/")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/config.ini" DESTINATION "/etc/pinusdb/")
  install(CODE "execute_process(COMMAND touch /var/run/pinusdb.pid)")
  install(CODE "execute_process(COMMAND mkdir /var/pinusdb/table)")
  install(CODE "execute_process(COMMAND mkdir /var/pinusdb/normaldata)")
  install(CODE "execute_process(COMMAND mkdir /var/pinusdb/cmpdata)")
  install(CODE "execute_process(COMMAND mkdir /var/pinusdb/datalog)")
  install(CODE "execute_process(COMMAND mkdir /var/pinusdb/syslog)")
  install(CODE "execute_process(COMMAND chmod 755 /etc/pinusdb)")
  install(CODE "execute_process(COMMAND chmod 755 /var/run/pinusdb.pid)")
  install(CODE "execute_process(COMMAND chmod 755 /var/pinusdb)")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
  add_executable(pinusdb ${SRC_LIST})
  target_link_libraries(pinusdb ws2_32.lib wsock32.lib dbghelp.lib zlib.lib)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
