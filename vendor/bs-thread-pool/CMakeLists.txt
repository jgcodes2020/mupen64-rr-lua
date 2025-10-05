cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(bs-thread-pool
    DESCRIPTION "BS::thread_pool: a fast, lightweight, modern, and easy-to-use C++17/C++20/C++23 thread pool library"
    LANGUAGES CXX
)

add_library(bs-thread-pool INTERFACE)
target_sources(bs-thread-pool INTERFACE
    "BS_thread_pool.hpp"
)
target_include_directories(bs-thread-pool INTERFACE ".")