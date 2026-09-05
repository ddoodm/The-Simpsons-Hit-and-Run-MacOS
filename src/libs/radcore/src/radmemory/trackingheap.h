#ifndef TRACKINGHEAP_H_
#define TRACKINGHEAP_H_
//-----------------------------------------------------------------------------
// Copyright (C) 2001 Radical Entertainment Ltd.  All rights reserved.
//
// staticheap.h
//
// Description: a heap that does nothing but track memory allocations
//
// Modification History:
//
//  + Created Mar 21, 2003 Ian Gipson
//-----------------------------------------------------------------------------

#include <radmemory.hpp>
#include <map>
#include <mutex>

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// StaticHeap
//-----------------------------------------------------------------------------
class TrackingHeap:
	public IRadMemoryHeap,
    public radRefCount
{
public:
    TrackingHeap();
    ~TrackingHeap();
    void   AddRef( void );
    bool   CanFreeMemory( void* pMemory );
    bool   CanFreeMemoryAligned( void* pMemory );
    void   FreeMemory( void* pMemory );
    void   FreeMemoryAligned( void* pMemory );
    void*  GetMemory ( unsigned int size );
    void*  GetMemoryAligned( unsigned int size, unsigned int align );
    void   GetMemoryObject( IRadMemoryObject** ppMemoryObject, unsigned int size );
	void   GetMemoryObjectAligned( IRadMemoryObject ** ppIRadMemoryObject, unsigned int size, unsigned int alignment );
    unsigned int GetSize();
    void   GetStatus(
		unsigned int* totalFreeMemory,
		unsigned int* largestBlock,
		unsigned int* numberOfObjects,
		unsigned int* highWaterMark );
    void   Release( void );
    void   SetSize( size_t size );
    void   TrackAllocations( bool trackAllocations );
    bool   ValidateHeap( void );
protected:
    void   RecordAllocation( void* address, size_t size );
    typedef std::map< void*, size_t > ADDRESS_SIZE_MAP;
    ADDRESS_SIZE_MAP m_Map;

    // The game replaces global operator new, so system frameworks allocate and
    // free through this heap too -- Apple's OpenGL driver releases its
    // resources from a Metal completion thread, concurrently with the game's
    // own allocations. Recursive because the allocation monitor can re-enter.
    std::recursive_mutex m_Mutex;

    bool   m_TrackAllocations;
    size_t m_TotalAllocated;
    size_t m_MaxSize;
    size_t m_HighWater;
    unsigned int m_NumberOfAllocations;
};

IRadMemoryHeap* radMemoryCreateTrackingHeap( unsigned int size,
	radMemoryAllocator allocator,
    const char * pName );

#endif // TRACKINGHEAP_H_

