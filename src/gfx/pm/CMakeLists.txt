# Graphics progressive mesh subcomponent
# Based on build/ac/gfx/pm/Makefile.am

add_library(pm STATIC
    badheap.cpp
    pm.cpp
    treap.cpp
)

# Set include directories if needed
target_include_directories(pm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})