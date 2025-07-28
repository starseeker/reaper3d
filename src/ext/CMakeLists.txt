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