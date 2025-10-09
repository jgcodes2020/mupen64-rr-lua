add_executable(Mupen64RR.Core.Tests
    "stdafx.h"
    "vcr_tests.cpp"
)
set_target_properties(Mupen64RR.Core.Tests PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
target_precompile_headers(Mupen64RR.Core.Tests PRIVATE "stdafx.h")
target_link_libraries(Mupen64RR.Core.Tests PRIVATE
    Catch2::Catch2WithMain
    Mupen64RR.Core._TestIncludes
)
add_test(NAME "core-tests" COMMAND Mupen64RR.Core.Tests)