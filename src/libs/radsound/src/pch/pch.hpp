//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#ifndef RADSOUND_PCH_HPP
#define RADSOUND_PCH_HPP

#if defined( RAD_WIN32 ) || defined( RAD_UWP )
#define _WIN32_WINNT 0x0602
#endif

//
// OpenAL header files
//
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>

//
// Standard C header files
//
#include <stdio.h>

#endif // RADSOUND_PCH_HPP
