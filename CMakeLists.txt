cmake_minimum_required(VERSION 3.10)

project(socks5-server)

IF (CMAKE_BUILD_TYPE STREQUAL Debug)
  ADD_DEFINITIONS(-DDEBUG)
ENDIF()

add_executable(${PROJECT_NAME} main.cc  worker.cc listener.cc handshake.cc)
set(CMAKE_CXX_FLAGS -Werror)
target_link_libraries(${PROJECT_NAME} fmt)

