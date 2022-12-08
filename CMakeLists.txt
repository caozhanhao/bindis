cmake_minimum_required(VERSION 3.24)
project(bindis)

set(CMAKE_CXX_STANDARD 17)
add_executable(bindis main.cpp)
target_link_libraries(bindis pthread)
