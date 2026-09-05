# Third-party dependency discovery.
#
# Two supported sources: the vcpkg manifest (vcpkg.json) or system packages
# (Homebrew on macOS). Each dependency ends up behind a srr2::* target so the
# library CMakeLists never care which one supplied it.

find_package(PkgConfig QUIET)

# Homebrew installs some formulae keg-only, so they are not on the default
# search path. Ask brew where they live.
if(APPLE)
    find_program(SRR2_BREW brew)
    if(SRR2_BREW)
        foreach(formula openal-soft ffmpeg libpng sdl2 sdl2-compat)
            execute_process(COMMAND "${SRR2_BREW}" --prefix ${formula}
                OUTPUT_VARIABLE prefix
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
            if(prefix AND IS_DIRECTORY "${prefix}")
                list(APPEND CMAKE_PREFIX_PATH "${prefix}")
                if(EXISTS "${prefix}/lib/pkgconfig")
                    set(ENV{PKG_CONFIG_PATH} "${prefix}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
                endif()
            endif()
        endforeach()
    endif()
endif()

# --- SDL2 ---------------------------------------------------------------------
add_library(srr2_sdl2 INTERFACE)
add_library(srr2::sdl2 ALIAS srr2_sdl2)

find_package(SDL2 CONFIG QUIET)
if(TARGET SDL2::SDL2)
    target_link_libraries(srr2_sdl2 INTERFACE SDL2::SDL2)
    set(SRR2_SDL2_ORIGIN "CMake config package")
elseif(PkgConfig_FOUND)
    pkg_check_modules(SDL2_PC REQUIRED IMPORTED_TARGET sdl2)
    target_link_libraries(srr2_sdl2 INTERFACE PkgConfig::SDL2_PC)
    set(SRR2_SDL2_ORIGIN "pkg-config ${SDL2_PC_VERSION}")
else()
    message(FATAL_ERROR "SDL2 not found. Install it (brew install sdl2) or supply vcpkg.")
endif()

# --- OpenAL -------------------------------------------------------------------
add_library(srr2_openal INTERFACE)
add_library(srr2::openal ALIAS srr2_openal)

if(PkgConfig_FOUND)
    pkg_check_modules(OPENAL_PC QUIET IMPORTED_TARGET openal)
endif()
if(TARGET PkgConfig::OPENAL_PC)
    target_link_libraries(srr2_openal INTERFACE PkgConfig::OPENAL_PC)
    set(SRR2_OPENAL_ORIGIN "pkg-config ${OPENAL_PC_VERSION}")
else()
    find_package(OpenAL REQUIRED)
    target_link_libraries(srr2_openal INTERFACE OpenAL::OpenAL)
    set(SRR2_OPENAL_ORIGIN "FindOpenAL")
endif()

# radsound includes the EFX headers as <AL/efx.h> style paths on some
# distributions and bare on others; expose both roots.
get_target_property(_openal_inc srr2_openal INTERFACE_INCLUDE_DIRECTORIES)

# --- libpng -------------------------------------------------------------------
find_package(PNG REQUIRED)
add_library(srr2_png INTERFACE)
add_library(srr2::png ALIAS srr2_png)
target_link_libraries(srr2_png INTERFACE PNG::PNG)
set(SRR2_PNG_ORIGIN "${PNG_VERSION_STRING}")

# --- FFmpeg -------------------------------------------------------------------
add_library(srr2_ffmpeg INTERFACE)
add_library(srr2::ffmpeg ALIAS srr2_ffmpeg)

set(SRR2_FFMPEG_MODULES libavcodec libavformat libavutil libswscale libswresample)
if(PkgConfig_FOUND)
    pkg_check_modules(FFMPEG_PC QUIET IMPORTED_TARGET ${SRR2_FFMPEG_MODULES})
endif()
if(TARGET PkgConfig::FFMPEG_PC)
    target_link_libraries(srr2_ffmpeg INTERFACE PkgConfig::FFMPEG_PC)
    set(SRR2_FFMPEG_ORIGIN "pkg-config ${FFMPEG_PC_libavcodec_VERSION}")
else()
    find_package(FFMPEG REQUIRED)
    target_include_directories(srr2_ffmpeg INTERFACE ${FFMPEG_INCLUDE_DIRS})
    target_link_libraries(srr2_ffmpeg INTERFACE ${FFMPEG_LIBRARIES})
    set(SRR2_FFMPEG_ORIGIN "FindFFMPEG")
endif()

# --- OpenGL -------------------------------------------------------------------
# macOS exports the whole GL 2.1 entry point set straight from OpenGL.framework,
# so the glad loader is only needed on platforms that resolve GL at runtime.
add_library(srr2_gl INTERFACE)
add_library(srr2::gl ALIAS srr2_gl)

if(APPLE)
    find_library(SRR2_OPENGL_FRAMEWORK OpenGL REQUIRED)
    target_link_libraries(srr2_gl INTERFACE "${SRR2_OPENGL_FRAMEWORK}")
    target_compile_definitions(srr2_gl INTERFACE
        SRR2_GL_FRAMEWORK
        GL_SILENCE_DEPRECATION)
    set(SRR2_GL_ORIGIN "OpenGL.framework (no loader)")
else()
    find_package(glad CONFIG REQUIRED)
    target_link_libraries(srr2_gl INTERFACE glad::glad)
    set(SRR2_GL_ORIGIN "glad")
endif()
