# World system component
# Based on build/ac/world/Makefile.am

add_library(world STATIC
    geometry.cpp
    level.cpp
    quadtree.cpp
    search.cpp
    world.cpp
    world_impl.cpp
)

# Set include directories if needed
target_include_directories(world PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})