# Hardware abstraction layer component
# Based on build/ac/hw/Makefile.am

add_library(hw STATIC
    compat.cpp
    conc_pthread.cpp
    debug.cpp
    dynload.cpp
    event.cpp
    event_impl.cpp
    event_prim.cpp
    gfx_drv.cpp
    gfx_glfw.cpp
    event_glfw.cpp
    gl_defs.cpp
    gl_info.cpp
    gl_state.cpp
    glh_exts.cpp
    mapping.cpp
    netcompat.cpp
    osinfo.cpp
    profile.cpp
    snd_system.cpp
    snd_dummy.cpp
    socket.cpp
    stacktrace.cpp
    time.cpp
    time_types.cpp
    videomode.cpp
    worker.cpp
)

# Set include directories if needed
target_include_directories(hw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link GLFW for the new windowing backend
target_link_libraries(hw glfw)

# Note: The original build system also creates plugin libraries
# These would be implemented as separate CMake targets:
# - gfx_x11 module (legacy, being migrated to GLFW)
# - gfx_glfw module (new GLFW-based windowing)
# - event_x11 module (legacy, being migrated to GLFW)  
# - event_glfw module (new GLFW-based input handling)
# - snd_dummy, snd_wave, snd_mp3, snd_simple, snd_openal modules
# - libconc_pthreads, libconc_win32 modules