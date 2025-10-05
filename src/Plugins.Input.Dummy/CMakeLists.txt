add_library(Mupen64RR.Plugins.Input.Dummy MODULE
    "Main.cpp"
    "Main.h"
)
set_target_properties(Mupen64RR.Plugins.Input.Dummy PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(Mupen64RR.Plugins.Input.Dummy PRIVATE
    Mupen64RR.Common
    Mupen64RR.Core.Headers
    Mupen64RR.Views.Win32.Headers
)