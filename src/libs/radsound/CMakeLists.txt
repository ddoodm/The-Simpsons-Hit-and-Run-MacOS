# Generated from radsound.vcxproj by tools/gen_cmake_sources.py.

set(radsound_SOURCES
    src/common/radsoundobject.cpp
    src/common/radsoundupdatableobject.cpp
    src/hal/common/audioformat.cpp
    src/hal/common/banner.cpp
    src/hal/common/memoryregion.cpp
    src/hal/common/radsoundfile.cpp
    src/hal/common/rolloff.cpp
    src/hal/win32/buffer.cpp
    src/hal/win32/bufferloader.cpp
    src/hal/win32/effect.cpp
    src/hal/win32/listener.cpp
    src/hal/win32/positionalgroup.cpp
    src/hal/win32/radsoundwin.cpp
    src/hal/win32/system.cpp
    src/hal/win32/voice.cpp
    src/math/radsoundmath.cpp
    src/pch/pch.cpp
    src/radsound/buffereddatasource.cpp
    src/radsound/clip.cpp
    src/radsound/clipplayer.cpp
    src/radsound/datacache.cpp
    src/radsound/memorydatasource.cpp
    src/radsound/memoryspaceobject.cpp
    src/radsound/radicaladpcm.cpp
    src/radsound/rsdfiledatasource.cpp
    src/radsound/stitcheddatasource.cpp
    src/radsound/streamplayer.cpp)

srr2_add_library(radsound
    SOURCES ${radsound_SOURCES}
    PUBLIC_INCLUDES include
    INCLUDES src/common src/pch
    LINK radcore srr2::openal
)
