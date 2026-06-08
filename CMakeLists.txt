cmake_minimum_required(VERSION 4.2)
project(my_project)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(my_project
        sts_io.c
        sts_io.h
        main.c
)