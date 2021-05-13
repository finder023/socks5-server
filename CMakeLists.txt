cmake_minimum_required(VERSION 3.10)

project(socks5-server)

set(CMAKE_CXX_STANDARD 17)
IF (CMAKE_BUILD_TYPE STREQUAL Debug)
  ADD_DEFINITIONS(-DDEBUG)
ENDIF()

add_executable(${PROJECT_NAME} 
  main.cc  
  worker.cc 
  listener.cc 
  handshake-socks5.cc 
  handshake-private.cc
  handshake-pass.cc
  handshake-ss.cc
  confirm.cc
)

set(CMAKE_CXX_FLAGS -Werror)
target_link_libraries(${PROJECT_NAME} fmt)

