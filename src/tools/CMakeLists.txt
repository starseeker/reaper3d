# Simple tools for testing
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
endif()