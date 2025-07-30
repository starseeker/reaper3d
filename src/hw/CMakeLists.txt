# Hardware abstraction layer component
# Based on build/ac/hw/Makefile.am

add_library(hw STATIC
    compat.cpp
    conc_pthread.cpp
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

# Link GLFW for the windowing backend
target_link_libraries(hw glfw)

# Note: Plugin architecture has been removed.
# The system now uses GLFW directly for graphics and event handling.