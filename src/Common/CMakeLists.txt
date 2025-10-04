add_library(Mupen64RR.Common INTERFACE
    "include/CommonPCH.h"
    "include/MiscHelpers.h"
    "include/PlatformService.h"
)
set_target_properties(Mupen64RR.Common PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(Mupen64RR.Common INTERFACE include)
target_link_libraries(Mupen64RR.Common INTERFACE
    vendor::xxhash64
)
target_precompile_headers(Mupen64RR.Common INTERFACE "include/CommonPCH.h")