cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(mupen64-rr
    VERSION 1.3.0.4
    DESCRIPTION "Powerful N64 TASing emulator with Lua scripting support"
    LANGUAGES C CXX
)

if (NOT WIN32)
    message(WARNING "Not building on Windows. Things may not work as expected.")
endif()

# pkg-config libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(xxhash IMPORTED_TARGET GLOBAL "libxxhash")

# cmake libraries
find_package(libdeflate CONFIG REQUIRED)

# vendored libraries
add_subdirectory(vendor)

# subprojects
add_subdirectory(src/Common)
add_subdirectory(src/Core)