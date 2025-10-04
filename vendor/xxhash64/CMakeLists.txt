cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(xxhash64
    DESCRIPTION "Stripped-down implementation of xxhash64."
    LANGUAGES CXX
)

add_library(xxhash64 INTERFACE)
target_sources(xxhash64 INTERFACE
    "xxh64.h"
)
target_include_directories(xxhash64 INTERFACE ".")