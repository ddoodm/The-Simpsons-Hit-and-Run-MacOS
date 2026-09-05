//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#ifndef RADSCRIPT_PCH_HPP
#define RADSCRIPT_PCH_HPP

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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#endif // RADSCRIPT_PCH_HPP
