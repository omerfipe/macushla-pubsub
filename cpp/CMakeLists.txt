cmake_minimum_required(VERSION 3.10)
project(macushla_pubsub CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(node2_pub node2_pub.cpp)
add_executable(node4_sub node4_sub.cpp)

# WinSock
if (WIN32)
  target_link_libraries(node2_pub ws2_32)
  target_link_libraries(node4_sub ws2_32)
endif()
