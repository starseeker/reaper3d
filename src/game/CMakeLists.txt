# Game component library  
# Based on build/ac/game/Makefile.am

add_library(game STATIC
    forked.cpp
    helpers.cpp
    menus.cpp
    mission.cpp
    object_mgr.cpp
    scenario_mgr.cpp
    state.cpp
    state_base.cpp
)

# Set include directories if needed
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})