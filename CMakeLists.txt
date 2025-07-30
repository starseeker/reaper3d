cmake_minimum_required(VERSION 3.16)
project(reaper3d)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add permissive compiler flags to handle legacy C++ code
add_compile_options(-fpermissive -Wno-deprecated -Wno-deprecated-declarations)

# Enable monolithic build - plugin architecture removed, using GLFW directly
add_compile_definitions(MONOLITHIC)

# Add src directory to include path for the whole project
include_directories(${CMAKE_SOURCE_DIR}/src)

# Find required system libraries
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Threads REQUIRED)

# Find GLFW for windowing and input handling
find_package(glfw3 REQUIRED)

# Try to find OpenAL (may not be available on all systems)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(OPENAL openal)
endif()

# Try to find OSMesa for headless rendering
find_path(OSMESA_INCLUDE_DIR GL/osmesa.h)
find_library(OSMESA_LIBRARY OSMesa)
if(OSMESA_INCLUDE_DIR AND OSMESA_LIBRARY)
    set(OSMESA_FOUND TRUE)
    add_compile_definitions(HAVE_OSMESA)
    message(STATUS "Found OSMesa: ${OSMESA_LIBRARY}")
else()
    set(OSMESA_FOUND FALSE)
    message(STATUS "OSMesa not found - headless rendering disabled")
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
    ext game main net object phys prototype res 
    # Group libraries with potential circular dependencies
    -Wl,--start-group ai gfx misc snd world hw pm shadow cloud -Wl,--end-group
    # System libraries
    ${OPENGL_LIBRARIES}
    ${X11_LIBRARIES}
    glfw
    ZLIB::ZLIB
    PNG::PNG
    Threads::Threads
)

# Add OpenAL if found
if(OPENAL_FOUND)
    target_link_libraries(reaper3d ${OPENAL_LIBRARIES})
    target_include_directories(reaper3d PRIVATE ${OPENAL_INCLUDE_DIRS})
endif()

# Add OSMesa if found
if(OSMESA_FOUND)
    target_link_libraries(reaper3d ${OSMESA_LIBRARY})
    target_include_directories(reaper3d PRIVATE ${OSMESA_INCLUDE_DIR})
endif()

# Set output directories
set_target_properties(reaper3d PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy data directory to build folder so assets can be found at runtime
add_custom_target(copy_data_dir ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying data directory to build folder"
)

# Ensure data is copied before building the main executable
add_dependencies(reaper3d copy_data_dir)