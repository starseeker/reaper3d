# Graphics cloud system subcomponent
# Based on build/ac/gfx/cloud/Makefile.am

add_library(cloud STATIC
    cloud.cpp
    impostor.cpp
    particle.cpp
    simulate.cpp
)

# Set include directories if needed
target_include_directories(cloud PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})