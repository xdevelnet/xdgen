cmake_minimum_required(VERSION 3.2)
project(xdgen C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")

set(SOURCE_FILES
    main.c parser_routines.h routines.h constants_and_globals.h mem_handling.h)

add_executable(xdgen ${SOURCE_FILES})
