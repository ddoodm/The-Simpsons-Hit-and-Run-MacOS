//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#ifndef RADMOIVE_PCH_HPP
#define RADMOIVE_PCH_HPP

#if defined( RAD_WIN32 ) || defined( RAD_UWP )
//
// Microsoft header files. The DirectShow headers are left over from the
// original movie backend; playback now goes through FFmpeg.
//
#define _WIN32_WINNT 0x0602
#include <windows.h>

#include <strmif.h>
#include <control.h>
#include <uuids.h>
#include <evcode.h>
#include <vfwmsgs.h>
#endif

//
// Standard C header files
//

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

//
// FTech header files
//
#include "radoptions.hpp"

#endif // RADMOIVE_PCH_HPP
