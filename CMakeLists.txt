cmake_minimum_required(VERSION 3.28)
project(lw_malloc C)

set(CMAKE_C_STANDARD 11)
# 设置调试选项
set(CMAKE_BUILD_TYPE Debug)  # 设置为Debug模式
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")  # 添加-g调试符号

add_executable(lw_malloc main.c
        lw_malloc.c
        lw_malloc.h)

