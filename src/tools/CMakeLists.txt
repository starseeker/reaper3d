# Simple tools for testing
add_executable(service_handle_test service_handle_test.cpp)
set_target_properties(service_handle_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(BUILD_TESTING)
    add_test(NAME service_handles COMMAND service_handle_test)
endif()

if(OSMESA_FOUND)
    add_executable(osmesa_test osmesa_test.cpp)
    target_link_libraries(osmesa_test ${OSMESA_LIBRARY} ${OPENGL_LIBRARIES})
    target_include_directories(osmesa_test PRIVATE ${OSMESA_INCLUDE_DIR})
    
    add_executable(vbo_test vbo_test.cpp)
    target_link_libraries(vbo_test gfx hw ${OSMESA_LIBRARY} ${OPENGL_LIBRARIES})
    target_include_directories(vbo_test PRIVATE ${OSMESA_INCLUDE_DIR})
    
    add_executable(shader_test shader_test.cpp)
    target_link_libraries(shader_test gfx hw ${OSMESA_LIBRARY} ${OPENGL_LIBRARIES})
    
    set_target_properties(osmesa_test vbo_test shader_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    if(BUILD_TESTING)
        add_test(NAME headless_osmesa COMMAND osmesa_test)
        add_test(NAME headless_vbo COMMAND vbo_test)
        add_test(NAME headless_shaders COMMAND shader_test)
        set_tests_properties(headless_osmesa headless_vbo headless_shaders
            PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endif()
endif()

# Modern replacements for the old GLUT/GLUI editors.  These are interactive
# tools, so they are deliberately not registered as CTest tests.
add_executable(level_editor editor_imgui.cpp)
target_compile_definitions(level_editor PRIVATE REAPER_EDITOR_TITLE="Reaper3D Level Editor")
target_link_libraries(level_editor PRIVATE imgui
    -Wl,--start-group world res ai misc main gfx hw -Wl,--end-group
    ${OPENGL_LIBRARIES} glfw)
set_target_properties(level_editor PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(navigraph_editor editor_imgui.cpp)
target_compile_definitions(navigraph_editor PRIVATE REAPER_EDITOR_TITLE="Reaper3D Navigation Graph Editor" REAPER_NAVIGRAPH_EDITOR=1)
target_link_libraries(navigraph_editor PRIVATE imgui
    -Wl,--start-group world res ai misc main gfx hw -Wl,--end-group
    ${OPENGL_LIBRARIES} glfw)
set_target_properties(navigraph_editor PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
