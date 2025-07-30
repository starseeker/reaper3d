# Graphics component with subdirectories
# Based on build/ac/gfx/Makefile.am

# Add subdirectories first
add_subdirectory(cloud)
add_subdirectory(pm)
add_subdirectory(shadow)

# Main graphics library
add_library(gfx STATIC
    billboard.cpp
    camera.cpp
    cubemap.cpp
    dynamic_texture.cpp
    effects.cpp
    envmapper.cpp
    exceptions.cpp
    fps_meter.cpp
    geometry.cpp
    gfx.cpp
    gfx_init.cpp
    gfx_render.cpp
    gfx_screenshot.cpp
    gfx_simulate.cpp
    hud.cpp
    instances.cpp
    io.cpp
    light.cpp
    light_mgr.cpp
    mesh.cpp
    mesh_file.cpp
    mesh_file_read.cpp
    mesh_mgr.cpp
    misc.cpp
    particles.cpp
    png.cpp
    settings.cpp
    shaders.cpp
    sky.cpp
    static_geom.cpp
    statistics.cpp
    terrain.cpp
    texture.cpp
    texture_mgr.cpp
    vbo_utils.cpp
    vertex_mgr.cpp
    water.cpp
)

# Set include directories if needed
target_include_directories(gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})