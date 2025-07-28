# Resource management component
# Based on build/ac/res/Makefile.am

add_library(res STATIC
    config.cpp
    res.cpp
    resource.cpp
    zstream.cpp
)

# Set include directories if needed
target_include_directories(res PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})