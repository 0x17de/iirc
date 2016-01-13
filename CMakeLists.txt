cmake_minimum_required(VERSION 3.3)
project(iirc)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/main.cpp src/IniReader.cpp src/IniReader.h src/DatabaseHandler.cpp src/DatabaseHandler.h src/Application.cpp src/Application.h src/UserHandler.cpp src/UserHandler.h src/irc/IrcClient.cpp src/irc/IrcClient.h src/data/ServerData.h src/config/db.h src/irc/IrcClientImpl.cpp src/irc/IrcClientImpl.h src/data/ChannelData.cpp src/data/ChannelData.h src/irc/IrcEvent.h src/tcp/TcpInterface.cpp src/tcp/TcpInterface.h src/tcp/TcpInterfaceImpl.cpp src/tcp/TcpInterfaceImpl.h src/tcp/TcpClient.cpp src/tcp/TcpClient.h)


find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS
		src/commands/common.proto
		src/commands/server.proto
		src/commands/client.proto)


add_executable(iirc ${SOURCE_FILES} ${PROTO_SRCS} ${PROTO_HDRS})


target_link_libraries(iirc ${PROTOBUF_LIBRARIES})


find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(iirc ${Boost_LIBRARIES})

find_package(OpenSSL REQUIRED)
if(${OPENSSL_FOUND})
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries(iirc ${OPENSSL_LIBRARIES})
else()
  message(WARNING "OPENSSL NOT FOUND")
  set(MISSING_LIB 1)
endif()

find_package(Soci REQUIRED)
if(${SOCI_FOUND})
  include_directories(${SOCI_INCLUDE_DIRS})
  target_link_libraries(iirc ${SOCI_LIBRARY} ${SOCI_postgresql_PLUGIN})
else()
  message(WARNING "SOCI NOT FOUND")
  set(MISSING_LIB 1)
endif()

find_package(LibIrcClient REQUIRED)
if(${LibIrcClient_FOUND})
  include_directories(${LibIrcClient_INCLUDE_DIRS})
  target_link_libraries(iirc ${LibIrcClient_LIBRARY})
else()
  message(WARNING "LibIrcClient NOT FOUND")
  set(MISSING_LIB 1)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
