add_library(Mupen64RR.Plugins.Audio.Dummy MODULE
    "Main.cpp"
    "Main.h"
)
set_target_properties(Mupen64RR.Plugins.Audio.Dummy PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "no-audio"
    LIBRARY_OUTPUT_DIRECTORY "${MUPEN64RR_OUT_DIR}/plugins"
    PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(Mupen64RR.Plugins.Audio.Dummy PRIVATE
    Mupen64RR.Common
    Mupen64RR.Core.Headers
    Mupen64RR.Views.Win32.Headers
)