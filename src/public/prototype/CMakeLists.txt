# Public prototype component
# Based on build/ac/public/prototype/Makefile.am

add_library(prototype STATIC
    prototype.cpp
)

# Set include directories if needed
target_include_directories(prototype PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})