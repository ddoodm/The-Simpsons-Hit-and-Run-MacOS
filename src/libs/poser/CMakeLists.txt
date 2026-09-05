# Generated from poser.vcxproj by tools/gen_cmake_sources.py.

set(poser_SOURCES
    src/joint.cpp
    src/pose.cpp
    src/posedriver.cpp
    src/poseengine.cpp
    src/transform.cpp)

srr2_add_library(poser
    SOURCES ${poser_SOURCES}
    PUBLIC_INCLUDES include
    LINK pure3d radmath radcore radcontent
)
