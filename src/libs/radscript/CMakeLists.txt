# Generated from radscript.vcxproj by tools/gen_cmake_sources.py.

set(radscript_SOURCES
    src/factory/radfactory.cpp
    src/namespace/namespace.cpp
    src/pch/pch.cpp
    src/script/remotescript.cpp
    src/script/script.cpp
    src/typeinfo/TypeInfoEnum.cpp
    src/typeinfo/TypeInfoIntLiteral.cpp
    src/typeinfo/TypeInfoParam.cpp
    src/typeinfo/typeinfoinstance.cpp
    src/typeinfo/typeinfointerface.cpp
    src/typeinfo/typeinfoloader.cpp
    src/typeinfo/typeinfomethod.cpp
    src/typeinfo/typeinfosystem.cpp
    src/typeinfo/win32/win32typeinfovfcall.cpp
    src/typeinfoutil/typeinfodistributor.cpp
    src/typeinfoutil/typeinfoutil.cpp)

srr2_add_library(radscript
    SOURCES ${radscript_SOURCES}
    PUBLIC_INCLUDES include
    INCLUDES src/pch
    LINK radcore
)
