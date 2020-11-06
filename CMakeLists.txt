cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)

project(MySocket)

add_library(MySocket SHARED ./include/Network/MySocket.h ./src/MySocket.cpp)

target_include_directories(MySocket PUBLIC include)


