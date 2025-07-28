# Miscellaneous utilities component  
# Based on build/ac/misc/Makefile.am

add_library(misc STATIC
    font.cpp
    menu_system.cpp
    parse.cpp
    plugin.cpp
    ptrstats.cpp
    unique.cpp
    utility.cpp
)

# Set include directories if needed
target_include_directories(misc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})