# Generated from radcore.vcxproj by tools/gen_cmake_sources.py.

set(radcore_SOURCES
    src/pch/pch.cpp
    src/radcontroller/sdlcontroller.cpp
    src/raddebug/debug.cpp
    src/raddebugcommunication/host.cpp
    src/raddebugcommunication/hostchannel.cpp
    src/raddebugcommunication/targetx.cpp
    src/raddebugconsole/consoleclient.cpp
    src/raddebugfileserver/fileserver.cpp
    src/raddebugwatch/watchclient.cpp
    src/raddispatch/dispatcher.cpp
    src/radfile/common/cementer.cpp
    src/radfile/common/drive.cpp
    src/radfile/common/drivethread.cpp
    src/radfile/common/file.cpp
    src/radfile/common/filecache.cpp
    src/radfile/common/filesystem.cpp
    src/radfile/common/instancedrive.cpp
    src/radfile/common/platformdrives.cpp
    src/radfile/common/radfile.cpp
    src/radfile/common/remotedrive.cpp
    src/radfile/common/requests.cpp
    src/radfile/win32/win32drive.cpp
    src/radkey/radkey.cpp
    src/radmemory/align.cpp
    src/radmemory/binallocator.cpp
    src/radmemory/dlheap.cpp
    src/radmemory/externalmemoryheap.cpp
    src/radmemory/externalmemoryobject.cpp
    src/radmemory/memorymanager.cpp
    src/radmemory/memoryobject.cpp
    src/radmemory/memorypool.cpp
    src/radmemory/memoryspacewin32.cpp
    src/radmemory/platalloc.cpp
    src/radmemory/staticheap.cpp
    src/radmemory/trackingheap.cpp
    src/radmemory/twowayallocator.cpp
    src/radmemorymonitor/memmonitorclient.cpp
    src/radmemorymonitor/memmonitorclienthelp.cpp
    src/radobject/object.cpp
    src/radobjectbtree/objectavltree.cpp
    src/radobjectbtree/objectbtree.cpp
    src/radobjectlist/dynamicarray.cpp
    src/radobjectlist/objectlist.cpp
    src/radobjectlist/weakcallbackwrapper.cpp
    src/radobjectlist/weakinterfacewrapper.cpp
    src/radplatform/platform.cpp
    src/radprofiler/microprofile.cpp
    src/radprofiler/profiler.cpp
    src/radprofiler/profiler2.cpp
    src/radprofiler/profilesample.cpp
    src/radremotecommand/functionlist.cpp
    src/radremotecommand/remotecommandserver.cpp
    src/radremotecommand/remotecommandtarget.cpp
    src/radstats/simplestat.cpp
    src/radstats/statmanager.cpp
    src/radstring/string.cpp
    src/radtextdisplay/textdisplay.cpp
    src/radthread/mutex.cpp
    src/radthread/semaphore.cpp
    src/radthread/system.cpp
    src/radthread/thread.cpp
    src/radtime/stopwatch.cpp
    src/radtime/time.cpp)

if(WIN32)
    list(APPEND radcore_SOURCES
        src/radcontroller/directinputcontroller.cpp
        src/raddebugcommunication/targetconnection.cpp
        src/raddebugcommunication/targetsocketchannel.cpp
        src/radstacktrace/win32/stacktrace.cpp)
endif()

srr2_add_library(radcore
    SOURCES ${radcore_SOURCES}
    PUBLIC_INCLUDES include
    INCLUDES src
    LINK srr2::sdl2 srr2::gl
)
