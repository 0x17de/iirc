cmake_minimum_required(VERSION 3.3)
project(iirc)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/main.cpp src/IniReader.cpp src/IniReader.h src/DatabaseHandler.cpp src/DatabaseHandler.h src/Application.cpp src/Application.h src/UserHandler.cpp src/UserHandler.h src/irc/IrcClient.cpp src/irc/IrcClient.h src/data/ServerData.h src/config/db.h src/irc/IrcClientImpl.cpp src/irc/IrcClientImpl.h src/data/ChannelData.h src/irc/IrcEvent.h)



add_executable(iirc ${SOURCE_FILES})

find_package(Soci)
if(${SOCI_FOUND})
  target_link_libraries(iirc ${SOCI_LIBRARY} ${SOCI_postgresql_PLUGIN})
else()
  message(WARNING "SOCI NOT FOUND")
  set(MISSING_LIB 1)
endif()

find_package(LibIrcClient)
if(${LibIrcClient_FOUND})
  target_link_libraries(iirc ${LibIrcClient_LIBRARY})
else()
  message(WARNING "LibIrcClient NOT FOUND")
  set(MISSING_LIB 1)
endif()

include_directories(${SOCI_INCLUDE_DIRS} ${LibIrcClient_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src)