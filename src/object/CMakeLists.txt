# Object system component
# Based on build/ac/object/Makefile.am

add_library(object STATIC
    base.cpp
    collide.cpp
    controls.cpp
    factory.cpp
    gvs.cpp
    hull.cpp
    mkinfo.cpp
    obj_factory.cpp
    object.cpp
    object_impl.cpp
    objectgroup.cpp
    objevent.cpp
    phys.cpp
    phys_data.cpp
    projectile.cpp
    renderer.cpp
    sheep.cpp
    ship.cpp
    shot.cpp
    sound.cpp
    tree.cpp
    turret.cpp
    weapon.cpp
)

# Set include directories if needed
target_include_directories(object PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})