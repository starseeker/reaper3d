cmake_minimum_required(VERSION 3.16)
project(reaper3d)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add main source directories
add_subdirectory(src/ai)
add_subdirectory(src/ext)
add_subdirectory(src/game)
add_subdirectory(src/gfx)
add_subdirectory(src/hw)
add_subdirectory(src/main)
add_subdirectory(src/misc)
add_subdirectory(src/net)
add_subdirectory(src/object)
add_subdirectory(src/phys)
add_subdirectory(src/public)
add_subdirectory(src/res)
add_subdirectory(src/snd)
add_subdirectory(src/world)

# Main executable (placeholder - to be implemented later)
# add_executable(reaper)
# target_link_libraries(reaper 
#     ai ext game gfx hw main misc net object phys prototype res snd world
# )