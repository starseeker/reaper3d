cmake_minimum_required(VERSION 3.10)

project(reaper VERSION 0.98.2 LANGUAGES CXX C)

# Set default C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add component directories
add_subdirectory(build/ac/ai)
add_subdirectory(build/ac/ext)
add_subdirectory(build/ac/game)
add_subdirectory(build/ac/gfx)
add_subdirectory(build/ac/hw)
add_subdirectory(build/ac/net)
add_subdirectory(build/ac/object)
add_subdirectory(build/ac/phys)
add_subdirectory(build/ac/snd)
add_subdirectory(build/ac/world)