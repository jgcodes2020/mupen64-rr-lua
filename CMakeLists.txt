#[===[
Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).

SPDX-License-Identifier: GPL-2.0-or-later
]===]
cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(mupen64-rr
  VERSION 1.3.0
  DESCRIPTION "Powerful N64 TASing emulator with Lua scripting support"
  LANGUAGES C CXX
)

# initial checks
if(NOT WIN32)
  message(WARNING "This build setup has not been tested outside of Windows. You're on your own; good luck.")
endif()

if(WIN32 AND NOT DEFINED MUPEN64_VCPKG_TOOLCHAIN)
  message(FATAL_ERROR "Please use the provided toolchain file on Windows.")
endif()

# version suffix
set(MUPEN64RR_VERSION_SUFFIX "")
message(STATUS "Full version name: ${PROJECT_VERSION}${MUPEN64RR_VERSION_SUFFIX}")

# build output
set(MUPEN64RR_OUT_DIR "${PROJECT_BINARY_DIR}/out/")

# default build type
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(NOT CMAKE_BUILD_TYPE AND NOT is_multi_config)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "The build type to use with CMake. [Debug;Release;MinSizeRel;RelWithDebInfo]")
endif()

# CMake modules
# =======================================
include(CTest)

# LIBRARIES
# =======================================

# pkg-config libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(speexdsp IMPORTED_TARGET GLOBAL "speexdsp")

# cmake libraries
find_package(libdeflate CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)

# Lua, because Lua sucks
find_package(Lua REQUIRED)

if(LUA_FOUND AND NOT TARGET Lua::Lua)
  add_library(Lua::Lua INTERFACE IMPORTED)
  set_target_properties(
    Lua::Lua
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}"
  )
endif()

# vendored libraries
add_subdirectory(vendor)

# subprojects
add_subdirectory(src)

# tests
add_subdirectory(test)