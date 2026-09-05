# Generated from radmath.vcxproj by tools/gen_cmake_sources.py.

set(radmath_SOURCES
    radmath/matrix.cpp
    radmath/numerical.cpp
    radmath/plane.cpp
    radmath/quaternion.cpp
    radmath/radmath.cpp
    radmath/random.cpp
    radmath/spline.cpp
    radmath/triangle.cpp
    radmath/trig.cpp
    radmath/vector.cpp)

srr2_add_library(radmath
    SOURCES ${radmath_SOURCES}
    PUBLIC_INCLUDES . radmath
)
