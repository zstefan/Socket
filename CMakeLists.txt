cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)

set(this MySocket)

project(${this})

add_library(${this} SHARED ./include/Network/MySocket.h ./src/MySocket.cpp)

target_include_directories(${this} PUBLIC include)

if(WIN32)
  target_link_libraries(${this} wsock32 ws2_32)
endif()
