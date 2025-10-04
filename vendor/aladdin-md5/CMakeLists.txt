cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(aladdin-md5
    DESCRIPTION "Implementation of the MD5 hash"
    LANGUAGES C
)

add_library(aladdin-md5 STATIC "md5.c" "md5.h")
target_include_directories(aladdin-md5 PUBLIC ".")