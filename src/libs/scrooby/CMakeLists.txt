# Generated from ScroobyLib.vcxproj by tools/gen_cmake_sources.py.

set(scrooby_SOURCES
    src/FeApp.cpp
    src/FeBoundedDrawable.cpp
    src/FeChunkHandler.cpp
    src/FeDrawable.cpp
    src/FeEntity.cpp
    src/FeGroup.cpp
    src/FeLanguage.cpp
    src/FeLayer.cpp
    src/FeLoaders.cpp
    src/FeMovie.cpp
    src/FeOwner.cpp
    src/FePage.cpp
    src/FeParent.cpp
    src/FePolygon.cpp
    src/FeProject.cpp
    src/FePure3dObject.cpp
    src/FeScreen.cpp
    src/FeSprite.cpp
    src/FeText.cpp
    src/FeTextBible.cpp
    src/FeTextStyle.cpp
    src/ResourceManager/FeResourceManager.cpp
    src/scrooby/App.cpp
    src/strings/UnicodeString.cpp
    src/strings/pcstring.cpp
    src/strings/stricmp.cpp
    src/tLinearTable.cpp
    src/utility/AsyncFileLoader.cpp
    src/utility/BufferReader.cpp
    src/utility/EnumConversion.cpp
    src/utility/debugMessages.cpp
    src/xml/XMLParser.cpp
    src/xml/XMLTree.cpp)

srr2_add_library(scrooby
    SOURCES ${scrooby_SOURCES}
    PUBLIC_INCLUDES include src
    LINK radmath pure3d radcore radcontent
)

target_compile_definitions(scrooby PUBLIC SCROOBY_RUNTIME)
