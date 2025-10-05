cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(mini
    DESCRIPTION "INI file reader and writer"
    LANGUAGES CXX
)

add_library(mini INTERFACE)
target_sources(mini INTERFACE
    "ini.h"
)
target_include_directories(mini INTERFACE ".")