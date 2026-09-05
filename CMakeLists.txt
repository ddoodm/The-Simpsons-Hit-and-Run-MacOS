cmake_minimum_required(VERSION 3.24)

# Toolchain must be selected before project(). vcpkg is optional: on macOS the
# dependencies are usually supplied by Homebrew instead.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

project(SRR2
    VERSION 1.0.0
    DESCRIPTION "The Simpsons: Hit & Run"
    LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# RelWithDebInfo stands in for the old "Tune" configuration.
set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(SRR2Compiler)
include(SRR2Dependencies)

add_subdirectory(src/libs/radmath)
add_subdirectory(src/libs/radcore)
add_subdirectory(src/libs/radcontent)
add_subdirectory(src/libs/radscript)
add_subdirectory(src/libs/radsound)
add_subdirectory(src/libs/radmusic)
add_subdirectory(src/libs/pure3d)
add_subdirectory(src/libs/poser)
add_subdirectory(src/libs/sim)
add_subdirectory(src/libs/choreo)
add_subdirectory(src/libs/scrooby)
add_subdirectory(src/libs/radmovie)
add_subdirectory(src/srr2)

message(STATUS "")
message(STATUS "SRR2 ${PROJECT_VERSION} configured")
message(STATUS "  System      : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Compiler    : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS "  SDL2        : ${SRR2_SDL2_ORIGIN}")
message(STATUS "  OpenAL      : ${SRR2_OPENAL_ORIGIN}")
message(STATUS "  libpng      : ${SRR2_PNG_ORIGIN}")
message(STATUS "  FFmpeg      : ${SRR2_FFMPEG_ORIGIN}")
message(STATUS "  OpenGL      : ${SRR2_GL_ORIGIN}")
message(STATUS "")
