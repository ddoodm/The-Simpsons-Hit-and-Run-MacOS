//===========================================================================
// Copyright (C) 2002 Radical Entertainment Ltd.  All rights reserved.
//
// Component:   SdlPlatform   
//
// Description: Abstracts the differences for setting up and shutting down
//              the different platforms.
//
// History:     + Stolen and cleaned up from Penthouse -- Darwin Chau
//
//===========================================================================

//========================================
// System Includes
//========================================
// Standard Lib
#include <stdlib.h>
#include <string.h>
// Pure 3D
#include <p3d/anim/compositedrawable.hpp>
#include <p3d/anim/expression.hpp>
#include <p3d/anim/multicontroller.hpp>
#include <p3d/anim/polyskin.hpp>
#include <p3d/anim/sequencer.hpp>
#include <p3d/anim/skeleton.hpp>
#include <p3d/camera.hpp>
#include <p3d/gameattr.hpp>
#include <p3d/image.hpp>
#include <p3d/imagefont.hpp>
#include <p3d/light.hpp>
#include <p3d/locator.hpp>
#include <p3d/platform.hpp>
#include <p3d/scenegraph/scenegraph.hpp>
#include <p3d/sprite.hpp>
#include <p3d/utility.hpp>
#include <p3d/texture.hpp>
#include <p3d/file.hpp>
#include <p3d/shader.hpp>
#include <p3d/matrixstack.hpp>
#include <p3d/memory.hpp>
#include <p3d/bmp.hpp>
#include <p3d/png.hpp>
#include <p3d/targa.hpp>
#include <p3d/font.hpp>
#include <p3d/texturefont.hpp>
#include <p3d/unicode.hpp>
// Pure 3D: Loader-specific
#include <render/Loaders/GeometryWrappedLoader.h>
#include <render/Loaders/StaticEntityLoader.h>
#include <render/Loaders/StaticPhysLoader.h>
#include <render/Loaders/TreeDSGLoader.h>
#include <render/Loaders/FenceLoader.h>
#include <render/Loaders/IntersectLoader.h>
#include <render/Loaders/AnimCollLoader.h>
#include <render/Loaders/AnimDSGLoader.h>
#include <render/Loaders/DynaPhysLoader.h>
#include <render/Loaders/InstStatPhysLoader.h>
#include <render/Loaders/InstStatEntityLoader.h>
#include <render/Loaders/WorldSphereLoader.h>
#include <loading/roaddatasegmentloader.h>
#include <render/Loaders/BillboardWrappedLoader.h>
#include <render/Loaders/instparticlesystemloader.h>
#include <render/Loaders/breakableobjectloader.h>
#include <render/Loaders/AnimDynaPhysLoader.h>
#include <render/Loaders/lensflareloader.h>
#include <p3d/shadow.hpp>
#include <p3d/anim/animatedobject.hpp>
#include <p3d/effects/particleloader.hpp>
#include <p3d/effects/opticloader.hpp>
#include <p3d/anim/vertexanimkey.hpp>
#include <stateprop/statepropdata.hpp>

// Foundation Tech
#include <raddebug.hpp>
#include <radthread.hpp>
#include <radplatform.hpp>
#include <radtime.hpp>
#include <radmemorymonitor.hpp>
#include <raddebugcommunication.hpp>
#include <raddebugwatch.hpp>
#include <radfile.hpp>
#include <radmovie2.hpp>

//This is so we can get the name of the file that's failing.
#include <../src/radfile/common/requests.hpp>

// sim - for InstallSimLoaders
#include <simcommon/simutility.hpp>

//========================================
// Project Includes
//========================================
#include <input/inputmanager.h>
#include <main/sdlplatform.h>
#include <main/commandlineoptions.h>
#include <main/game.h>
#include <render/RenderManager/RenderManager.h>
#include <render/RenderFlow/renderflow.h>
#include <render/Loaders/AllWrappers.h>
#include <memory/srrmemory.h>

#include <loading/locatorloader.h>
#include <loading/cameradataloader.h>
#include <loading/roadloader.h>
#include <loading/pathloader.h>
#include <loading/intersectionloader.h>
#include <loading/roaddatasegmentloader.h>
#include <atc/atcloader.h>
#include <data/gamedatamanager.h>
#include <data/config/gameconfigmanager.h>
#include <debug/debuginfo.h>
#include <constants/srrchunks.h>
#include <gameflow/gameflow.h>
#include <sound/soundmanager.h>
#include <presentation/presentation.h>
#include <presentation/gui/guitextbible.h>
#include <cheats/cheatinputsystem.h>
#include <mission/gameplaymanager.h>




#include <radload/radload.hpp>

#include <main/errorswin32.h>

#define _stricmp SDL_strcasecmp
#define WIN32_SECTION "WIN32_SECTION"
#define TIMER_LEAVE 1

//#define PRINT_WINMESSAGES

//******************************************************************************
//
// Global Data, Local Data, Local Classes
//
//******************************************************************************

// Static pointer to instance of singleton.
SdlPlatform* SdlPlatform::spInstance = NULL;

// Other static members.
SDL_Window* SdlPlatform::mWnd = NULL;
void* SdlPlatform::mhMutex = NULL;
bool SdlPlatform::mShowCursor = true;

//
// Define the starting resolution.
//
static const SdlPlatform::Resolution StartingResolution = SdlPlatform::Res_800x600;
static const int StartingBPP = 32;

// This specifies the PDDI DLL to use.
#ifdef RAD_DEBUG
static const char pddiLibraryName[] = "pddi%sd.dll";
#endif
#ifdef RAD_TUNE
static const char pddiLibraryName[] = "pddi%st.dll";
#endif
#ifdef RAD_RELEASE
static const char pddiLibraryName[] = "pddi%sr.dll";
#endif

// Name of the application.  This is the string that appears in the Window's
// title bar.
static const char ApplicationName[] = "The Simpsons: Hit & Run";

// The gamma of the desktop.. needed to reset it on alt-tabs.
static Uint16 DesktopGammaRamp[3][256] = { 0 };

void LoadMemP3DFile( unsigned char* buffer, unsigned int size, tEntityStore* store )
{
    tFileMem* file = new tFileMem(buffer,size);
    file->AddRef();
    file->SetFilename("memfile.p3d");
    p3d::loadManager->GetP3DHandler()->Load( file, p3d::inventory );
    file->Release();
}

//******************************************************************************
//
// Public Member Functions
//
//******************************************************************************

//==============================================================================
// SdlPlatform::CreateInstance
//==============================================================================
//
// Description: Creates the SdlPlatform.
//
// Parameters:	win32 parameters.
//
// Return:      Pointer to the SdlPlatform.
//
// Constraints: This is a singleton so only one instance is allowed.
//
//==============================================================================
SdlPlatform* SdlPlatform::CreateInstance()
{
MEMTRACK_PUSH_GROUP( "SdlPlatform" );
    rAssert( spInstance == NULL );

    spInstance = new(GMA_PERSISTENT) SdlPlatform();

    rAssert( spInstance );
MEMTRACK_POP_GROUP( "SdlPlatform" );

    return spInstance;
}

//==============================================================================
// SdlPlatform::GetInstance
//==============================================================================
//
// Description: - Access point for the SdlPlatform singleton.  
//
// Parameters:	None.
//
// Return:      Pointer to the SdlPlatform.
//
// Constraints: This is a singleton so only one instance is allowed.
//
//==============================================================================
SdlPlatform* SdlPlatform::GetInstance()
{
    rAssert( spInstance != NULL );

    return spInstance;
}


//==============================================================================
// SdlPlatform::DestroyInstance
//==============================================================================
//
// Description: Destroy the SdlPlatform.
//
// Parameters:	None.
//
// Return:      None.
//
//==============================================================================
void SdlPlatform::DestroyInstance()
{
    rAssert( spInstance != NULL );

    delete( GMA_PERSISTENT, spInstance );
    spInstance = NULL;
}



//==============================================================================
// SdlPlatform::InitializeWindow
//==============================================================================
// Description: Creates the window class and window instance for the application.
//              We must do this before initializing the platform.
//
// Parameters:	hInstance - the handle to the instance.
//
// Return:      true if successful and the program can run.
//              false if another simpsons window already exists and this
//                instance should terminate.
//
// Constraints: Must be initialized before the platform.
//
//==============================================================================
bool SdlPlatform::InitializeWindow() 
{
#if defined( RAD_WIN32 )
    // check to see if another instance is running...
    mhMutex = CreateMutex(NULL, 0, ApplicationName);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // simpsons is already running, so lets find the window and give it focus
        HWND hwnd = FindWindow(ApplicationName, NULL);
        if (hwnd != NULL)
        {
            // if window is minimized, restore it
            WINDOWPLACEMENT wndpl;
            if (GetWindowPlacement(hwnd, &wndpl) != 0)
            {
                if ((wndpl.showCmd == SW_MINIMIZE) ||
                    (wndpl.showCmd == SW_SHOWMINIMIZED))
                {
                    ShowWindow(hwnd, SW_RESTORE);
                }
            }

            // activate the window
            SetForegroundWindow(hwnd);

            return false;
        }
    }
#endif

    // These three attributes must be set prior to creating the first window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    int w, h;
    TranslateResolution( StartingResolution, w, h );
    mWnd = SDL_CreateWindow( ApplicationName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL );

    rAssert(mWnd != NULL);

#if defined( PRINT_WINMESSAGES ) && defined( RAD_DEBUG )
    SDL_SetHint( SDL_HINT_EVENT_LOGGING, "1" );
#endif
    SDL_SetEventFilter( WndProc, mWnd );

    ShowTheCursor( false );

    SDL_GetWindowGammaRamp( mWnd, DesktopGammaRamp[0], DesktopGammaRamp[1], DesktopGammaRamp[2] );

    SDL_DisableScreenSaver();

    return true;
}

//==============================================================================
// SdlPlatform::InitializeFoundation
//==============================================================================
// Description: FTech must be setup first so that all the memory services
//              are ready to go before we begin allocating anything.
//
// Parameters:	None.
//
// Return:      None.
//
// Constraints: The FTech systems must be initialized in a particular order.
//              Consult their documentation before changing.
//
//==============================================================================
void SdlPlatform::InitializeFoundation() 
{
    //
    // Initialize the memory heaps
    // obsolete now.. the heaps initialize memory.
    //
    //InitializeMemory();

    //
    // Register an out-of-memory display handler in case something goes bad
    // while allocating the heaps
    //
    ::radMemorySetOutOfMemoryCallback( PrintOutOfMemoryMessage, NULL );

    //
    // Initialize memory monitor by JamesCo. TM.
    //
    if( CommandLineOptions::Get( CLO_MEMORY_MONITOR ) )
    {
        const int KB = 1024;
        ::radMemoryMonitorInitialize( 64 * KB, GMA_DEBUG );
    }

    // Setup the memory heaps
    //
    HeapMgr()->PrepareHeapsStartup ();

    // Seed the heap stack
    //
    HeapMgr()->PushHeap (GMA_PERSISTENT);

    //
    // Initilalize the platform system
    // 
    ::radPlatformInitialize( mWnd );

    //
    // Initialize the timer system
    //
    ::radTimeInitialize();

    //
    // Initialize the debug communication system.
    //
    ::radDbgComTargetInitialize( WinSocket, 
        radDbgComDefaultPort, // Default
        NULL,                 // Default
        GMA_DEBUG );


    //
    // Initialize the Watcher.
    //
    ::radDbgWatchInitialize( "SRR2",
                             32 * 16384, // 2 * Default
                             GMA_DEBUG );

    //
    // Initialize the file system.
    //
    ::radFileInitialize( 50, // Default
        32, // Default
        GMA_PERSISTENT );

    ::radLoadInitialize();
    //radLoad->SetSyncLoading( true );	

    ::radDriveMount( NULL, GMA_PERSISTENT);

    //
    // Initialize the new movie player
    //
    ::radMovieInitialize2( GMA_PERSISTENT );

    HeapMgr()->PopHeap (GMA_PERSISTENT);
}

//==============================================================================
// SdlPlatform::InitializeMemory
//==============================================================================
//
// Description: 
//
// Parameters:  
//
// Return:      
//
//==============================================================================
void SdlPlatform::InitializeMemory()
{
    //
    // Only do this once!
    //
    if( gMemorySystemInitialized == true )
    {
        return;
    }

    gMemorySystemInitialized = true;

    //
    // Initialize the thread system.
    //
    ::radThreadInitialize();

    //
    // Initialize the memory system.
    //
    ::radMemoryInitialize();
}

//==============================================================================
// SdlPlatform::ShutdownMemory
//==============================================================================
//
// Description: 
//
// Parameters:  
//
// Return:      
//
//==============================================================================
void SdlPlatform::ShutdownMemory()
{
    if( gMemorySystemInitialized )
    {
        gMemorySystemInitialized = false;

        // No shutdown the memory.  This leads to bad errors when destroying 
        // static variables sprinkled here and there.
        //::radMemoryTerminate();
        
        ::radThreadTerminate();
    }
}

//==============================================================================
// SdlPlatform::InitializePlatform
//==============================================================================
// Description: Get the Win32 ready to go.
//
// Parameters:	None.
//
// Return:      None.
//
//==============================================================================
void SdlPlatform::InitializePlatform() 
{
    HeapMgr()->PushHeap (GMA_PERSISTENT);

    //
    // Rendering is good.
    //
    InitializePure3D();

    //
    // Add anything here that needs to be before the drive is opened.
    //
    DisplaySplashScreen( Error ); // blank screen

    //
    // Show in fullscreen if fullscreen flag is set.
    //
    SDL_SetWindowFullscreen( mWnd, mFullscreen ? SDL_WINDOW_FULLSCREEN : 0 );

    //
    // Opening the drive is SLOW...
    //
    InitializeFoundationDrive();    

    //
    // Initialize the controller.
    //
    GetInputManager()->Init();

    HeapMgr()->PopHeap (GMA_PERSISTENT);
}


//==============================================================================
// SdlPlatform::ShutdownPlatform
//==============================================================================
// Description: Shut down the PS2.
//
// Parameters:	None.
//
// Return:      None.
//
//==============================================================================
void SdlPlatform::ShutdownPlatform()
{
    ShutdownPure3D();
    ShutdownFoundation();
}

//=============================================================================
// SdlPlatform::LaunchDashboard
//=============================================================================
// Description: We use this a the emergency exit from the game if we arent in a context that suppose the transition 
//                    to the CONTEXT_EXIT  
// Parameters:  ()
//
// Return:      void 
//
//=============================================================================
void SdlPlatform::LaunchDashboard()
{   

    {
        //chuck I copied and pasted from the other platform's implementations

        GetLoadingManager()->CancelPendingRequests();
           //TODO: Make sure sounds shut down too.
        GetSoundManager()->SetMasterVolume( 0.0f );

       // DisplaySplashScreen( FadeToBlack );

        GetPresentationManager()->StopAll();

        //Shouldn't need to do this since, this singleton and the others should get destroyed once we 
        //retrun the main loop
        //GameDataManager::DestroyInstance();  //Get rid of memcards

        p3d::loadManager->CancelAll();

        GetSoundManager()->StopForMovie();

        //Shouldnt need the early destruction of this singleton either
        //SoundManager::DestroyInstance();
        
        //Dont want to shutdown platform early either.
        //ShutdownPlatform();
        //rAssertMsg( false, "Doesn't make sense for win32." );
    }
}

//=============================================================================
// SdlPlatform::ResetMachine
//=============================================================================
// Description: Comment
//
// Parameters:  ()
//
// Return:      void 
//
//=============================================================================
void SdlPlatform::ResetMachine()
{
    rAssertMsg( false, "Doesn't make sense for win32." );
}

//=============================================================================
// SdlPlatform::DisplaySplashScreen
//=============================================================================
// Description: Comment
//
// Parameters:  ( SplashScreen screenID, 
//                const char* overlayText = NULL, 
//                float fontScale = 1.0f, 
//                float textPosX = 0.0f, 
//                float textPosY = 0.0f,
//                tColour textColour,
//                int fadeFrames = 3 )
//
// Return:      void 
//
//=============================================================================
void SdlPlatform::DisplaySplashScreen( SplashScreen screenID, 
                                       const char* overlayText, 
                                       float fontScale, 
                                       float textPosX, 
                                       float textPosY,
                                       tColour textColour,
                                       int fadeFrames )
{
    HeapMgr()->PushHeap( GMA_TEMP );

    p3d::inventory->PushSection();
    p3d::inventory->AddSection( WIN32_SECTION );
    p3d::inventory->SelectSection( WIN32_SECTION );

    P3D_UNICODE unicodeText[256];

    // Save the current Projection mode so I can restore it later
    pddiProjectionMode pm = p3d::pddi->GetProjectionMode();
    p3d::pddi->SetProjectionMode(PDDI_PROJECTION_DEVICE);

    pddiCullMode cm = p3d::pddi->GetCullMode();
    p3d::pddi->SetCullMode(PDDI_CULL_NONE);


    //CREATE THE FONT
    tTextureFont* thisFont = NULL;

    // Convert memory buffer into a texturefont.
    #include <font/defaultfont.h>
    LoadMemP3DFile( gFont, DEFAULTFONT_SIZE, p3d::inventory );

    thisFont = p3d::find<tTextureFont>("adlibn_20");
    rAssert( thisFont );

    thisFont->AddRef();
    tShader* fontShader = thisFont->GetShader();
    //fontShader->SetInt( )


    p3d::AsciiToUnicode( overlayText, unicodeText, 256 );

    // Make the missing letter into somthing I can see
    //
    thisFont->SetMissingLetter(p3d::ConvertCharToUnicode('j'));

    int a = 0;

    do
    {
        p3d::pddi->SetColourWrite(true, true, true, true);
        p3d::pddi->SetClearColour( pddiColour(0,0,0) );
        p3d::pddi->BeginFrame();
        p3d::pddi->Clear(PDDI_BUFFER_COLOUR);

        //This is for fading in the font and shaders.
        int bright = 255;
        if (a < fadeFrames) bright = (a * 255) / fadeFrames;
        if ( bright > 255 ) bright = 255;
        tColour c(bright, bright, bright, 255);

        //Display font
        if (overlayText != NULL)
        {
            tColour colour = textColour;
            colour.SetAlpha( bright );

            thisFont->SetColour( colour );

            p3d::pddi->SetProjectionMode(PDDI_PROJECTION_ORTHOGRAPHIC);
            p3d::stack->Push();
            p3d::stack->LoadIdentity();
            
            p3d::stack->Translate( textPosX, textPosY, 1.0f);
            float scaleSize = 1.0f / 480.0f;  //This is likely good for 528 also.
            p3d::stack->Scale(scaleSize * fontScale, scaleSize* fontScale , 1.0f);

            if ( textPosX != 0.0f || textPosY != 0.0f )
            {
                thisFont->DisplayText( unicodeText );
            }
            else
            {
                thisFont->DisplayText( unicodeText, 3 );
            }

            p3d::stack->Pop();
        }

        p3d::pddi->EndFrame();
        p3d::context->SwapBuffers();

        ++a;

    } while (a <= fadeFrames);

    p3d::pddi->SetCullMode(cm);
    p3d::pddi->SetProjectionMode(pm);

    //Should do this after a vsync.
    thisFont->Release();

    p3d::inventory->RemoveSectionElements(WIN32_SECTION);
    p3d::inventory->DeleteSection(WIN32_SECTION);
    p3d::inventory->PopSection();

    HeapMgr()->PopHeap( GMA_TEMP );
}


//=============================================================================
// SdlPlatform::DisplaySplashScreen
//=============================================================================
// Description: Comment
//
// Parameters:  ( const char* textureName, 
//                const char* overlayText = NULL, 
//                float fontScale = 1.0f, 
//                float textPosX = 0.0f, 
//                float textPosY = 0.0f, 
//                tColour textColour,
//                int fadeFrames = 3 )
//
// Return:      void 
//
//=============================================================================
void SdlPlatform::DisplaySplashScreen( const char* textureName,
                                       const char* overlayText, 
                                       float fontScale, 
                                       float textPosX, 
                                       float textPosY, 
                                       tColour textColour,
                                       int fadeFrames )
{
}

void SdlPlatform::OnControllerError(const char *msg)
{
    DisplaySplashScreen( Error, msg, 0.7f, 0.0f, 0.0f, tColour(255, 255, 255), 0 );
    mErrorState = CTL_ERROR;
    mPauseForError = true;

}


//=============================================================================
// SdlPlatform::OnDriveError
//=============================================================================
// Description: Comment
//
// Parameters:  ( radFileError error, const char* pDriveName, void* pUserData )
//
// Return:      bool 
//
//=============================================================================
bool SdlPlatform::OnDriveError( radFileError error, const char* pDriveName, void* pUserData )
{
    // First check if the error is related to loading/saving games.
    // We do this here because windows has one drive for all operations.
    // If the game data manager is using the drive, it handles the error.
    GameDataManager* gm = GetGameDataManager();
    if( gm->IsUsingDrive() )
    {
        return gm->OnDriveError( error, pDriveName, pUserData );
    }

    switch ( error )
    {
    case Success:
        {
            if ( mErrorState != NONE )
            {
                DisplaySplashScreen( FadeToBlack );
                mErrorState = NONE;
                mPauseForError = false;
            }

            return true;
            break;
        }
    case FileNotFound:
        {
            rAssert( pUserData != NULL );

            radFileRequest* request = static_cast<radFileRequest*>( pUserData );
            const char* fileName = request->GetFilename();

            //Get rid of the slashes.
            unsigned int i;
            unsigned int lastIndex = 0;
            for ( i = 0; i < strlen( fileName ); ++i )
            {
                if ( fileName[ i ] == '\\' )
                {
                    lastIndex = i;
                }
            }

            unsigned int adjustedIndex = lastIndex == 0 ? lastIndex : lastIndex + 1;

            char adjustedName[32];
            strncpy( adjustedName, &fileName[adjustedIndex], ( strlen( fileName ) - lastIndex ) );
            adjustedName[ strlen( fileName ) - lastIndex ] = '\0';

            if( strcmp( fileName, GameConfigManager::ConfigFilename ) == 0 )
            {
                return false;
            }

            char errorString[256];
            sprintf( errorString, "%s:\n%s", ERROR_STRINGS[error], adjustedName );
            rDebugPrintf( "error_FileNotFound: %s\n", errorString );
            DisplaySplashScreen( Error, errorString, 1.0f, 0.0f, 0.0f, tColour(255, 255, 255), 0 );
            mErrorState = P_ERROR;
            mPauseForError = true;

            return true;
        }
    case NoMedia:
    case MediaNotFormatted:
    case MediaCorrupt:
    case NoFreeSpace:
    case HardwareFailure:
        {
            //This could be the wrong disc.
            rDebugPrintf( "ERROR_HardwareFailure: %s\n", ERROR_STRINGS[error] );
            DisplaySplashScreen( Error, ERROR_STRINGS[error], 1.0f, 0.0f, 0.0f, tColour(255, 255, 255), 0 );
            mErrorState = P_ERROR;
            mPauseForError = true;

            return true;
        }
    default:
        {
            //Others are not supported.
            rAssert( false );
        }
    }

    return false;
}

//=============================================================================
// SdlPlatform::SetResolution
//=============================================================================
// Description: Sets the screen resolution
//
// Parameters:  res - desired resolution
//
// Returns:     true if successful
//              false if not supported
//
// Notes:
//=============================================================================

bool SdlPlatform::SetResolution( Resolution res, int bpp, bool fullscreen )
{
    // Check if resolution is supported.
    if( !mpContext || !IsResolutionSupported( res, bpp ) )
    {
        return false;
    }

    // Set up the new properties
    mResolution = res;
    mbpp = bpp;
    mFullscreen = fullscreen;

    // Reinitialize the d3d context.
    InitializeContext();

    // Resize the window for the new resolution
    ResizeWindow();

    return true;
}

//=============================================================================
// SdlPlatform::GetResolution
//=============================================================================
// Description: Returns the current resolution
//
// Parameters:  n/a
//
// Returns:     resolution
//
// Notes:
//=============================================================================

SdlPlatform::Resolution SdlPlatform::GetResolution() const
{
    return mResolution;
}

//=============================================================================
// SdlPlatform::GetBPP
//=============================================================================
// Description: Returns the current bit depth.
//
// Parameters:  n/a
//
// Returns:     bit depth
//
// Notes:
//=============================================================================

int SdlPlatform::GetBPP() const
{
    return mbpp;
}

//=============================================================================
// SdlPlatform::IsFullscreen
//=============================================================================
// Description: Returns true if currently in full screen mode
//
// Parameters:  n/a
//
// Returns:     true if in full screen, false if in window
//
// Notes:
//=============================================================================

bool SdlPlatform::IsFullscreen() const
{
    return mFullscreen;
}

//=============================================================================
// SdlPlatform::GetConfigName
//=============================================================================
// Description: Returns the name of the win32 platform's config
//
// Parameters:  n/a
//
// Returns:     
//
// Notes:
//=============================================================================

const char* SdlPlatform::GetConfigName() const
{
    return "System";
}

//=============================================================================
// SdlPlatform::GetNumProperties
//=============================================================================
// Description: Returns the number of config properties
//
// Parameters:  n/a
//
// Returns:     
//
// Notes:
//=============================================================================

int SdlPlatform::GetNumProperties() const
{
    return 4;
}

//=============================================================================
// SdlPlatform::LoadDefaults
//=============================================================================
// Description: Loads the default configuration for the system.
//
// Parameters:  n/a
//
// Returns:     
//
// Notes:
//=============================================================================

void SdlPlatform::LoadDefaults()
{
#ifdef RAD_DEBUG
    SetResolution( StartingResolution, StartingBPP, !CommandLineOptions::Get( CLO_WINDOW_MODE ) );
#else
    SetResolution( StartingResolution, StartingBPP, true );
#endif
    

    GetRenderFlow()->SetGamma( 1.0f );
}

//=============================================================================
// SdlPlatform::LoadConfig
//=============================================================================
// Description: Loads the platforms configuration
//
// Parameters:  n/a
//
// Returns:     
//
// Notes:
//=============================================================================

void SdlPlatform::LoadConfig( ConfigString& config )
{
    char property[ ConfigString::MaxLength ];
    char value[ ConfigString::MaxLength ];

    while ( config.ReadProperty( property, value ) )
    {
        if( _stricmp( property, "display" ) == 0 )
        {
            if( _stricmp( value, "window" ) == 0 )
            {
                mFullscreen = false;
            }
            else if( _stricmp( value, "fullscreen" ) == 0 )
            {
                mFullscreen = true;
            }
        }
        else if( _stricmp( property, "resolution" ) == 0 )
        {
            if( strcmp( value, "640x480" ) == 0 )
            {
                mResolution = Res_640x480;
            }
            else if( strcmp( value, "800x600" ) == 0 )
            {
                mResolution = Res_800x600;
            }
            else if( strcmp( value, "1024x768" ) == 0 )
            {
                mResolution = Res_1024x768;
            }
            else if( strcmp( value, "1152x864" ) == 0 )
            {
                mResolution = Res_1152x864;
            }
            else if( strcmp( value, "1280x1024" ) == 0 )
            {
                mResolution = Res_1280x1024;
            }
            else if( strcmp( value, "1600x1200" ) == 0 )
            {
                mResolution = Res_1600x1200;
            }
            else if( strcmp( value, "1920x1080" ) == 0 )
            {
                mResolution = Res_1920x1080;
            }
            else if( strcmp( value, "2560x1440" ) == 0 )
            {
                mResolution = Res_2560x1440;
            }
            else if( strcmp( value, "3840x2160" ) == 0 )
            {
                mResolution = Res_3840x2160;
            }
            else if( strcmp( value, "5120x2880" ) == 0 )
            {
                mResolution = Res_5120x2880;
            }
            else if( strcmp( value, "7680x4320" ) == 0 )
            {
                mResolution = Res_7680x4320;
            }
        }
        else if( _stricmp( property, "bpp" ) == 0 )
        {
            if( strcmp( value, "16" ) == 0 )
            {
                mbpp = 16;
            }
            else if( strcmp( value, "32" ) == 0 )
            {
                mbpp = 32;
            }
        }
        else if( _stricmp( property, "gamma" ) == 0 )
        {
            float val = (float) atof( value );
            if( val > 0 )
            {
                GetRenderFlow()->SetGamma( val );
            }
        }
        else if (_stricmp(property, "renderer") == 0)
        {
            strncpy(mRenderer, value, ConfigString::MaxLength);
        }
    }

    // apply the new settings.
    SetResolution( mResolution, mbpp, mFullscreen );
}

//=============================================================================
// SdlPlatform::SaveConfig
//=============================================================================
// Description: Saves the system configuration to the config string.
//
// Parameters:  config string to save to
//
// Returns:     
//
// Notes:
//=============================================================================

void SdlPlatform::SaveConfig( ConfigString& config )
{
    config.WriteProperty( "display", mFullscreen ? "fullscreen" : "window" );

    const char* res = "800x600";
    switch( mResolution )
    {
        case Res_640x480:
        {
            res = "640x480";
            break;
        }
        case Res_800x600:
        {
            res = "800x600";
            break;
        }
        case Res_1024x768:
        {
            res = "1024x768";
            break;
        }
        case Res_1152x864:
        {
            res = "1152x864";
            break;
        }
        case Res_1280x1024:
        {
            res = "1280x1024";
            break;
        }
        case Res_1600x1200:
        {
            res = "1600x1200";
            break;
        }
        case Res_1920x1080:
        {
            res = "1920x1080";
            break;
        }
        case Res_2560x1440:
        {
            res = "2560x1440";
            break;
        }
        case Res_3840x2160:
        {
            res = "3840x2160";
            break;
        }
        case Res_5120x2880:
        {
            res = "5120x2880";
            break;
        }
        case Res_7680x4320:
        {
            res = "7680x4320";
            break;
        }
        default:
        {
            rAssert( false );
        }
    }

    config.WriteProperty( "resolution", res );

    config.WriteProperty( "bpp", mbpp == 16 ? "16" : "32" );

    char gamma[20];
    sprintf( gamma, "%f", GetRenderFlow()->GetGamma() );
    config.WriteProperty( "gamma", gamma );

    config.WriteProperty("renderer", mRenderer);
}


//******************************************************************************
//
// Private Member Functions
//
//******************************************************************************

//==============================================================================
// SdlPlatform::SdlPlatform
//==============================================================================
// Description: Constructor.
//
// Parameters:	None.
//
// Return:      N/A.
//
//==============================================================================
SdlPlatform::SdlPlatform() :
    mpPlatform( NULL ),
    mpContext( NULL ),
    mResolution( StartingResolution ),
    mbpp( StartingBPP ),
    mRenderer("gl")
{
    mFullscreen = false;
}


//==============================================================================
// SdlPlatform::~SdlPlatform
//==============================================================================
// Description: Destructor.
//
// Parameters:	None.
//
// Return:      N/A.
//
//==============================================================================
SdlPlatform::~SdlPlatform()
{
    HeapManager::DestroyInstance();

#if defined( RAD_WIN32 )
    CloseHandle( mhMutex );
#endif
}

//==============================================================================
// SdlPlatform::InitializeFoundationDrive
//==============================================================================
// Description: Get FTech ready to go.
//
// Parameters:	None.
//
// Return:      None.
//
// Constraints: The FTech systems must be initialized in a particular order.
//              Consult their documentation before changing.
//
//==============================================================================
void SdlPlatform::InitializeFoundationDrive() 
{
    //
    // Get the default drive and hold it open for the life of the game.
    // This is a costly operation so we only want to do it once.
    //

    char defaultDrive[ radFileDrivenameMax + 1 ];

    ::radGetDefaultDrive( defaultDrive );

    ::radDriveOpenSync( &mpIRadDrive, 
                        defaultDrive,
                        NormalPriority, // Default
                        GMA_PERSISTENT );

    rAssert( mpIRadDrive != NULL );

    mpIRadDrive->RegisterErrorHandler( this, NULL );
}


//==============================================================================
// SdlPlatform::ShutdownFoundation
//==============================================================================
// Description: Shut down Foundation Tech.
//
// Parameters:	None.
//
// Return:      None.
//
// Constraints: The FTech systems must be terminated in the reverse order that
//              they were initialized in.
//
//==============================================================================
void SdlPlatform::ShutdownFoundation()
{
    //
    // Release the drive we've held open since the begining.
    //
    mpIRadDrive->Release();
    mpIRadDrive = NULL;

    //
    // Shutdown the systems in the reverse order.
    //
    ::radMovieTerminate2();
    ::radDriveUnmount( NULL );
    ::radLoadTerminate();
    ::radFileTerminate();
    ::radDbgWatchTerminate();
    if( CommandLineOptions::Get( CLO_MEMORY_MONITOR ) )
    {
        ::radMemoryMonitorTerminate();
    }
    ::radDbgComTargetTerminate();
    ::radTimeTerminate();
    ::radPlatformTerminate();
}


//==============================================================================
// SdlPlatform::InitializePure3D
//==============================================================================
// Description: Get Pure3D ready to go.
//
// Parameters:	None.
//
// Return:      None.
//
//==============================================================================
void SdlPlatform::InitializePure3D() 
{
MEMTRACK_PUSH_GROUP( "SdlPlatform" );
    //    p3d::SetMemAllocator( p3d::ALLOC_DEFAULT, GMA_PERSISTENT );
    //    p3d::SetMemAllocator( p3d::ALLOC_LOADED, GMA_LEVEL );

    //
    // Initialise Pure3D platform object.
    // This call differs between different platforms.  The Win32 version,
    // for example requires the application instance to be passed in.
    //
    mpPlatform = tPlatform::Create( mWnd );
    rAssert( mpPlatform != NULL );

    //
    // Initialize the d3d context.
    //
    InitializeContext();

    //
    // This call installs chunk handlers for all the primary chunk types that
    // Pure3D supports.  This includes textures, materials, geometries, and the
    // like.
    //
    //    p3d::InstallDefaultLoaders();
    P3DASSERT(p3d::context);
    tP3DFileHandler* p3d = new(GMA_PERSISTENT) tP3DFileHandler;
    //    p3d::loadManager->AddHandler(p3d, "p3d");
    p3d::context->GetLoadManager()->AddHandler(p3d, "p3d");
    p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tPNGHandler, "png");

    if( CommandLineOptions::Get( CLO_FE_UNJOINED ) )
    {
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tBMPHandler, "bmp");
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tTargaHandler, "tga");
    }
    else
    {
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tBMPHandler, "p3d");
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tPNGHandler, "p3d");
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tTargaHandler, "p3d");
    }

    //    p3d->AddHandler(new tGeometryLoader);
    //    GeometryWrappedLoader* pGWL = new GeometryWrappedLoader;
    GeometryWrappedLoader* pGWL = 
        (GeometryWrappedLoader*)GetAllWrappers()->mpLoader( AllWrappers::msGeometry );
    pGWL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pGWL );

    StaticEntityLoader* pSEL = 
        (StaticEntityLoader*)GetAllWrappers()->mpLoader( AllWrappers::msStaticEntity );
    pSEL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pSEL );

    StaticPhysLoader* pSPL = 
        (StaticPhysLoader*)GetAllWrappers()->mpLoader( AllWrappers::msStaticPhys );
    pSPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pSPL );

    TreeDSGLoader* pTDL = 
        (TreeDSGLoader*)GetAllWrappers()->mpLoader( AllWrappers::msTreeDSG );
    pTDL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pTDL );

    FenceLoader* pFL = 
        (FenceLoader*)GetAllWrappers()->mpLoader( AllWrappers::msFenceEntity );
    pFL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pFL );

    IntersectLoader* pIL = 
        (IntersectLoader*)GetAllWrappers()->mpLoader( AllWrappers::msIntersectDSG );
    pIL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pIL );

    AnimCollLoader* pACL = 
        (AnimCollLoader*)GetAllWrappers()->mpLoader( AllWrappers::msAnimCollEntity );
    pACL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pACL );

    AnimDSGLoader* pAnimDSGLoader = 
        (AnimDSGLoader*)GetAllWrappers()->mpLoader( AllWrappers::msAnimEntity );
    pAnimDSGLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pAnimDSGLoader );


    DynaPhysLoader* pDPL = 
        (DynaPhysLoader*)GetAllWrappers()->mpLoader( AllWrappers::msDynaPhys );
    pDPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pDPL );

    InstStatPhysLoader* pISPL = 
        (InstStatPhysLoader*)GetAllWrappers()->mpLoader( AllWrappers::msInstStatPhys );
    pISPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pISPL );

    InstStatEntityLoader* pISEL = 
        (InstStatEntityLoader*)GetAllWrappers()->mpLoader( AllWrappers::msInstStatEntity );
    pISEL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pISEL );

    LocatorLoader* pLL = 
        (LocatorLoader*)GetAllWrappers()->mpLoader( AllWrappers::msLocator);
    pLL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pLL );

    RoadLoader* pRL = 
        (RoadLoader*)GetAllWrappers()->mpLoader( AllWrappers::msRoadSegment);
    pRL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pRL );

    PathLoader* pPL = 
        (PathLoader*)GetAllWrappers()->mpLoader( AllWrappers::msPathSegment);
    pPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pPL );

    WorldSphereLoader* pWSL = 
        (WorldSphereLoader*)GetAllWrappers()->mpLoader( AllWrappers::msWorldSphere);
    pWSL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pWSL );

    LensFlareLoader* pLSL = 
        (LensFlareLoader*)GetAllWrappers()->mpLoader( AllWrappers::msLensFlare);
    pLSL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pLSL );

    BillboardWrappedLoader* pBWL = 
        (BillboardWrappedLoader*)GetAllWrappers()->mpLoader( AllWrappers::msBillboard);
    pBWL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pBWL );


    InstParticleSystemLoader* pInstParticleSystemLoader = 
        (InstParticleSystemLoader*) GetAllWrappers()->mpLoader( AllWrappers::msInstParticleSystem);
    pInstParticleSystemLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pInstParticleSystemLoader );

    BreakableObjectLoader* pBreakableObjectLoader = 
        (BreakableObjectLoader*) GetAllWrappers()->mpLoader( AllWrappers::msBreakableObject);
    pBreakableObjectLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pBreakableObjectLoader );

    AnimDynaPhysLoader*	pAnimDynaPhysLoader = 
        (AnimDynaPhysLoader*) GetAllWrappers()->mpLoader( AllWrappers::msAnimDynaPhys);
    pAnimDynaPhysLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pAnimDynaPhysLoader );

    AnimDynaPhysWrapperLoader* pAnimWrapperLoader = 
        (AnimDynaPhysWrapperLoader*) GetAllWrappers()->mpLoader( AllWrappers::msAnimDynaPhysWrapper);
    pAnimWrapperLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pAnimWrapperLoader );

    p3d->AddHandler(new(GMA_PERSISTENT) tTextureLoader);
    p3d->AddHandler( new(GMA_PERSISTENT) tSetLoader );
    p3d->AddHandler(new(GMA_PERSISTENT) tShaderLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tCameraLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tGameAttrLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tLightLoader);

    p3d->AddHandler(new(GMA_PERSISTENT) tLocatorLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tLightGroupLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tImageLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tTextureFontLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tImageFontLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tSpriteLoader);
    //p3d->AddHandler(new(GMA_PERSISTENT) tBillboardQuadGroupLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tSkeletonLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tPolySkinLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tCompositeDrawableLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tVertexAnimKeyLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tAnimationLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tFrameControllerLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tMultiControllerLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tAnimatedObjectFactoryLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tAnimatedObjectLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tParticleSystemFactoryLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tParticleSystemLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tLensFlareGroupLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) sg::Loader);

    p3d->AddHandler(new(GMA_PERSISTENT) tExpressionGroupLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tExpressionMixerLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tExpressionLoader);

    //ATCloader, hope this doesnt blow up
    p3d->AddHandler(new(GMA_PERSISTENT) ATCLoader);

    //p3d->AddHandler(new p3d::tIgnoreLoader);

    tSEQFileHandler* sequencerFileHandler = new(GMA_PERSISTENT) tSEQFileHandler;
    p3d::loadManager->AddHandler(sequencerFileHandler, "seq");

    // sim lib
    sim::InstallSimLoaders();

    p3d->AddHandler(new(GMA_PERSISTENT) CameraDataLoader, SRR2::ChunkID::FOLLOWCAM);    
    p3d->AddHandler(new(GMA_PERSISTENT) CameraDataLoader, SRR2::ChunkID::WALKERCAM);    
    p3d->AddHandler(new(GMA_PERSISTENT) IntersectionLoader);    
    //p3d->AddHandler(new(GMA_PERSISTENT) RoadLoader);    
    p3d->AddHandler(new(GMA_PERSISTENT) RoadDataSegmentLoader);    
    p3d->AddHandler(new(GMA_PERSISTENT) CStatePropDataLoader);
MEMTRACK_POP_GROUP( "SdlPlatform" );
}


//==============================================================================
// SdlPlatform::ShutdownPure3D
//==============================================================================
// Description: Clean up and shut down Pure3D.
//
// Parameters:	None.
// 
// Return:      None.
//
//==============================================================================
void SdlPlatform::ShutdownPure3D()
{
    //
    // Clean-up the Pure3D Inventory
    //
    p3d::inventory->RemoveAllElements();
    p3d::inventory->DeleteAllSections();

    //
    // Clean-up the space taken by the Pure 3D context.
    //
    if( mpContext != NULL )
    {
        mpPlatform->DestroyContext( mpContext );
        mpContext = NULL;
    }

    //
    // Clean-up the space taken by the Pure 3D platform.
    //
    if( mpPlatform != NULL )
    {
        tPlatform::Destroy( mpPlatform );
        mpPlatform = NULL;
    }
}

//==============================================================================
// SdlPlatform::InitializeContext
//==============================================================================
// Description: Initializes the d3d context for this application according to
//              the class' display settings - resolution, bpp, fullscreen.
//
// Parameters:	n/a
//
// Return:      n/a
//
//==============================================================================

void SdlPlatform::InitializeContext()
{
    tContextInitData init;

    //
    // This is the window we want to render into.
    //
    init.window = mWnd;

    //
    // Set the fullscreen/window mode.
    //
    init.displayMode = mFullscreen ? PDDI_DISPLAY_FULLSCREEN : PDDI_DISPLAY_WINDOW;

    //
    // All applications should supply PDDI_BUFFER_COLOUR.  PDDI_BUFFER_DEPTH
    // specifies that we also want to allocate a Z-buffer.
    //
    init.bufferMask = PDDI_BUFFER_COLOUR | PDDI_BUFFER_DEPTH;
    init.enableSnapshot = false;

    //
    // These values only take effect in fullscreen mode.  In windowed mode, the
    // dimensions of the window define the rendering area.  We'll define them
    // anyway for completeness sake.
    //
    TranslateResolution( mResolution, init.xsize, init.ysize );

    //
    // Depth of the rendering buffer.  Again, this value only works in
    // fullscreen mode.  In window mode, the depth of the desktop is used.
    // This value should be either 16 or 32.
    //
    init.bpp = mbpp;

    init.lockToVsync = false;

    if( mpContext == NULL )
    {
        //
        // This the name of the PDDI we will be using for rendering
        //
        snprintf(init.PDDIlib, 128, pddiLibraryName, mRenderer);

        // Create the context
        mpContext = mpPlatform->CreateContext( &init );
        rAssert( mpContext != NULL );

        //
        // Assign this context to the platform.
        //
        mpPlatform->SetActiveContext( mpContext );
        p3d::pddi->EnableZBuffer( true );
    }
    else
    {
        // Update the display settings.
        mpContext->GetDisplay()->InitDisplay( &init );
    }
}

//==============================================================================
// SdlPlatform::TranslateResolution
//==============================================================================
// Description: translates resolution enums to x and y
//
// Parameters:	resolution - the res enum
//              x - corresponding width
//              y - corresponding height
//
// Return:      N/A.
//
//==============================================================================

// TODO(3ur): this sucks, dynamic would be nicer
void SdlPlatform::TranslateResolution( Resolution res, int&x, int&y )
{
    switch( res )
    {
        case Res_640x480:
        {
            x = 640;
            y = 480;
            break;
        }
        case Res_800x600:
        {
            x = 800;
            y = 600;
            break;
        }
        case Res_1024x768:
        {
            x = 1024;
            y = 768;
            break;
        }
        case Res_1152x864:
        {
            x = 1152;
            y = 864;
            break;
        }
        case Res_1280x1024:
        {
            x = 1280;
            y = 1024;
            break;
        }
        case Res_1600x1200:
        {
            x = 1600;
            y = 1200;
            break;
        }
        // cant believe im adding onto this slop the way this is handled needs a refactor
        case Res_1920x1080:
        {
            x = 1920;
            y = 1080;
            break;
        }
        case Res_2560x1440:
        {
            x = 2560;
            y = 1440;
            break;
        }
        case Res_3840x2160:
        {
            x = 3840;
            y = 2160;
            break;
        }
        case Res_5120x2880:
        {
            x = 5120;
            y = 2880;
            break;
        }
        case Res_7680x4320:
        {
            x = 7680;
            y = 4320;
            break;
        }
        default:
        {
            rAssert( false );
        }
    }
}

//==============================================================================
// SdlPlatform::IsResolutionSupported
//==============================================================================
// Description: Determines if a resolution is supported on this pc
//
// Parameters:	resolution - the res enum
//
// Return:      true if supported.
//
//==============================================================================

bool SdlPlatform::IsResolutionSupported( Resolution res, int bpp ) const
{
    int x,y;

    TranslateResolution( res, x, y );

    // Get the display info for the device
    pddiDisplayInfo* displays = NULL;
    int num_adapters = mpContext->GetDevice()->GetDisplayInfo( &displays );
    rAssert( num_adapters > 0 );

    // Go through the supported modes and see if we can do it.
    // Ignore the refresh rate - directx uses default.
    for( int i = 0; i < displays[0].nDisplayModes; i++ )
    {
        if( displays[0].modeInfo[i].width == x &&
            displays[0].modeInfo[i].height == y &&
            displays[0].modeInfo[i].bpp == bpp )
        {
            return true;
        }
    }

    return false;
}

//=============================================================================
// SdlPlatform::ResizeWindow
//=============================================================================
// Description: Resizes the app's window based on the current resolution.
//
// Parameters:  n/a
//
// Returns:     n/a
//
// Notes:
//=============================================================================

void SdlPlatform::ResizeWindow()
{
    // If fullscreen, no need to change the window size.
    if( mFullscreen )
    {
        return;
    }

    int w,h;
    TranslateResolution( mResolution, w, h );

    SDL_SetWindowSize( mWnd, w, h );
    SDL_SetWindowPosition( mWnd, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
    SDL_ShowWindow( mWnd );
}

//=============================================================================
// SdlPlatform::ShowTheCursor
//=============================================================================
// Description: Shows or hides the cursor.  Wrapper for the windows ShowCursor
//              function, except it doesn't keep a counter for the number of
//              shows/hides.
//
// Parameters:  show - show cursor
//
// Returns:     n/a
//
// Notes:
//=============================================================================

void SdlPlatform::ShowTheCursor( bool show )
{
    if( mShowCursor != show )
    {        
        mShowCursor = show;
        SDL_ShowCursor( mShowCursor ? SDL_ENABLE : SDL_DISABLE );
    }
}

//=============================================================================
// SdlPlatform::WndProc
//=============================================================================
// Description: The windows os messaging callback for the game.
//              Routes messages to pure3d.
//
// Parameters:  hwnd - handle for window
//                message - message ID
//                wParam - word parameter
//                lParam - long parameter
//
// Returns:     windows result
//
// Notes:
//=============================================================================

int SDLCALL SdlPlatform::WndProc( void * userdata, SDL_Event * event )
{
    SDL_Window * wnd = (SDL_Window *)userdata;

    switch(event->type)
    {
    case SDL_WINDOWEVENT: // WM_ACTIVATEAPP
        {
            //
            // Under Win32, Pure3D needs to get a crack at the Windows messages so
            // it can detect window moving, resizing, and activation.
            //
            p3d::platform->ProcessWindowsMessage( wnd, &event->window );

            InputManager* pInputManager = GetInputManager();

            if( spInstance != NULL && spInstance->mpContext != NULL )
            {
                switch(event->window.event)
                {
                case SDL_WINDOWEVENT_FOCUS_GAINED: // Window is being shown (in focus)
                    {
                        RenderFlow* rf = GetRenderFlow();

                        rf->SetGamma( rf->GetGamma() );
                        if( pInputManager )
                        {
                            //GetInputManager()->SetRumbleForDevice(0, true);
                            //rDebugPrintf("Force Effects Started!!! \n");
                        }
                    }
                    break;

                case SDL_WINDOWEVENT_FOCUS_LOST:  // Window is being hidden (not in focus)
                    SDL_SetWindowGammaRamp( wnd,
                        DesktopGammaRamp[0],
                        DesktopGammaRamp[1],
                        DesktopGammaRamp[2] );
                    if( pInputManager )
                    {
                        //GetInputManager()->SetRumbleForDevice(0, false);
                        //rDebugPrintf("Force Effects Stopped!!! \n");
                    }
                    break;
                case SDL_WINDOWEVENT_LEAVE:
                    GetInputManager()->GetFEMouse()->getCursor()->SetVisible( false );
                    break;
                }

                ShowTheCursor( event->window.event == SDL_WINDOWEVENT_FOCUS_LOST );
            }

            break;
        }

    case SDL_KEYDOWN: // WM_SYSKEYDOWN
    case SDL_KEYUP:   // WM_SYSKEYUP
        {
            //Ignore Alt and F10 keys.
            switch(event->key.keysym.sym) 
            {
            case SDLK_LALT:
            case SDLK_RALT:
            	return 0;
            case SDLK_F10:
            	return 0;
            default: break;
            }
        }

    case SDL_MOUSEMOTION:  
        {
            // For some reason beyond my comprehension WM_MOUSEMOVE seems to be getting called regardless if the
            // mouse moved or not. So let the FEMouse determine if we moved.
            FEMouse* pFEMouse = GetInputManager()->GetFEMouse();
            if( pFEMouse->DidWeMove( event->motion.x, event->motion.y ) )
            {
                int w, h;
                SDL_GetWindowSize( wnd, &w, &h );
                pFEMouse->Move( event->motion.x, event->motion.y, w, h );
            }

            ShowTheCursor( false );

            break;
        }
    case SDL_MOUSEBUTTONDOWN:
        if (event->button.button == SDL_BUTTON_LEFT)
            GetInputManager()->GetFEMouse()->ButtonDown(BUTTON_LEFT);
        //        rDebugPrintf("LEFT MOUSE BUTTON PRESSED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
        break;

    case SDL_MOUSEBUTTONUP:
        if (event->button.button == SDL_BUTTON_LEFT)
            GetInputManager()->GetFEMouse()->ButtonUp(BUTTON_LEFT);
        break;

        // PDDI will sent this message to enable or disable rendering in response to an
        // application level window event.  For example, if the user clicks away from
        // the rendering window, or uses ALT-TAB to select another application, PDDI
        // will tell sent a WM_PDDI_DRAW_ENABLE(0) message.  When the application
        // regains focus, WM_PDDI_DRAW_ENABLE(1) will be sent.
    //case WM_PDDI_DRAW_ENABLE:
        //GetApplication()->EnableRendering(wParam == 1);
        break;

    default:
        break;
    }

    return 1;
}