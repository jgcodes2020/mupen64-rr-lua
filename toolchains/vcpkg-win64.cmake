#[===[
Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).

SPDX-License-Identifier: GPL-2.0-or-later
]===]

# Toolchain file for the Visual Studio pipeline.
# =========================================================

if(NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    message(FATAL_ERROR "This toolchain file is designed for use on Windows.")
endif()

if (NOT DEFINED ENV{VSCMD_VER})
    message(FATAL_ERROR "Please build from a Visual Studio developer environment (cmd.exe or PowerShell)")
endif()

set(
    MUPEN64_VCPKG_TOOLCHAIN "$ENV{VSINSTALLDIR}VC\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake"
    CACHE INTERNAL "Location of vcpkg's toolchain file."
)
if (NOT EXISTS "${MUPEN64_VCPKG_TOOLCHAIN}")
    message(FATAL_ERROR "Expected vcpkg.cmake at ${MUPEN64_VCPKG_TOOLCHAIN}")
endif()

# set some necessary settings to get compilation to work properly
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "MSVCRT variant needed to get things to work.")
add_compile_definitions(UNICODE)

# setup a few last values for vcpkg
set(VCPKG_TARGET_TRIPLET "$ENV{VSCMD_ARG_TGT_ARCH}-windows-static" CACHE INTERNAL "target triplet for vcpkg")


message(STATUS "VS architecture set to: $ENV{VSCMD_ARG_TGT_ARCH}")

# hand off the rest to vcpkg
include(${MUPEN64_VCPKG_TOOLCHAIN})