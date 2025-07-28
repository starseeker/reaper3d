# Networking component
# Based on build/ac/net/Makefile.am

add_library(net STATIC
    game_mgr.cpp
    nameservice.cpp
    net.cpp
    netgame.cpp
    server.cpp
    sockstream.cpp
)

# Set include directories if needed
target_include_directories(net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})