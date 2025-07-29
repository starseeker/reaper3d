# AI component library
# Based on build/ac/ai/Makefile.am

add_library(ai STATIC
    astar.cpp
    fsm.cpp
    gvbase.cpp
    gvcontainer.cpp
    gvordinary.cpp
    gvturret.cpp
    navigation_graph.cpp
    player.cpp
    ship_base.cpp
    ship_bomber.cpp
    ship_fighter.cpp
    steering.cpp
    turret.cpp
)

# Set C++17 features and include directories
target_compile_features(ai PUBLIC cxx_std_17)
target_include_directories(ai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})