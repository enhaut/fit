cmake_minimum_required(VERSION 3.17)
project(table_manager)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(table_examples)

add_executable(table_manager
        table_examples/tab2.txt
        Makefile
        README.md
        sheet.c)
macro(use_c99)
    if (CMAKE_VERSION VERSION_LESS "3.1")
        if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
            set (CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
        endif ()
    else ()
        set (CMAKE_C_STANDARD 99)
    endif ()
endmacro(use_c99)
