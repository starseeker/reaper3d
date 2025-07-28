# Physics component
# Based on build/ac/phys/Makefile.am

add_library(phys STATIC
    collisionQueue.cpp
    engine.cpp
    pairs.cpp
)

# Set include directories if needed
target_include_directories(phys PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})