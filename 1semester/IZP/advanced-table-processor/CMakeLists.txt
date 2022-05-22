cmake_minimum_required(VERSION 3.17)
project(advanced_table_processor C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic")

add_executable(sps
        sps.c)
