# Sound system component
# Based on build/ac/snd/Makefile.am

add_library(snd STATIC
    sound_system.cpp
)

# Set include directories if needed
target_include_directories(snd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})