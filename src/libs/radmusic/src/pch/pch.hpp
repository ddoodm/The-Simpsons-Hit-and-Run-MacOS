#ifndef RADMUSIC_PCH_HPP
#define RADMUSIC_PCH_HPP

#if defined( RAD_WIN32 ) || defined( RAD_UWP )
//
// Microsoft header files
//
#define _WIN32_WINNT 0x0602
#include <windows.h>
#endif

//
// Standard C header files
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <radoptions.hpp>
#include <radfile.hpp>
#include <radmemory.hpp>
#include <radsound.hpp>

#endif // RADMUSIC_PCH_HPP
