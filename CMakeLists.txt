cmake_minimum_required(VERSION 3.16)
project(reaper3d)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add permissive compiler flags to handle legacy C++ code
add_compile_options(-fpermissive -Wno-deprecated -Wno-deprecated-declarations)

# Add src directory to include path for the whole project
include_directories(${CMAKE_SOURCE_DIR}/src)

# Find required system libraries
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Threads REQUIRED)

# Try to find OpenAL (may not be available on all systems)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(OPENAL openal)
endif()

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

# Main executable
add_executable(reaper3d
    src/public/prototype/prototype.cpp
)

# The main function is provided by test_main.h which is included in prototype.cpp
target_link_libraries(reaper3d 
    ai ext game gfx hw main misc net object phys prototype res snd world
    # System libraries
    ${OPENGL_LIBRARIES}
    ${X11_LIBRARIES}
    ZLIB::ZLIB
    PNG::PNG
    Threads::Threads
)

# Add OpenAL if found
if(OPENAL_FOUND)
    target_link_libraries(reaper3d ${OPENAL_LIBRARIES})
    target_include_directories(reaper3d PRIVATE ${OPENAL_INCLUDE_DIRS})
endif()

# Set output directories
set_target_properties(reaper3d PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)