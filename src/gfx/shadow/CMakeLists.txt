# Graphics shadow system subcomponent
# Based on build/ac/gfx/shadow/Makefile.am

add_library(shadow STATIC
    main.cpp
    silhouette_all.cpp
    silhouette_dynamic.cpp
    silhouette_player.cpp
    simple_dynamic.cpp
    simple_player.cpp
    types.cpp
    utility.cpp
)

# Set include directories if needed
target_include_directories(shadow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})