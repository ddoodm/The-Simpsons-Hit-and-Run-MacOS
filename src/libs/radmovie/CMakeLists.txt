# Generated from radmovie.vcxproj by tools/gen_cmake_sources.py.

set(radmovie_SOURCES
    src/common/audiodatasource.cpp
    src/common/binkrenderstrategy.cpp
    src/common/ffmpegmovieplayer.cpp
    src/pch/pch.cpp)

srr2_add_library(radmovie
    SOURCES ${radmovie_SOURCES}
    PUBLIC_INCLUDES include
    INCLUDES src/common src/pch
    LINK radcore radsound pure3d radmath radcontent srr2::ffmpeg
)
