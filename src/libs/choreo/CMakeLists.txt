# Generated from choreo.vcxproj by tools/gen_cmake_sources.py.

set(choreo_SOURCES
    src/animation.cpp
    src/bank.cpp
    src/basebank.cpp
    src/blend.cpp
    src/blendtemplate.cpp
    src/constants.cpp
    src/driver.cpp
    src/engine.cpp
    src/foot.cpp
    src/footblenddriver.cpp
    src/footblender.cpp
    src/footplanter.cpp
    src/internalanimation.cpp
    src/jointblenddriver.cpp
    src/jointblender.cpp
    src/load.cpp
    src/locomotion.cpp
    src/partition.cpp
    src/puppet.cpp
    src/replay.cpp
    src/replayblenddriver.cpp
    src/replayblender.cpp
    src/rig.cpp
    src/root.cpp
    src/rootblenddriver.cpp
    src/rootblender.cpp
    src/scriptreader.cpp
    src/scriptwriter.cpp
    src/synchronization.cpp
    src/transition.cpp
    src/utility.cpp)

srr2_add_library(choreo
    SOURCES ${choreo_SOURCES}
    PUBLIC_INCLUDES include
    LINK pure3d poser sim radmath radcore radcontent
)
