include(${CMAKE_CURRENT_SOURCE_DIR}/sources.cmake)

add_executable(SRR2 ${SRR2_SOURCES})

target_include_directories(SRR2 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${SRR2_GAME_DIR}"
    "${SRR2_GAME_DIR}/main")

target_link_libraries(SRR2 PRIVATE
    srr2_options
    radcore
    radmath
    radcontent
    radscript
    radsound
    radmusic
    pure3d
    poser
    sim
    choreo
    scrooby
    radmovie
    srr2::sdl2
    srr2::gl)

# The old project force-included this header into every translation unit rather
# than having the sources include it themselves.
if(MSVC)
    target_compile_options(SRR2 PRIVATE /FIpchsrr2.h)
else()
    target_compile_options(SRR2 PRIVATE -include pchsrr2.h)
endif()

if(WIN32)
    target_link_libraries(SRR2 PRIVATE winmm dinput8 dxguid ws2_32)
endif()
