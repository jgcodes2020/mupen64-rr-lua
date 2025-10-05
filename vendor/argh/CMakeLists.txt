cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(argh
    DESCRIPTION "Argh! A minimalist argument handler."
    LANGUAGES CXX
)

add_library(argh INTERFACE)
target_sources(argh INTERFACE
    "argh.h"
)
target_include_directories(argh INTERFACE ".")