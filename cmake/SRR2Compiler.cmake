# Shared compile settings. Every SRR2 target links this.
add_library(srr2_options INTERFACE)

# RAD_* names mirror the old Visual Studio configurations: Debug, Release and
# Tune. RelWithDebInfo takes the place of Tune.
target_compile_definitions(srr2_options INTERFACE
    $<$<CONFIG:Debug>:_DEBUG;DEBUG;RAD_DEBUG>
    $<$<CONFIG:Release>:NDEBUG;RELEASE;RAD_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:NDEBUG;TUNE;RAD_TUNE>)

if(APPLE)
    target_compile_definitions(srr2_options INTERFACE RAD_MACOS RAD_UNIX)
elseif(WIN32)
    # RAD_FORCE_FEEDBACK gates the force-feedback effect classes, which are
    # written directly against DirectInput's DIEFFECT.
    target_compile_definitions(srr2_options INTERFACE
        RAD_WIN32 WIN64 _CRT_SECURE_NO_WARNINGS RAD_FORCE_FEEDBACK)
else()
    message(FATAL_ERROR "Unsupported target platform")
endif()

if(MSVC)
    target_compile_options(srr2_options INTERFACE /permissive- /Zc:__cplusplus)
else()
    target_compile_options(srr2_options INTERFACE
        # __declspec and MSVC anonymous structs appear throughout the Radical
        # libraries; clang accepts both with these enabled.
        -fdeclspec
        -fms-extensions
        -fno-strict-aliasing)

    # This is 2003 code. Silence the categories it trips on wholesale rather
    # than editing thousands of call sites.
    target_compile_options(srr2_options INTERFACE
        -Wno-deprecated-declarations
        -Wno-writable-strings
        -Wno-invalid-offsetof
        -Wno-c++11-narrowing
        -Wno-unused-value
        -Wno-unused-variable
        -Wno-unused-private-field
        -Wno-parentheses
        -Wno-logical-op-parentheses
        -Wno-dangling-else
        -Wno-switch
        -Wno-format
        -Wno-reorder-ctor
        -Wno-overloaded-virtual
        -Wno-delete-non-abstract-non-virtual-dtor
        -Wno-microsoft-goto
        -Wno-inconsistent-missing-override)
endif()

# Helper that applies the house style to a static library: shared options, the
# _LIB define the sources expect, and the src/game include root that every
# Radical library reaches into for game-side headers.
function(srr2_add_library name)
    cmake_parse_arguments(ARG "" "" "SOURCES;INCLUDES;PUBLIC_INCLUDES;LINK" ${ARGN})

    add_library(${name} STATIC ${ARG_SOURCES})
    target_compile_definitions(${name} PRIVATE _LIB)
    target_link_libraries(${name} PUBLIC srr2_options ${ARG_LINK})
    target_include_directories(${name}
        PRIVATE ${ARG_INCLUDES}
        PUBLIC ${ARG_PUBLIC_INCLUDES} "${SRR2_GAME_DIR}")
endfunction()

set(SRR2_GAME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/game" CACHE INTERNAL "")
set(SRR2_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/libs" CACHE INTERNAL "")
