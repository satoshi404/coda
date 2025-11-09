#include <platform/window/factory.h>
#include <core/event.h>
#include <core/debug.h>
#include <core/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

// Global state for cleanup tracking

static struct  
{
    u8 EventsInitialized;
    u8 WindowInitialized;
    u8 CallbacksRegistered;
} ClearUpState = {0} ;

void onKeyboardEvent(const void* event)
{
    const KeyboardEvent* kbEvent = (const KeyboardEvent*)event;
    if (kbEvent->state == KEY_STATE_DOWN) {
        LOG_DEBUG("Key pressed: %d (timestamp: %u)", 
                  kbEvent->keycode, kbEvent->timestamp);
    }
}

void onWindowEvent(const void* event)
{
    const WindowEvent* winEvent = (const WindowEvent*)event;
    LOG_DEBUG("Window resized: %dx%d", winEvent->width, winEvent->height);
}

// Cleanup function: properly handles all cleanup in reverse initialization order
static void cleanUp( char* _cleanup_var )
{
    UNUSED( _cleanup_var );
    
    LOG_INFO( "Starting cleanup..." );
    
    // Cleanup in reverse order of initialization
    if ( ClearUpState.CallbacksRegistered ) 
    {

        core_EventUnregisterCallback( EVENT_TYPE_KEYBOARD , onKeyboardEvent );
        core_EventUnregisterCallback( EVENT_TYPE_WINDOW   , onWindowEvent   );
        ClearUpState.CallbacksRegistered = False;

        LOG_DEBUG( "Event callbacks unregistered" );
    }
    
    if ( ClearUpState.WindowInitialized ) 
    {
        platform_WindowShutdown();
        platform_WindowFree();
        ClearUpState.WindowInitialized = False;
        LOG_DEBUG( "Window shutdown complete" );
    }
    
    if ( ClearUpState.EventsInitialized ) 
    {
        core_EventShutdown();
        ClearUpState.EventsInitialized = False;
        LOG_DEBUG( "Event system shutdown complete" );
    }
    
    LOG_INFO( "Cleanup complete" );
}

static void coda_load( )
{   
    // Initialize event system
    if ( !core_EventInit() )
    {
        LOG_FATAL( "Failed to initialize event" );
    }
    
    ClearUpState.EventsInitialized = True;
    
    LOG_INFO("Event system initialized");

    core_EventRegisterCallback( EVENT_TYPE_KEYBOARD, onKeyboardEvent );
    core_EventRegisterCallback( EVENT_TYPE_WINDOW  , onWindowEvent   );
    
    ClearUpState.CallbacksRegistered = True;
    
    LOG_INFO("Event callbacks registered");
  
    if ( !platform_WindowInit() )
    {
        LOG_FATAL("Failed to initialize window!");
    }
    
    ClearUpState.WindowInitialized = True;
    
    LOG_INFO( "Name: %s", platform_WindowGetCaption() );
    LOG_INFO( "OS: %s\n", PIPE_NAME );
    LOG_INFO( "Window size: %dx%d", platform_WindowGetWidth(), platform_WindowGetHeight() );
    LOG_INFO( "Press ESC to exit" );
}

static inline Bool coda_runtime( )
{
    u32 frameCount = 0;
    u32 lastLogFrame = 0;
    
    // Main loop
    while ( platform_WindowIsRunning() ) 
    {
        platform_WindowPoll();
        platform_WindowSwapBuffers();

        frameCount++;
        
        // Optional: Print frame count periodically
        if (frameCount - lastLogFrame >= 60 ) {
            LOG_TRACE("Frame: %u", frameCount);
            lastLogFrame = frameCount;
        }

        /* Cpu */
        usleep( 16666 );
    }
   
    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    UNUSED(argc);
    UNUSED(argv);
    
    char _auto_cleanup_var __attribute__( ( cleanup( cleanUp ) ) );
    UNUSED(_auto_cleanup_var);

    LOG_INFO("=== Application Starting ===");

    coda_load();
   
    LOG_INFO("Application closed successfully");
    
    return coda_runtime();    
}