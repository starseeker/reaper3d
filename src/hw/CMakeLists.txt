# Hardware abstraction layer component
# Based on build/ac/hw/Makefile.am

add_library(hw STATIC
    concurrent.cpp
    debug.cpp
    event.cpp
    event_impl.cpp
    event_prim.cpp
    gfx_drv.cpp
    gfx_glfw.cpp
    gfx_osmesa.cpp
    event_glfw.cpp
    gl_defs.cpp
    gl_info.cpp
    gl_state.cpp
    glh_exts.cpp
    mapping.cpp
    netcompat.cpp
    profile.cpp
    snd_system.cpp
    snd_dummy.cpp
    snd_openal.cpp
    snd_wave.cpp
    snd_mp3.cpp
    socket.cpp
    stacktrace.cpp
    time.cpp
    time_types.cpp
    videomode.cpp
    worker.cpp
)

# Set include directories if needed
target_include_directories(hw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link GLFW for the windowing backend
target_link_libraries(hw PUBLIC glfw ext Threads::Threads)

if(OPENAL_FOUND)
    target_link_libraries(hw PUBLIC ${OPENAL_LIBRARIES})
    target_include_directories(hw PRIVATE ${OPENAL_INCLUDE_DIRS})
endif()

# Note: Plugin architecture has been removed.
# The system now uses GLFW directly for graphics and event handling.
