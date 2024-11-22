cmake_minimum_required(VERSION 3.0)

project(solar)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -O0")

add_subdirectory(src)
add_subdirectory(tests)