//=============================================================================
// Copyright (c) 2025 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        sdlcontroller.cpp
//
// Subsystem:	Foundation Technologies - Controller System
//
// Description:	This file contains the implementation of the Foundation 
//              Technologies sdl contoller
//
// Date:    	
//
//=============================================================================

//============================================================================
// Include Files
//============================================================================

#include "pch.hpp"
#if defined(RAD_UWP) || defined(RAD_MACOS)
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <radobject.hpp>
#include <radcontroller.hpp>
#include <raddebug.hpp>
#include <radstring.hpp>
#include <radobjectlist.hpp>
#include <radtime.hpp>
#include <radmemorymonitor.hpp>
#include "radcontrollerbuffer.hpp"

#include <SDL2/SDL.h>
#include <raddinputcodes.hpp>

//============================================================================
// Internal Interfaces
//============================================================================

struct IRadControllerInputPointSDL
    :
    public IRadControllerInputPoint
{
	virtual void iInitialize( void ) = 0;
    virtual void iVirtualTimeReMapped( unsigned int virtualTime ) = 0;
    virtual void iVirtualTimeChanged( unsigned int virtualTime ) = 0;
};

struct IRadControllerSDL
    :
    public IRadController
{
    virtual void iPoll( unsigned int virtualTime ) = 0;
    virtual void iVirtualTimeReMapped( unsigned int virtualTime ) = 0;
    virtual void iVirtualTimeChanged( unsigned int virtualTime ) = 0;
    virtual void iSetBufferTime( unsigned int milliseconds, unsigned int pollingRate ) = 0;
};

//============================================================================
// Globals
//============================================================================

struct SDLInputPoint
{
    const char * m_pType;
    const char * m_pName;
    unsigned int m_Mask;
};

static const char * g_Sdlipt[] =
{
    "Button",
    "AnalogButton",
    "XAxis",
    "YAxis",
    "RelXAxis",
    "RelYAxis",
    "RelZAxis"
};

//
// Which kind of device an input point belongs to. Pads read through
// SDL_GameController; keyboard and mouse read global SDL state instead.
//

enum SDLDeviceKind
{
    SDLDevice_Pad,
    SDLDevice_Keyboard,
    SDLDevice_Mouse
};

static SDLInputPoint g_SDLPoints[] =
{
    { g_Sdlipt[ 0 ], "DPadUp",           SDL_CONTROLLER_BUTTON_DPAD_UP },
    { g_Sdlipt[ 0 ], "DPadDown",         SDL_CONTROLLER_BUTTON_DPAD_DOWN },
    { g_Sdlipt[ 0 ], "DPadLeft",         SDL_CONTROLLER_BUTTON_DPAD_LEFT },
    { g_Sdlipt[ 0 ], "DPadRight",        SDL_CONTROLLER_BUTTON_DPAD_RIGHT },
    { g_Sdlipt[ 0 ], "Start",            SDL_CONTROLLER_BUTTON_START },
    { g_Sdlipt[ 0 ], "Back",             SDL_CONTROLLER_BUTTON_BACK },
    { g_Sdlipt[ 0 ], "LeftThumb",        SDL_CONTROLLER_BUTTON_LEFTSTICK },
    { g_Sdlipt[ 0 ], "RightThumb",       SDL_CONTROLLER_BUTTON_RIGHTSTICK },
#ifdef __SWITCH__
    { g_Sdlipt[ 0 ], "A",                SDL_CONTROLLER_BUTTON_B },
    { g_Sdlipt[ 0 ], "B",                SDL_CONTROLLER_BUTTON_A },
    { g_Sdlipt[ 0 ], "X",                SDL_CONTROLLER_BUTTON_Y },
    { g_Sdlipt[ 0 ], "Y",                SDL_CONTROLLER_BUTTON_X },
#else
    { g_Sdlipt[ 0 ], "A",                SDL_CONTROLLER_BUTTON_A },
    { g_Sdlipt[ 0 ], "B",                SDL_CONTROLLER_BUTTON_B },
    { g_Sdlipt[ 0 ], "X",                SDL_CONTROLLER_BUTTON_X },
    { g_Sdlipt[ 0 ], "Y",                SDL_CONTROLLER_BUTTON_Y },
#endif
    { g_Sdlipt[ 0 ], "Black",            SDL_CONTROLLER_BUTTON_LEFTSHOULDER },
    { g_Sdlipt[ 0 ], "White",            SDL_CONTROLLER_BUTTON_RIGHTSHOULDER },
    { g_Sdlipt[ 1 ], "LeftTrigger",      SDL_CONTROLLER_AXIS_TRIGGERLEFT },
    { g_Sdlipt[ 1 ], "RightTrigger",     SDL_CONTROLLER_AXIS_TRIGGERRIGHT },
    { g_Sdlipt[ 2 ], "LeftStickX",       SDL_CONTROLLER_AXIS_LEFTX },
    { g_Sdlipt[ 3 ], "LeftStickY",       SDL_CONTROLLER_AXIS_LEFTY },
    { g_Sdlipt[ 2 ], "RightStickX",      SDL_CONTROLLER_AXIS_RIGHTX },
    { g_Sdlipt[ 3 ], "RightStickY",      SDL_CONTROLLER_AXIS_RIGHTY }
};

//
// The input layer names keys by DirectInput scancode, so the keyboard device
// publishes one input point per DIK_* code and reads it from the matching SDL
// scancode. m_Mask holds the SDL scancode; m_pName the DIK code, since the
// point's index in this table is what VirtualKeyToIndex resolves to.
//

struct SDLKeyPoint
{
    int          m_DIK;
    unsigned int m_Scancode;
};

static const SDLKeyPoint g_SDLKeyPoints[] =
{
    { DIK_ESCAPE,       SDL_SCANCODE_ESCAPE },
    { DIK_1,            SDL_SCANCODE_1 },
    { DIK_2,            SDL_SCANCODE_2 },
    { DIK_3,            SDL_SCANCODE_3 },
    { DIK_4,            SDL_SCANCODE_4 },
    { DIK_5,            SDL_SCANCODE_5 },
    { DIK_6,            SDL_SCANCODE_6 },
    { DIK_7,            SDL_SCANCODE_7 },
    { DIK_8,            SDL_SCANCODE_8 },
    { DIK_9,            SDL_SCANCODE_9 },
    { DIK_0,            SDL_SCANCODE_0 },
    { DIK_MINUS,        SDL_SCANCODE_MINUS },
    { DIK_EQUALS,       SDL_SCANCODE_EQUALS },
    { DIK_BACK,         SDL_SCANCODE_BACKSPACE },
    { DIK_TAB,          SDL_SCANCODE_TAB },
    { DIK_Q,            SDL_SCANCODE_Q },
    { DIK_W,            SDL_SCANCODE_W },
    { DIK_E,            SDL_SCANCODE_E },
    { DIK_R,            SDL_SCANCODE_R },
    { DIK_T,            SDL_SCANCODE_T },
    { DIK_Y,            SDL_SCANCODE_Y },
    { DIK_U,            SDL_SCANCODE_U },
    { DIK_I,            SDL_SCANCODE_I },
    { DIK_O,            SDL_SCANCODE_O },
    { DIK_P,            SDL_SCANCODE_P },
    { DIK_LBRACKET,     SDL_SCANCODE_LEFTBRACKET },
    { DIK_RBRACKET,     SDL_SCANCODE_RIGHTBRACKET },
    { DIK_RETURN,       SDL_SCANCODE_RETURN },
    { DIK_LCONTROL,     SDL_SCANCODE_LCTRL },
    { DIK_A,            SDL_SCANCODE_A },
    { DIK_S,            SDL_SCANCODE_S },
    { DIK_D,            SDL_SCANCODE_D },
    { DIK_F,            SDL_SCANCODE_F },
    { DIK_G,            SDL_SCANCODE_G },
    { DIK_H,            SDL_SCANCODE_H },
    { DIK_J,            SDL_SCANCODE_J },
    { DIK_K,            SDL_SCANCODE_K },
    { DIK_L,            SDL_SCANCODE_L },
    { DIK_SEMICOLON,    SDL_SCANCODE_SEMICOLON },
    { DIK_APOSTROPHE,   SDL_SCANCODE_APOSTROPHE },
    { DIK_GRAVE,        SDL_SCANCODE_GRAVE },
    { DIK_LSHIFT,       SDL_SCANCODE_LSHIFT },
    { DIK_BACKSLASH,    SDL_SCANCODE_BACKSLASH },
    { DIK_Z,            SDL_SCANCODE_Z },
    { DIK_X,            SDL_SCANCODE_X },
    { DIK_C,            SDL_SCANCODE_C },
    { DIK_V,            SDL_SCANCODE_V },
    { DIK_B,            SDL_SCANCODE_B },
    { DIK_N,            SDL_SCANCODE_N },
    { DIK_M,            SDL_SCANCODE_M },
    { DIK_COMMA,        SDL_SCANCODE_COMMA },
    { DIK_PERIOD,       SDL_SCANCODE_PERIOD },
    { DIK_SLASH,        SDL_SCANCODE_SLASH },
    { DIK_RSHIFT,       SDL_SCANCODE_RSHIFT },
    { DIK_MULTIPLY,     SDL_SCANCODE_KP_MULTIPLY },
    { DIK_LMENU,        SDL_SCANCODE_LALT },
    { DIK_SPACE,        SDL_SCANCODE_SPACE },
    { DIK_CAPITAL,      SDL_SCANCODE_CAPSLOCK },
    { DIK_F1,           SDL_SCANCODE_F1 },
    { DIK_F2,           SDL_SCANCODE_F2 },
    { DIK_F3,           SDL_SCANCODE_F3 },
    { DIK_F4,           SDL_SCANCODE_F4 },
    { DIK_F5,           SDL_SCANCODE_F5 },
    { DIK_F6,           SDL_SCANCODE_F6 },
    { DIK_F7,           SDL_SCANCODE_F7 },
    { DIK_F8,           SDL_SCANCODE_F8 },
    { DIK_F9,           SDL_SCANCODE_F9 },
    { DIK_F10,          SDL_SCANCODE_F10 },
    { DIK_NUMLOCK,      SDL_SCANCODE_NUMLOCKCLEAR },
    { DIK_SCROLL,       SDL_SCANCODE_SCROLLLOCK },
    { DIK_NUMPAD7,      SDL_SCANCODE_KP_7 },
    { DIK_NUMPAD8,      SDL_SCANCODE_KP_8 },
    { DIK_NUMPAD9,      SDL_SCANCODE_KP_9 },
    { DIK_SUBTRACT,     SDL_SCANCODE_KP_MINUS },
    { DIK_NUMPAD4,      SDL_SCANCODE_KP_4 },
    { DIK_NUMPAD5,      SDL_SCANCODE_KP_5 },
    { DIK_NUMPAD6,      SDL_SCANCODE_KP_6 },
    { DIK_ADD,          SDL_SCANCODE_KP_PLUS },
    { DIK_NUMPAD1,      SDL_SCANCODE_KP_1 },
    { DIK_NUMPAD2,      SDL_SCANCODE_KP_2 },
    { DIK_NUMPAD3,      SDL_SCANCODE_KP_3 },
    { DIK_NUMPAD0,      SDL_SCANCODE_KP_0 },
    { DIK_DECIMAL,      SDL_SCANCODE_KP_PERIOD },
    { DIK_OEM_102,      SDL_SCANCODE_NONUSBACKSLASH },
    { DIK_F11,          SDL_SCANCODE_F11 },
    { DIK_F12,          SDL_SCANCODE_F12 },
    { DIK_F13,          SDL_SCANCODE_F13 },
    { DIK_F14,          SDL_SCANCODE_F14 },
    { DIK_F15,          SDL_SCANCODE_F15 },
    { DIK_NUMPADEQUALS, SDL_SCANCODE_KP_EQUALS },
    { DIK_PREVTRACK,    SDL_SCANCODE_AUDIOPREV },
    { DIK_STOP,         SDL_SCANCODE_STOP },
    { DIK_NEXTTRACK,    SDL_SCANCODE_AUDIONEXT },
    { DIK_NUMPADENTER,  SDL_SCANCODE_KP_ENTER },
    { DIK_RCONTROL,     SDL_SCANCODE_RCTRL },
    { DIK_MUTE,         SDL_SCANCODE_AUDIOMUTE },
    { DIK_CALCULATOR,   SDL_SCANCODE_CALCULATOR },
    { DIK_PLAYPAUSE,    SDL_SCANCODE_AUDIOPLAY },
    { DIK_MEDIASTOP,    SDL_SCANCODE_AUDIOSTOP },
    { DIK_VOLUMEDOWN,   SDL_SCANCODE_VOLUMEDOWN },
    { DIK_VOLUMEUP,     SDL_SCANCODE_VOLUMEUP },
    { DIK_WEBHOME,      SDL_SCANCODE_AC_HOME },
    { DIK_NUMPADCOMMA,  SDL_SCANCODE_KP_COMMA },
    { DIK_DIVIDE,       SDL_SCANCODE_KP_DIVIDE },
    { DIK_SYSRQ,        SDL_SCANCODE_PRINTSCREEN },
    { DIK_RMENU,        SDL_SCANCODE_RALT },
    { DIK_PAUSE,        SDL_SCANCODE_PAUSE },
    { DIK_HOME,         SDL_SCANCODE_HOME },
    { DIK_UP,           SDL_SCANCODE_UP },
    { DIK_PRIOR,        SDL_SCANCODE_PAGEUP },
    { DIK_LEFT,         SDL_SCANCODE_LEFT },
    { DIK_RIGHT,        SDL_SCANCODE_RIGHT },
    { DIK_END,          SDL_SCANCODE_END },
    { DIK_DOWN,         SDL_SCANCODE_DOWN },
    { DIK_NEXT,         SDL_SCANCODE_PAGEDOWN },
    { DIK_INSERT,       SDL_SCANCODE_INSERT },
    { DIK_DELETE,       SDL_SCANCODE_DELETE },
    { DIK_LWIN,         SDL_SCANCODE_LGUI },
    { DIK_RWIN,         SDL_SCANCODE_RGUI },
    { DIK_APPS,         SDL_SCANCODE_APPLICATION },
    { DIK_POWER,        SDL_SCANCODE_POWER },
    { DIK_WEBSEARCH,    SDL_SCANCODE_AC_SEARCH },
    { DIK_WEBFAVORITES, SDL_SCANCODE_AC_BOOKMARKS },
    { DIK_WEBREFRESH,   SDL_SCANCODE_AC_REFRESH },
    { DIK_WEBSTOP,      SDL_SCANCODE_AC_STOP },
    { DIK_WEBFORWARD,   SDL_SCANCODE_AC_FORWARD },
    { DIK_WEBBACK,      SDL_SCANCODE_AC_BACK },
    { DIK_MYCOMPUTER,   SDL_SCANCODE_COMPUTER },
    { DIK_MAIL,         SDL_SCANCODE_MAIL },
    { DIK_MEDIASELECT,  SDL_SCANCODE_MEDIASELECT }
};

//
// Mouse points are ordered to match DIMOUSESTATE2's button order (left,
// right, middle, then extras), because Mouse::MapInputToDICode() turns
// "Button <n>" straight into DIMOFS_BUTTON<n>.
//

static SDLInputPoint g_SDLMousePoints[] =
{
    // Axis m_Mask indexes s_MouseDelta; button m_Mask is the SDL button mask.
    { g_Sdlipt[ 4 ], "X Axis",   0 },
    { g_Sdlipt[ 5 ], "Y Axis",   1 },
    { g_Sdlipt[ 6 ], "Wheel",    2 },
    { g_Sdlipt[ 0 ], "Button 0", SDL_BUTTON_LMASK },
    { g_Sdlipt[ 0 ], "Button 1", SDL_BUTTON_RMASK },
    { g_Sdlipt[ 0 ], "Button 2", SDL_BUTTON_MMASK },
    { g_Sdlipt[ 0 ], "Button 3", SDL_BUTTON_X1MASK },
    { g_Sdlipt[ 0 ], "Button 4", SDL_BUTTON_X2MASK }
};

//
// Relative mouse motion has to be accumulated as it arrives, since polling it
// with SDL_GetRelativeMouseState() would consume the delta for whichever axis
// asked first. Each axis takes and clears its own total once per poll.
//

static float s_MouseDelta[ 3 ] = { 0.0f, 0.0f, 0.0f };

static int SDLWatchMouseMotion( void * userdata, SDL_Event * event )
{
    if ( event->type == SDL_MOUSEMOTION )
    {
        s_MouseDelta[ 0 ] += (float) event->motion.xrel;
        s_MouseDelta[ 1 ] += (float) event->motion.yrel;
    }
    else if ( event->type == SDL_MOUSEWHEEL )
    {
        s_MouseDelta[ 2 ] += (float) event->wheel.y;
    }

    return 1;
}

//
// Names keys by DirectInput scancode for the game's input layer. Index is
// DIK code, value is the keyboard device's input point index, or -1.
//

static int    s_VirtualKeyToIndex[ 256 ];
const int *   VirtualKeyToIndex    = &s_VirtualKeyToIndex[ -1 ]; // DIK_* starts at 1 not 0
const int *   VirtualJoyKeyToIndex = &s_VirtualKeyToIndex[ -48 ]; // DIJOFS_BUTTON(0) is 48

static class radControllerSystemSDL* s_pTheSDLControllerSystem2 = NULL;
static radMemoryAllocator g_ControllerSystemAllocator = RADMEMORY_ALLOC_DEFAULT;

//============================================================================
// Component: radControllerOutputPointSDL
//============================================================================

class radControllerOutputPointSDL
    :
    public IRadControllerOutputPoint,
    public radRefCount
{
    public:

    IMPLEMENT_REFCOUNTED( "radControllerOutputPointSDL" )

    //========================================================================
    // radControllerOutputPointSDL::rSDLControllerOutputPoint
    //========================================================================

    radControllerOutputPointSDL( const char * pName )
        :
        radRefCount( 0 ),
        m_pName( pName ),
        m_Gain( 0.0f )
    {
        radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radControllerOutputPointSDL" );
    }

    //========================================================================
    // radControllerOutputPointSDL::~rSDLControllerOutputPoint
    //========================================================================

    ~radControllerOutputPointSDL( void )
    {
    }

    //========================================================================
    // radControllerOutputPointSDL::GetName
    //========================================================================

    virtual const char * GetName( void )
    {
        return m_pName;
    }

    //========================================================================
    // radControllerOutputPointSDL::GetType
    //========================================================================

    virtual const char * GetType( void )
    {
        return "Analog";
    }

    //========================================================================
    // radControllerOutputPointSDL::GetGain
    //========================================================================

    virtual float GetGain( void )
    {
        return m_Gain;
    }

    //========================================================================
    // radControllerOutputPointSDL::SetGain
    //========================================================================

    virtual void SetGain( float value )
    {
        if ( value < 0.0f )
        {
            value = 0.0f;
        }
        else if ( value > 1.0f )
        {
            value = 1.0f;
        }

        m_Gain = value;
    }

    // FIXME
    long GetOffset() const { return 0; }
    void UpdateEffect(const SDL_HapticEffect*) {}
    void Start() {}
    void Stop() {}
    void ReleaseEffect() {}

    //========================================================================
    // radControllerOutputPointSDL Data Members
    //========================================================================

    const char * m_pName;
    float m_Gain;
};

//============================================================================
// Component: radControllerInputPointSDL
//============================================================================

class radControllerInputPointSDL
    :
    public IRadControllerInputPointSDL,
    public radRefCount
{
    public:

    IMPLEMENT_REFCOUNTED( "radControllerInputPointSDL" )

    //========================================================================
    // radControllerInputPointSDL::iIVirtualTimeReMapped
    //========================================================================

    virtual void iVirtualTimeReMapped( unsigned int virtualTime )
    {
        //
        // The client has done a re-sync to game time, all we can do is
        // set our changed-state time to "now".
        //
        m_TimeInState = 0;
        m_TimeOfStateChange = virtualTime;
    }

    //========================================================================
    // radControllerInputPointSDL::CalculateNewValue
    //========================================================================

	float CalculateNewValue( void )
	{
        //
        // Calculate the current value of the input point according to the
        // data structure passed in.  We get initialized with the offset
        // into this data array to get our data.  Knowing this and our type
        // we can determine our new floating point value.
        //

        float newValue = 0.0f;

        if ( m_DeviceKind == SDLDevice_Keyboard )
        {
            const Uint8 * pKeys = SDL_GetKeyboardState( NULL );

            return ( pKeys != NULL && pKeys[ m_Identifier ] ) ? 1.0f : 0.0f;
        }

        if ( m_DeviceKind == SDLDevice_Mouse )
        {
            if ( m_pType == g_Sdlipt[ 0 ] ) // Button
            {
                return ( SDL_GetMouseState( NULL, NULL ) & m_Identifier ) ? 1.0f : 0.0f;
            }

            //
            // Relative axis. Take the accumulated delta and clear it, so the
            // next poll reports only motion that happened since this one.
            //

            newValue = s_MouseDelta[ m_Identifier ];
            s_MouseDelta[ m_Identifier ] = 0.0f;

            return newValue;
        }

        if ( m_pController != NULL )
        {
            if ( m_pType == g_Sdlipt[ 0 ] ) // Button
            {
                newValue = SDL_GameControllerGetButton( m_pController, (SDL_GameControllerButton)m_Identifier ) ? 1.0f : 0.0f;
            }
            else if ( m_pType == g_Sdlipt[ 1 ] ) // Analog Button
            {
                newValue = SDL_GameControllerGetAxis( m_pController, (SDL_GameControllerAxis)m_Identifier );
                newValue /= 32767.0f;
            }
            else if ( ( m_pType == g_Sdlipt[ 2 ] ) || ( m_pType == g_Sdlipt[ 3 ] ) ) // X/Y Axis
            {
                newValue = SDL_GameControllerGetAxis( m_pController, (SDL_GameControllerAxis)m_Identifier );
                if ( newValue > 0.0f )
                {
                    newValue /= 65534.0f;
                }
                else
                {
                    newValue /= 65536.0f;
                }

                newValue += 0.5f;

                if ( m_Identifier == SDL_CONTROLLER_AXIS_LEFTY || m_Identifier == SDL_CONTROLLER_AXIS_RIGHTY )
                {
                    newValue = 1.0f - newValue;
                }
            }
            else
            {
                rAssert( 0 );
            }
        }

		return newValue;
	}

    //========================================================================
    // radControllerInputPointSDL::iIVirtualTimeChanged
    //========================================================================

    virtual void iVirtualTimeChanged( unsigned int virtualTime )
    {
		// Get a new value from the pData structure

		float newValue = CalculateNewValue( );
        
        //
        // Check tolerance
        //

        if
        (
            ( newValue != m_Value ) && 
            ( fabsf( newValue - m_Value ) >= m_Tolerance )
        )
        {
            //
            // The input point has changed and we are in tolerance.
            //

            m_Value = newValue;

            m_TimeOfStateChange = virtualTime;
            m_TimeInState = 0; // Just changed

            //
            // Notify callbacks
            //

            AddRef( ); // Don't want to self destruct while we're calling out

            IRadWeakCallbackWrapper * pIWcr;

            m_xIOl_Callbacks->Reset( );

            if ((pIWcr = reinterpret_cast< IRadWeakCallbackWrapper * >( m_xIOl_Callbacks->GetNext( ) )))
            {
                IRadControllerInputPointCallback * pCallback = ( IRadControllerInputPointCallback* ) pIWcr->GetWeakInterface( );
                unsigned int userData = reinterpret_cast< uintptr_t >( pIWcr->GetUserData( ) );

                pCallback->OnControllerInputPointChange( userData, m_Value );           
            }

            Release( );
        }
        else
        {
            //
            // This input point has not changed value, or failed the tolerance
            // test, so count time in state.
            //

            m_TimeInState = virtualTime - m_TimeOfStateChange;
        }
    }

    //========================================================================
    // radControllerInputPointSDL::iInitialize
    //========================================================================

	virtual void iInitialize( void )
	{
		// Set the new value

		m_Value = CalculateNewValue( );
	}

    //========================================================================
    // radControllerInputPointSDL::GetName
    //========================================================================

    virtual const char * GetName( void )
    {
        //
        // This points to a string in the global controller definition array
        // (see top of file)
        //

        return m_pName;
    }
    
    //========================================================================
    // radControllerInputPointSDL::GetType
    //========================================================================
    
    virtual const char * GetType( void )
    {
        //
        // This points to a string in the global controller definition array
        // (see top of file)
        //

        return m_pType;      
    }

    //========================================================================
    // radControllerInputPointSDL::SetTolerance
    //========================================================================

    virtual void  SetTolerance( float percentage )
    {
        //
        // Set tolerance in range.
        //

        rAssert( percentage >= 0.0f && percentage <= 1.0f );

        if ( percentage < 0.0f )
        {
            percentage = 0.0f;
        }
        else if ( percentage > 1.0f )
        {
            percentage = 1.0f;
        }

        m_Tolerance = percentage;
    }

    //========================================================================
    // radControllerInputPointSDL::GetTolerance
    //========================================================================

    virtual float /* percentage */ GetTolerance( void )
    {
        //
        // Simply retur the tolerance
        // 

        return m_Tolerance;
    }

    //========================================================================
    // radControllerInputPointSDL::RegisterControllerInputPointCallback
    //========================================================================
           
    virtual void RegisterControllerInputPointCallback
    (
        IRadControllerInputPointCallback * pCallback,
        unsigned int userData = 0
    )
    {
        //
        // Wrap the weak interface we a callback wrapper so we can store it
        // in our object list.
        //

        rAssert( pCallback != NULL );

        radRef< IRadWeakCallbackWrapper > xIWcr;

        radWeakCallbackWrapperCreate( &xIWcr, g_ControllerSystemAllocator ); 

        rAssert( xIWcr != NULL );

        if ( xIWcr != NULL )
        {
            xIWcr->SetWeakInterface( pCallback );
            xIWcr->SetUserData( (void*)(uintptr_t) userData );
        }

        m_xIOl_Callbacks->AddObject( xIWcr );
    }

    //========================================================================
    // radControllerInputPointSDL::UnRegisterControllerInputPointCallback
    //========================================================================

    virtual void UnRegisterControllerInputPointCallback
    (
        IRadControllerInputPointCallback * pCallback
    )
    {
        //
        // Simply look for the callback in the list and delete it, if found.
        //
        rAssert( pCallback != NULL );

        IRadWeakCallbackWrapper * pIWcr;

        m_xIOl_Callbacks->Reset( );

        if ((pIWcr = reinterpret_cast< IRadWeakCallbackWrapper * >( m_xIOl_Callbacks->GetNext( ) )))
        {
            if ( pIWcr->GetWeakInterface( ) == pCallback )
            {
                m_xIOl_Callbacks->RemoveObject( pIWcr );
                return;
            }
        }

        rAssertMsg( false, "Controller Input Point Callback Not Registered." );
    }

    //========================================================================
    // radControllerInputPointSDL::GetCurrentValue
    //========================================================================

    virtual float GetCurrentValue( unsigned int * pTime = NULL )
    {
        if ( pTime != NULL )
        {
            *pTime = m_TimeInState;
        }                
        
        //
        // we map it to the client's range here, this solves a bunch of
        // problems with the client remapping in mid-game.
        //

        return (( m_MaxRange - m_MinRange ) * m_Value ) + m_MinRange;
    }

    //========================================================================
    // radControllerInputPointSDL::SetRange
    //========================================================================

    virtual void  SetRange( float min, float max )
    {
        //
        // Note that max range CAN be less than min range, if this is what
        // the client wants.
        //

        m_MinRange = min;
        m_MaxRange = max;
    }

    //========================================================================
    // radControllerInputPointSDL::GetRange
    //========================================================================
    
    virtual void GetRange( float * pMin, float * pMax )
    {
        //
        // Either param can be null, but not both!
        //

        rAssert( pMin != NULL || pMax != NULL );

        if ( pMin != NULL )
        {
            *pMin = m_MinRange;
        }

        if ( pMax != NULL )
        {
            *pMax = m_MaxRange;
        }
    }

    //========================================================================
    // radControllerInputPointSDL::radControllerInputPointSDL
    //========================================================================

    radControllerInputPointSDL
    (
        SDL_GameController * pController,
        const char * pType,
        const char * pName,
        int id,
        SDLDeviceKind deviceKind = SDLDevice_Pad
    )
        :
        radRefCount( 0 ),
        m_Value( 0.0f ),
        m_MinRange( 0.0f ),
        m_MaxRange( 1.0f ),
        m_Tolerance( 0.0f ),
        m_TimeInState( 0 ),
        m_TimeOfStateChange( 0 ),
        m_pType( pType ),
        m_pName( pName ),
        m_Identifier( id ),
        m_pController( pController ),
        m_DeviceKind( deviceKind )
    {
        radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radControllerInputPointSDL" );
        
        //
        // Object list to store our callbacks
        //

        ::radObjectListCreate( & m_xIOl_Callbacks, g_ControllerSystemAllocator );
    }

    //========================================================================
    // radControllerInputPointSDL::~radControllerInputPointSDL
    //========================================================================
    
    ~radControllerInputPointSDL( void )
    {        
        rAssertMsg( m_xIOl_Callbacks->GetSize() == 0, "Sombody forgot to UnRegister an input point callback" );
    }

    //========================================================================
    // radControllerInputPointSDL Data Members
    //========================================================================

    float m_Value;
    float m_MinRange;
    float m_MaxRange;
    float m_Tolerance;

    unsigned int m_TimeInState;
    unsigned int m_TimeOfStateChange;

    const char * m_pType;
    const char * m_pName;

    int m_Identifier;
    SDL_GameController * m_pController;
    SDLDeviceKind m_DeviceKind;

    radRef< IRadObjectList > m_xIOl_Callbacks;
};

//============================================================================
// Component: radControllerSDL
//============================================================================

class radControllerSDL
    :
    public IRadControllerSDL,
    public radRefCount
{
    public:

    IMPLEMENT_REFCOUNTED( "radControllerSDL" )

    //========================================================================
    // radControllerSDL::iPoll
    //========================================================================

    virtual void iPoll( unsigned int virtualTime )
    {
        //
        // Query the hardware for current state and store it in the
        // controller buffer, it will be pulled out by virtual time
        // changing.
        //

        if ( GetRefCount( ) > 1 )
        {
            if ( m_pController != NULL )
            {
                SDL_GameControllerUpdate();
            }

            //
            // Send our output point data to the device here
            //

            if ( m_DeviceKind == SDLDevice_Pad )
            {                
                IRadControllerOutputPoint * pICop2_Left  = reinterpret_cast< IRadControllerOutputPoint * >( m_xIOl_OutputPoints->GetAt( 0 ) );
                IRadControllerOutputPoint * pICop2_Right = reinterpret_cast< IRadControllerOutputPoint * >( m_xIOl_OutputPoints->GetAt( 1 ) );

                uint16_t newLeftGain  = (uint16_t) ( pICop2_Left->GetGain( ) * 65535.0f );
                uint16_t newRightGain = (uint16_t) ( pICop2_Right->GetGain( ) * 65535.0f );
        
                if
                (
                    ( newLeftGain  != m_LeftGain ) ||
                    ( newRightGain != m_RightGain )
                )
                {
                    m_LeftGain =  newLeftGain;
                    m_RightGain = newRightGain;

					rAssert(m_pController != NULL);

                    int result = 0;
					if(m_pController != NULL)
					{
                        result = SDL_GameControllerRumble( m_pController,
                            m_LeftGain, m_RightGain, 0 );
					}

                    //
                    // Old Controllers don't support output and this will
                    // fail
                    //

                    // rAssert( result == ERROR_IO_PENDING );
                }
            }
        }                  
    }

    //========================================================================
    // radControllerSDL::iVirtualTimeReMapped
    //========================================================================

    virtual void iVirtualTimeReMapped( unsigned int virtualTime )
    {
        IRadControllerInputPointSDL * pICip2;

        m_xIOl_InputPoints->Reset( );

        while ((pICip2 = reinterpret_cast< IRadControllerInputPointSDL * >( m_xIOl_InputPoints->GetNext( ) )))
        {
            pICip2->iVirtualTimeReMapped( virtualTime );
        }
    }

    //========================================================================
    // radControllerSDL::iVirtualTimeChanged
    //========================================================================

    virtual void iVirtualTimeChanged( unsigned int virtualTime )
    {
        if( GetRefCount( ) > 1 )
        {
            //
            // Regardless of whether anything happened, notify all the input
            // points that time has passed
            //

            if( m_xIOl_InputPoints != NULL )
            {
                m_xIOl_InputPoints->Reset();

                IRadControllerInputPointSDL* pIXbcip2;

                while((pIXbcip2 = reinterpret_cast<IRadControllerInputPointSDL*>(m_xIOl_InputPoints->GetNext())))
                {
                    pIXbcip2->iVirtualTimeChanged( virtualTime );
                }
            }
        }
    }

    //========================================================================
    // radControllerSDL::iSetBufferTime
    //========================================================================

    virtual void iSetBufferTime
    (
        unsigned int milliseconds,
        unsigned int pollingRate
    )
    {
    }

    //========================================================================
    // radControllerSDL::IsConnection
    //========================================================================

    virtual bool IsConnected( void )
    {
        if ( m_DeviceKind != SDLDevice_Pad )
        {
            return true;
        }

        return SDL_GameControllerGetAttached( m_pController ) == SDL_TRUE;
    }

    //========================================================================
    // radControllerSDL::GetType
    //========================================================================

    virtual const char * GetType( void )
    {
        switch ( m_DeviceKind )
        {
            case SDLDevice_Keyboard: return "SDLKeyboard";
            case SDLDevice_Mouse:    return "SDLMouse";
            default:                 return "SDLStandard";
        }
    }
    
    //========================================================================
    // radControllerSDL::GetClassification
    //========================================================================

    virtual const char * GetClassification( void )
    {
        //
        // The game looks devices up by this string, so it has to match the
        // names the DirectInput backend reports.
        //

        switch ( m_DeviceKind )
        {
            case SDLDevice_Keyboard: return "Keyboard";
            case SDLDevice_Mouse:    return "Mouse";
            default:                 return "Joystick";
        }
    }

    //========================================================================
    // radControllerSDL::GetNumberOfInputPointsOfType
    //========================================================================

    virtual unsigned int GetNumberOfInputPointsOfType
    (
        const char * pType
    )
    {
        //
        // Count up the number of input points of this time in the input
        // point list
        //

        rAssert( pType != NULL );

        unsigned int count = 0;

        m_xIOl_InputPoints->Reset( );

        IRadControllerInputPoint * pICip2;

        while ((pICip2 = reinterpret_cast< IRadControllerInputPointSDL * >( m_xIOl_InputPoints->GetNext( ) )))
        {
            if ( strcmp( pICip2->GetType( ), pType ) == 0 )
            {
                count++;
            }        
        }

        return count;
    }

    //========================================================================
    // radControllerSDL::GetNumberOfOutputPointsOfType
    //========================================================================

    unsigned int GetNumberOfOutputPointsOfType( const char * pType )
    {
        //
        // Count up the number of Output points of this time in the Output
        // point list
        //

        rAssert( pType != NULL );

        unsigned int count = 0;

        m_xIOl_OutputPoints->Reset( );

        IRadControllerOutputPoint * pICip2;

        while ((pICip2 = reinterpret_cast< IRadControllerOutputPoint * >( m_xIOl_OutputPoints->GetNext( ) )))
        {
            if ( strcmp( pICip2->GetType( ), pType ) == 0 )
            {
                count++;
            }        
        }

        return count;
    }

    //========================================================================
    // radControllerSDL::GetInputPointByTypeAndIndex
    //========================================================================

    virtual IRadControllerInputPoint * GetInputPointByTypeAndIndex
    (
        const char * pType,
        unsigned int index
    )
    {
        //
        // Just loop through all of the input points counting each one of
        // that time
        //

        rAssert( pType != NULL );

        unsigned int count = 0;

        m_xIOl_InputPoints->Reset( );

        IRadControllerInputPoint * pICip2;

        while ((pICip2 = reinterpret_cast< IRadControllerInputPointSDL * >( m_xIOl_InputPoints->GetNext( ) )))
        {
            if ( strcmp( pICip2->GetType( ), pType ) == 0 )
            {
                if ( count == index )
                {
                    return pICip2;
                }

                count++;
            }
        }
        
        return NULL;
    }

    //========================================================================
    // radControllerSDL::GetOutputPointByTypeAndIndex
    //========================================================================

    IRadControllerOutputPoint * GetOutputPointByTypeAndIndex
    (  
        const char * pType,
        unsigned int index
    ) 
    {
        //
        // Just loop through all of the Output points counting each one of
        // that time
        //

        rAssert( pType != NULL );

        unsigned int count = 0;

        m_xIOl_OutputPoints->Reset( );

        IRadControllerOutputPoint * pICip2;

        while ((pICip2 = reinterpret_cast< IRadControllerOutputPoint * >( m_xIOl_OutputPoints->GetNext( ) )))
        {
            if ( strcmp( pICip2->GetType( ), pType ) == 0 )
            {
                if ( count == index )
                {
                    return pICip2;
                }

                count++;
            }
        }

        return NULL;
    }

    //========================================================================
    // radControllerSDL::GetInputPointByName
    //========================================================================

    virtual IRadControllerInputPoint * GetInputPointByName
    (
        const char * pName
    )
    {
        //
        // Just loop through all of the input points comparing each ones
        // name to the name passed in.
        //

        rAssert( pName != NULL );

        m_xIOl_InputPoints->Reset( );

        IRadControllerInputPoint * pICip2;

        while ((pICip2 = reinterpret_cast< IRadControllerInputPointSDL * >( m_xIOl_InputPoints->GetNext( ) )))
        {
            if ( strcmp( pName, pICip2->GetName( ) ) == 0 )
            {
                return pICip2;
            }
        }

        return NULL;
    }

    //========================================================================
    // radControllerSDL::GetOutputPointByName
    //========================================================================

    IRadControllerOutputPoint * GetOutputPointByName
    (
        const char * pName
    )
    {
        //
        // Just loop through all of the Output points comparing each ones
        // name to the name passed in.
        //

        rAssert( pName != NULL );

        m_xIOl_OutputPoints->Reset( );

        IRadControllerOutputPoint * pICip2;

        while ((pICip2 = reinterpret_cast< IRadControllerOutputPoint * >( m_xIOl_OutputPoints->GetNext( ) )))
        {
            if ( strcmp( pName, pICip2->GetName( ) ) == 0 )
            {
                return pICip2;
            }
        }

        return NULL;
    }

    //========================================================================
    // radControllerSDL::GetLocation
    //========================================================================

    virtual const char * GetLocation( void )
    {
        //
        // Just return the location string;
        //

        return m_xIString_Location->GetChars( );
    }

    //========================================================================
    // radControllerSDL::GetNumberOfInputPoints
    //========================================================================

    virtual unsigned int GetNumberOfInputPoints( void )
    {
        return m_xIOl_InputPoints->GetSize( );
    }

    //========================================================================
    // radControllerSDL::GetInputPointByIndex
    //========================================================================

    virtual IRadControllerInputPoint * GetInputPointByIndex( unsigned int index )
    {
        return reinterpret_cast< IRadControllerInputPointSDL * >( m_xIOl_InputPoints->GetAt( index ) );
    }

    //========================================================================
    // radControllerSDL::GetNumberOfInputPoints
    //========================================================================

    virtual unsigned int GetNumberOfOutputPoints( void )
    {
        return m_xIOl_OutputPoints->GetSize( );
    }

    //========================================================================
    // radControllerSDL::GetOutputPointByIndex
    //========================================================================

    virtual IRadControllerOutputPoint * GetOutputPointByIndex( unsigned int index )
    {
        return reinterpret_cast< IRadControllerOutputPoint * >( m_xIOl_OutputPoints->GetAt( index ) );
    }

    //========================================================================
    // radControllerSDL::radControllerSDL
    //========================================================================

    radControllerSDL
    (
        unsigned int thisAllocator,
        SDL_GameController* pController,
        unsigned int virtualTime,
        unsigned int bufferTime,
        unsigned int pollingRate
    )
        :
        radRefCount( 0 ),
        m_pController( pController ),
        m_DeviceKind( SDLDevice_Pad )
    {
        radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radControllerSDL" );

        m_LeftGain = m_RightGain = 0;

        //
        // Get an object list to store our input points
        //

        ::radObjectListCreate( & m_xIOl_InputPoints, g_ControllerSystemAllocator );
        ::radObjectListCreate( & m_xIOl_OutputPoints, g_ControllerSystemAllocator );

        //
        // Get a string to store our location
        //

        ::radStringCreate( & m_xIString_Location, g_ControllerSystemAllocator );

        //
        // Create our location name based on our port and slot
        //

        int iController = ( std::max )( SDL_GameControllerGetPlayerIndex( pController ), 0 );
        m_xIString_Location->SetSize( 12 );
#if defined( RAD_MACOS )
        m_xIString_Location->Append( "Joystick" );
        m_xIString_Location->Append( (unsigned int) iController );
#else
        m_xIString_Location->Append( "Port" );
        m_xIString_Location->Append( (unsigned int) iController );
        m_xIString_Location->Append( "\\Slot0" );
#endif

        //
        // Create all of our intput points, this is always the same for every
        // sdl controller.  See static array above.
        //

        for ( unsigned int button = 0; button < ( sizeof( g_SDLPoints ) / sizeof( SDLInputPoint ) ); button++ )
        {
			radRef< radControllerInputPointSDL > pInputPoint = new( g_ControllerSystemAllocator ) radControllerInputPointSDL
			(
                m_pController,
                g_SDLPoints[ button ].m_pType, 
                g_SDLPoints[ button ].m_pName,
                g_SDLPoints[ button ].m_Mask
            );

            m_xIOl_InputPoints->AddObject( pInputPoint );

			//
			// Hand the point its first value
			//

            pInputPoint->iInitialize( );
        }

        if ( m_xIOl_OutputPoints != NULL )
        {
            radControllerOutputPointSDL * pLeft = new( g_ControllerSystemAllocator ) radControllerOutputPointSDL( "LeftMotor" );

            radControllerOutputPointSDL * pRight = new( g_ControllerSystemAllocator ) radControllerOutputPointSDL( "RightMotor" );

            m_xIOl_OutputPoints->AddObject( reinterpret_cast< IRefCount * >( pLeft ) );
            m_xIOl_OutputPoints->AddObject( reinterpret_cast< IRefCount * >( pRight ) );
        }

        //
        // Set everything to a know state using our regular runtime functions.
        // Note that the controller might get created during gameplay after
        // the controller system has been run for a while.
        // 

        iSetBufferTime( bufferTime, pollingRate );
        iVirtualTimeReMapped( virtualTime );
    }

    //========================================================================
    // radControllerSDL::radControllerSDL (keyboard / mouse)
    //========================================================================

    radControllerSDL
    (
        unsigned int thisAllocator,
        SDLDeviceKind deviceKind,
        unsigned int virtualTime,
        unsigned int bufferTime,
        unsigned int pollingRate
    )
        :
        radRefCount( 0 ),
        m_pController( NULL ),
        m_DeviceKind( deviceKind )
    {
        radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radControllerSDL" );

        m_LeftGain = m_RightGain = 0;

        ::radObjectListCreate( & m_xIOl_InputPoints, g_ControllerSystemAllocator );
        ::radStringCreate( & m_xIString_Location, g_ControllerSystemAllocator );

        //
        // The game looks these devices up by name, matching the locations the
        // DirectInput backend uses. There is only ever one of each.
        //

        m_xIString_Location->SetSize( 12 );
        m_xIString_Location->Append( deviceKind == SDLDevice_Keyboard ? "Keyboard0" : "Mouse0" );

        if ( deviceKind == SDLDevice_Keyboard )
        {
            for ( unsigned int i = 0; i < 256; i++ )
            {
                s_VirtualKeyToIndex[ i ] = -1;
            }

            for ( unsigned int key = 0; key < ( sizeof( g_SDLKeyPoints ) / sizeof( SDLKeyPoint ) ); key++ )
            {
                radRef< radControllerInputPointSDL > pInputPoint = new( g_ControllerSystemAllocator ) radControllerInputPointSDL
                (
                    NULL,
                    g_Sdlipt[ 0 ],
                    SDL_GetScancodeName( (SDL_Scancode) g_SDLKeyPoints[ key ].m_Scancode ),
                    g_SDLKeyPoints[ key ].m_Scancode,
                    SDLDevice_Keyboard
                );

                //
                // Keyboard.cpp resolves a DIK code to an input point through
                // VirtualKeyToIndex, so record where each key landed.
                //

                s_VirtualKeyToIndex[ g_SDLKeyPoints[ key ].m_DIK - 1 ] = (int) m_xIOl_InputPoints->GetSize( );

                m_xIOl_InputPoints->AddObject( pInputPoint );
                pInputPoint->iInitialize( );
            }
        }
        else
        {
            SDL_AddEventWatch( SDLWatchMouseMotion, this );

            for ( unsigned int i = 0; i < ( sizeof( g_SDLMousePoints ) / sizeof( SDLInputPoint ) ); i++ )
            {
                radRef< radControllerInputPointSDL > pInputPoint = new( g_ControllerSystemAllocator ) radControllerInputPointSDL
                (
                    NULL,
                    g_SDLMousePoints[ i ].m_pType,
                    g_SDLMousePoints[ i ].m_pName,
                    g_SDLMousePoints[ i ].m_Mask,
                    SDLDevice_Mouse
                );

                m_xIOl_InputPoints->AddObject( pInputPoint );
                pInputPoint->iInitialize( );
            }
        }

        iSetBufferTime( bufferTime, pollingRate );
        iVirtualTimeReMapped( virtualTime );
    }

    //========================================================================
    // radControllerSDL::
    //========================================================================

    ~radControllerSDL( void )
    {
        if ( m_DeviceKind == SDLDevice_Mouse )
        {
            SDL_DelEventWatch( SDLWatchMouseMotion, this );
        }
    }

    //========================================================================
    // radControllerSDL Data Members
    //========================================================================

    SDL_GameController *              m_pController;
    SDLDeviceKind                     m_DeviceKind;

    radRef< IRadObjectList >             m_xIOl_InputPoints;
    radRef< IRadObjectList >             m_xIOl_OutputPoints;

    radRef< IRadString >                 m_xIString_Location;
    
    uint16_t                          m_LeftGain, m_RightGain;
};

//============================================================================
// Component: radControllerSystemSDL
//============================================================================

class radControllerSystemSDL
    :
    public IRadControllerSystem,
    public IRadTimerCallback,
    public radRefCount
{
    public:

    IMPLEMENT_REFCOUNTED( "radControllerSystemSDL" )

     //========================================================================
    // radControllerSystemSDL::CheckDeviceConnectionStatus
    //========================================================================

    static int CheckDeviceConnectionStatus( void * userdata, SDL_Event * event )
    {
        //
        // Check if devices have been inserted or removed
        //
        SDL_GameController* pController;
        if( event->type == SDL_CONTROLLERDEVICEADDED )
            pController = SDL_GameControllerOpen( event->cdevice.which );
        else if( event->type == SDL_CONTROLLERDEVICEREMOVED )
            pController = SDL_GameControllerFromInstanceID( event->cdevice.which );
        else
            return 1;

        radControllerSystemSDL* sys = (radControllerSystemSDL*)userdata;
        sys->AddRef( );

        //
        // Find the controller in question (may not exist though)
        //
        radRef< IRadController > xIController2;
        radRef< IRadControllerSDL > xISDLController2;

        char location[255];

        int iController = ( std::max )( SDL_GameControllerGetPlayerIndex( pController ), 0 );
#if defined( RAD_MACOS )
        sprintf( location, "Joystick%d", iController );
#else
        sprintf( location, "Port%d\\Slot0", iController );
#endif

        xIController2 = sys->GetControllerAtLocation( location );

        if ( xIController2 != NULL )
        {
            xISDLController2 = (IRadControllerSDL*)xIController2.m_pInterface;
            rAssert( xISDLController2 != NULL );
        }

        if( event->type == SDL_CONTROLLERDEVICEADDED )
        {
            //
            // Here a device has been inserted, so open it
            //

            if( xISDLController2 == NULL )
            {
                //
                // Here the controller at this location has not yet been 
                // constructed, so construct a new controller
                //

                unsigned int virtualTime = 0;
                unsigned int pollingRate = 10;

                virtualTime = radTimeGetMilliseconds() + sys->m_VirtualTimeAdjust;
                            
                if ( sys->m_xITimer != NULL )
                {
                    pollingRate = sys->m_xITimer->GetTimeout( );
                }

                xIController2 = new ( g_ControllerSystemAllocator ) radControllerSDL
                (
                    g_ControllerSystemAllocator,
                    pController,
                    virtualTime,
                    sys->m_EventBufferTime,
                    pollingRate
                );

                sys->m_xIOl_Controllers->AddObject
                (
                    xIController2
                );
            }
            else
            {
                sys->Release( );
                return 0;
            }
        }
        else if( event->type == SDL_CONTROLLERDEVICEREMOVED )
        {
			//
            // Here a device has been removed
            //
            if ( xIController2 != NULL )
            {
                //We need to remove this from the set as the next thing to 
                //plug in could be a new type of controller.
                sys->m_xIOl_Controllers->RemoveObject( xIController2 );
            }
        }

        IRadWeakInterfaceWrapper * pIWir;

        sys->m_xIOl_Callbacks->Reset( );

        while((pIWir = reinterpret_cast< IRadWeakInterfaceWrapper * >( sys->m_xIOl_Callbacks->GetNext( ) )))
        {

            IRadControllerConnectionChangeCallback * pCallback = (IRadControllerConnectionChangeCallback *) pIWir->GetWeakInterface( );
            pCallback->OnControllerConnectionStatusChange( xIController2 );
        }

        sys->Release( );
        return 0;
    }

    //========================================================================
    // radControllerSystemSDL::OnTimerDone
    //========================================================================

    virtual void OnTimerDone( unsigned int elapsedtime, void* pUserData )
    {
        //
        // Now, update all of our controllers
        //

        m_xIOl_Controllers->Reset( );

        IRadControllerSDL * pIXbc2;

        while ((pIXbc2 = reinterpret_cast< IRadControllerSDL * >( m_xIOl_Controllers->GetNext( ) )))
        {
            //
            // The controller stamps packets with virtual time, so
            // we pass in virtual time.
            //

            pIXbc2->iPoll( radTimeGetMilliseconds( ) + m_VirtualTimeAdjust );
        }

        //
        // If the client is not driving us, we drive ourselves from this timer
        //

        if ( m_UsingVirtualTime == false )
        {
            SetVirtualTime( radTimeGetMilliseconds( ) );
        }
    }

    //========================================================================
    // radControllerSystemSDL::GetNumberOfControlelrs
    //========================================================================

    virtual unsigned int GetNumberOfControllers( void )
    {
        return m_xIOl_Controllers->GetSize( );
    }

    //========================================================================
    // radControllerSystemSDL::GetControllerByIndex
    //========================================================================

    virtual IRadController * GetControllerByIndex( unsigned int myindex )
    {
        return reinterpret_cast< IRadControllerSDL * >( m_xIOl_Controllers->GetAt( myindex ) );
    }

    //========================================================================
    // radControllerSystemSDL::GetControllerAtLocation
    //========================================================================

    virtual IRadController * GetControllerAtLocation
    (
        const char * pLocation
    )
    {
        //
        // Just loop through all of the controllers asking it for its location
        // if we find a match, return it.
        //

        rAssert( pLocation != NULL );

        m_xIOl_Controllers->Reset( );

        IRadController * pIC2;

        while ((pIC2 = reinterpret_cast< IRadControllerSDL * >( m_xIOl_Controllers->GetNext( ) )))
        {
            if ( strcmp( pIC2->GetLocation(), pLocation ) == 0 )
            {
                return pIC2;
            }
        }

        return NULL;
    }

    //========================================================================
    // radControllerSystemSDL::SetBufferTime
    //========================================================================

    virtual void SetBufferTime( unsigned int milliseconds )
    {
        if ( milliseconds == 0 )
        {
            //
            // We are always buffering behind the scenes, so we set the
            // buffering time to one 60Hz frame even if the client thinks
            // we are just maintaining state.
            //

            m_UsingVirtualTime = false;

            MapVirtualTime( 0, 0 );

            milliseconds = 10;
        }
        else
        {
            m_UsingVirtualTime = true;
        }

        
        unsigned int pollingRate = 10;

        pollingRate = m_xITimer->GetTimeout( );

        m_EventBufferTime = milliseconds;

        m_xIOl_Controllers->Reset( );

        IRadControllerSDL * pIDipc2;

        while ((pIDipc2 = reinterpret_cast< IRadControllerSDL * >( m_xIOl_Controllers->GetNext( ) )))
        {
            pIDipc2->iSetBufferTime( milliseconds, pollingRate );
        }
    }

    //========================================================================
    // radControllerSystemSDL::MapVirtualTime
    //========================================================================

    virtual void MapVirtualTime
    (
        unsigned int systemTicks,
        unsigned int virtualTicks
    )
    {
        //
        // Here the client has requested a (re)mapping of virtual time to
        // timer manager time.
        //

        //
        // Recalculate the adjustment factor.  This number represents the
        // number to add to timer manager ticks to get game ticks.
        //
        m_VirtualTimeAdjust = virtualTicks - systemTicks;

        //
        // Inform each controller that virtual time has been remapped.
        //

        m_xIOl_Controllers->Reset( );

        IRadControllerSDL * pIXbc2;

        while ((pIXbc2 = reinterpret_cast< IRadControllerSDL * >( m_xIOl_Controllers->GetNext( ) )))
        {
            pIXbc2->iVirtualTimeReMapped( radTimeGetMilliseconds() + m_VirtualTimeAdjust );
        }       
    }

    //========================================================================
    // radControllerSystemSDL::SetVirtualTime
    //========================================================================

    virtual void SetVirtualTime( unsigned int virtualTime )
    {
        //
        // Inform each controller that virtual time has changed.
        //

        m_xIOl_Controllers->Reset( );

        IRadControllerSDL * pIXbc2;

        while ((pIXbc2 = reinterpret_cast< IRadControllerSDL * >( m_xIOl_Controllers->GetNext( ) )))
        {
            pIXbc2->iVirtualTimeChanged( virtualTime );
        }
    }

    //========================================================================
    // radControllerSystemSDL::SetCaptureRate
    //========================================================================

    virtual void SetCaptureRate( unsigned int ms )
    {
        m_xITimer->SetTimeout( ms );

        //
        // Reset event queue size given new filling rate
        //

        SetBufferTime( m_EventBufferTime );
    }

    //========================================================================
    // radControllerSystemSDL::RegisterConnectionChangeCallback2
    //========================================================================

    virtual void RegisterConnectionChangeCallback
    (
        IRadControllerConnectionChangeCallback * pCallback
    )
    {
        //
        // Store the callback in a weak interface wrapper so we can store
        // it in an object list.
        //

        rAssert( pCallback != NULL );

        radRef< IRadWeakInterfaceWrapper > xIWir;

        ::radWeakInterfaceWrapperCreate( & xIWir, g_ControllerSystemAllocator );

        xIWir->SetWeakInterface( pCallback );

        m_xIOl_Callbacks->AddObject( xIWir );
    }

    //========================================================================
    // radControllerSystemSDL::UnRegisterConnectionChagneCallback2
    //========================================================================

    virtual void UnRegisterConnectionChangeCallback
    (
        IRadControllerConnectionChangeCallback * pCallback
    )
    {
        //
        // Look for the callback in the list and remove it if found.
        //

        rAssert( pCallback != NULL );

        IRadWeakInterfaceWrapper * pIWir;

        m_xIOl_Callbacks->Reset( );

        while ((pIWir = reinterpret_cast< IRadWeakInterfaceWrapper * >( m_xIOl_Callbacks->GetNext( ) )))
        {
            if ( pIWir->GetWeakInterface() == pCallback )
            {
                m_xIOl_Callbacks->RemoveObject( pIWir  );
                return;
            }
        }
        
        rAssertMsg( false, "Controller connection change callback not registered." );            
    }

    //========================================================================
    // radControllerSystemSDL::Service
    //========================================================================

    void Service( void )
    {
        m_xITimerList->Service(  );   
    }

    //========================================================================
    // radControllerSystemSDL::radControllerSystemSDL
    //========================================================================

    radControllerSystemSDL
    (
        IRadControllerConnectionChangeCallback* pConnectionChangeCallback,
        radMemoryAllocator thisAllocator
    )
        :
        m_UsingVirtualTime( false ),
        m_VirtualTimeAdjust( 0 ),
        m_EventBufferTime( 0 ),
        m_DefaultConnectionChangeCallback( NULL )
    {
        radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radControllerSystemSDL" );

        //
        // Set our singleton to "this"
        //
        rAssert( s_pTheSDLControllerSystem2 == NULL );
        s_pTheSDLControllerSystem2 = this;
        
        g_ControllerSystemAllocator = thisAllocator;

		//
        // Create a timer to poll with, this will be changed to a thread if
        // people start complaining about latencies.
        //

        //
        // Create a timer list to drive the processing of contollers
        //
    
        radTimeCreateList( &m_xITimerList, 1, g_ControllerSystemAllocator );

        m_xITimerList->CreateTimer( & m_xITimer, 10, this );

        //
        // Object list to hold controllers
        //
        ::radObjectListCreate( & m_xIOl_Controllers, g_ControllerSystemAllocator );
        rAssert( m_xIOl_Controllers != NULL );

        //
        // Object list to hold callbacks
        //
        ::radObjectListCreate( & m_xIOl_Callbacks, g_ControllerSystemAllocator );
        rAssert( m_xIOl_Callbacks != NULL );

        //
        // Register the default connection state callback
        //
        m_DefaultConnectionChangeCallback = pConnectionChangeCallback;
        if( pConnectionChangeCallback )
        {
            m_DefaultConnectionChangeCallback = pConnectionChangeCallback;
            RegisterConnectionChangeCallback( pConnectionChangeCallback );
        }

        //
        // Keyboard and mouse are always present, so create them up front.
        // The game requires a keyboard at Keyboard0 to accept player one.
        //

        {
            unsigned int virtualTime = radTimeGetMilliseconds() + m_VirtualTimeAdjust;
            unsigned int pollingRate = ( m_xITimer != NULL ) ? m_xITimer->GetTimeout() : 10;

            static const SDLDeviceKind kinds[] = { SDLDevice_Keyboard, SDLDevice_Mouse };

            for( unsigned int i = 0; i < 2; i++ )
            {
                radRef< IRadController > xIController2 = new (g_ControllerSystemAllocator) radControllerSDL
                (
                    g_ControllerSystemAllocator,
                    kinds[ i ],
                    virtualTime,
                    m_EventBufferTime,
                    pollingRate
                );

                m_xIOl_Controllers->AddObject( xIController2 );
            }
        }

        //
        // TODO: If there is no connection change callback, wait synchronously for the connection
        //
        for( int i = 0; i < SDL_NumJoysticks(); i++ )
        {
            if( SDL_IsGameController( i ) )
            {
                radRef< IRadController > xIController2;
                unsigned int virtualTime = 0;
                unsigned int pollingRate = 10;

                virtualTime = radTimeGetMilliseconds() + m_VirtualTimeAdjust;

                if( m_xITimer != NULL )
                {
                    pollingRate = m_xITimer->GetTimeout();
                }

                xIController2 = new (g_ControllerSystemAllocator) radControllerSDL
                (
                    g_ControllerSystemAllocator,
                    SDL_GameControllerOpen( i ),
                    virtualTime,
                    m_EventBufferTime,
                    pollingRate
                );

                m_xIOl_Controllers->AddObject
                (
                    xIController2
                );

                IRadWeakInterfaceWrapper* pIWir;

                m_xIOl_Callbacks->Reset();

                while((pIWir = reinterpret_cast<IRadWeakInterfaceWrapper*>(m_xIOl_Callbacks->GetNext())))
                {

                    IRadControllerConnectionChangeCallback* pCallback = (IRadControllerConnectionChangeCallback*)pIWir->GetWeakInterface();
                    pCallback->OnControllerConnectionStatusChange( xIController2 );
                }
            }
        }

        //
        // Set everything to know state
        //        
        SetCaptureRate( 10 );
        MapVirtualTime( 0, 0 );
        SetBufferTime( 0 );
    }

    //========================================================================
    // radControllerSystemSDL::~radControllerSystemSDL
    //========================================================================
        
    ~radControllerSystemSDL( void )
    {
        //
        // Unregister the default connection change callback
        //
        if( m_DefaultConnectionChangeCallback != NULL )
        {
            UnRegisterConnectionChangeCallback( m_DefaultConnectionChangeCallback );
            m_DefaultConnectionChangeCallback = NULL;
        }

        //
        // Make sure the client(s) unregistered all of their callbacks.
        //
        
        rAssertMsg( m_xIOl_Callbacks->GetSize() == 0, "Somebody forgot to unregister a controller connection change callback" );

        g_ControllerSystemAllocator = RADMEMORY_ALLOC_DEFAULT;

        //
        // Set the singleton back to null.
        //

        rAssert( s_pTheSDLControllerSystem2 == this );
        s_pTheSDLControllerSystem2 = NULL;

    }


    //========================================================================
    // Data Members
    //========================================================================
    
    bool m_UsingVirtualTime;
    unsigned int m_VirtualTimeAdjust;
    unsigned int m_EventBufferTime;

    IRadControllerConnectionChangeCallback* m_DefaultConnectionChangeCallback;

    radRef< IRadObjectList >     m_xIOl_Callbacks;

    radRef< IRadObjectList >     m_xIOl_Controllers;
    radRef< IRadTimer >          m_xITimer;
    radRef< IRadTimerList >      m_xITimerList;

};

//============================================================================
// Function:    radControllerInitialize
// Paramters:   pConnectionChangeCallback - a callback that is called when the
//                  connection status of controllers changes.  It is called
//                  as soon as a controller is found and properly initialized.
//============================================================================

void radControllerInitialize
(
    IRadControllerConnectionChangeCallback* pConnectionChangeCallback,
    radMemoryAllocator alloc
)
{
    rAssert( s_pTheSDLControllerSystem2 == NULL );

    new ( alloc ) radControllerSystemSDL( pConnectionChangeCallback, alloc );

    SDL_AddEventWatch( radControllerSystemSDL::CheckDeviceConnectionStatus, s_pTheSDLControllerSystem2 );
}

void radControllerTerminate( void )
{
    SDL_DelEventWatch( radControllerSystemSDL::CheckDeviceConnectionStatus, s_pTheSDLControllerSystem2 );

    radRelease( s_pTheSDLControllerSystem2, NULL );
}

//============================================================================
// Function: radControllerSystemGet
//============================================================================
//
// Use this function to obtain an interface to the controller system object.
//
IRadControllerSystem* radControllerSystemGet
( 
    void
)
{
    rAssert( s_pTheSDLControllerSystem2 != NULL );

    return( s_pTheSDLControllerSystem2 );
}

//============================================================================
// Function: radControllerSystemGet
//============================================================================
//
// Use this function to drive the processing of the controller system
//
void radControllerSystemService( void )
{
    if( s_pTheSDLControllerSystem2 != NULL )
    {
        s_pTheSDLControllerSystem2->Service( );
    }
}

#endif // RAD_UWP || RAD_MACOS