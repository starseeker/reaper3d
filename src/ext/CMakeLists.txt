# External libraries component (mpegsound)
# Based on build/ac/ext/Makefile.am

add_library(ext STATIC
    mpegsound/bitwindow.cpp
    mpegsound/filter.cpp
    mpegsound/filter_2.cpp
    mpegsound/huffmantable.cpp
    mpegsound/mpeglayer1.cpp
    mpegsound/mpeglayer2.cpp
    mpegsound/mpeglayer3.cpp
    mpegsound/mpegtable.cpp
    mpegsound/mpegtoraw.cpp
    mpegsound/soundplayer.cpp
)

# Set include directories if needed
target_include_directories(ext PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Dear ImGui is vendored for the GLFW-based editor tools.  Keep the backend
# choice here (rather than requiring a system package) so editor builds are
# reproducible on Linux and other GLFW platforms.
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl2.cpp
)
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)
target_link_libraries(imgui PUBLIC glfw ${OPENGL_LIBRARIES})
