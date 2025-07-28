# Main component library
# Based on build/ac/main/Makefile.am

add_library(main STATIC
    types_io.cpp
    types_ops.cpp
)

# Set include directories if needed
target_include_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})