cmake_minimum_required(VERSION 3.28)
project(Malloc C)

set(CMAKE_C_STANDARD 11)

add_executable(Malloc main.c
        Malloc.c
        Malloc.h)
