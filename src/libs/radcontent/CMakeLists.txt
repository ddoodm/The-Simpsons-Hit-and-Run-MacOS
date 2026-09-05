# Generated from radcontent.vcxproj by tools/gen_cmake_sources.py.

set(radcontent_SOURCES
    src/pch/pch.cpp
    src/radload/hashtable.cpp
    src/radload/inventory.cpp
    src/radload/manager.cpp
    src/radload/object.cpp
    src/radload/queue.cpp
    src/radload/request.cpp
    src/radload/stream.cpp)

srr2_add_library(radcontent
    SOURCES ${radcontent_SOURCES}
    PUBLIC_INCLUDES include .
    INCLUDES src src/pch
    LINK radcore radmath
)
