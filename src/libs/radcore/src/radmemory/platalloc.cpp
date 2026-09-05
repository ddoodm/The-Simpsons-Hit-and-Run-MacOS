//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================



#include "pch.hpp"
#include <raddebug.hpp>
#include "platalloc.hpp"

#include <stdlib.h>

#if defined WIN32 || defined RAD_UWP

    #include <malloc.h>
    
    #if defined MALLOC_DEBUG
        #include <crtdbg.h>
    #endif

#endif

//============================================================================
// ::radMemoryPlatInitialize
//============================================================================
void radMemoryPlatInitialize( void )
{

}

//============================================================================
// ::radMemoryPlatTerminate
//============================================================================

void radMemoryPlatTerminate( void )
{
}

//============================================================================
// ::radMemoryPlatAlloc
//============================================================================

void * radMemoryPlatAlloc( unsigned int numberOfBytes )
{
    void * pMemory;
    //
    // C++ standard says you can allocate 0 byte memory object.
    //
    if ( numberOfBytes == 0 )
    {
        numberOfBytes = 1;
    }

    pMemory = malloc( numberOfBytes );
    
    rWarningMsg( pMemory != NULL, "radMemory: Platform (malloc) allocator failed to allocate memory\n" );
    return pMemory;
}

//============================================================================
// ::radMemoryPlatFree
//============================================================================

void radMemoryPlatFree( void * pMemory )
{
    free( pMemory );
}

//============================================================================
// ::radMemoryPlatAllocAligned
//============================================================================

void * radMemoryPlatAllocAligned( unsigned int numberOfBytes, unsigned int alignment )
{
	#ifndef WIN32

		// posix_memalign rather than aligned_alloc: callers pass sizes that are
		// not multiples of the alignment, which aligned_alloc rejects. It also
		// requires alignment to be at least a pointer width.
		if ( alignment < sizeof( void * ) )
		{
			alignment = sizeof( void * );
		}

		void * pMemory = NULL;
		if ( ::posix_memalign( &pMemory, alignment, numberOfBytes ) != 0 )
		{
			return NULL;
		}
		return pMemory;

	#else

        return _aligned_malloc( numberOfBytes, alignment );

	#endif
}

//============================================================================
// ::radMemoryPlatFreeAligned
//============================================================================

void radMemoryPlatFreeAligned( void * pAlignedMemory )
{

	#ifndef WIN32
		
		free( pAlignedMemory );
	
	#else

        _aligned_free( pAlignedMemory );

	#endif
}
