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
static void scope_cleanup(char* _cleanup_var)
{
    UNUSED(_cleanup_var);
    
    LOG_INFO("Starting cleanup...");
    
    // Cleanup in reverse order of initialization
    if (ClearUpState .CallbacksRegistered) {
        event_unregisterCallback(EVENT_TYPE_KEYBOARD, onKeyboardEvent);
        event_unregisterCallback(EVENT_TYPE_WINDOW  , onWindowEvent);
        ClearUpState.CallbacksRegistered = False;
        LOG_DEBUG("Event callbacks unregistered");
    }
    
    if (ClearUpState.WindowInitialized) {
        platform_WindowShutdown();
        platform_WindowFree();
        ClearUpState.WindowInitialized = False;
        LOG_DEBUG("Platform shutdown complete");
    }
    
    if (ClearUpState.EventsInitialized) {
        event_shutdown();
        ClearUpState.EventsInitialized = False;
        LOG_DEBUG("Event system shutdown complete");
    }
    
    LOG_INFO("Cleanup complete");
}

int main(int argc, char** argv)
{
    UNUSED(argc);
    UNUSED(argv);
    
    // Declare cleanup guard at the start of main
    // This ensures cleanup runs when main exits (normally or via error)
    char _auto_cleanup_var __attribute__((cleanup(scope_cleanup)));
    UNUSED(_auto_cleanup_var);
    
    LOG_INFO("=== Application Starting ===");
    
    // Initialize event system
    event_init();
    ClearUpState.EventsInitialized = True;
    LOG_INFO("Event system initialized");
    
    // Register event callbacks
    event_registerCallback(EVENT_TYPE_KEYBOARD, onKeyboardEvent);
    event_registerCallback(EVENT_TYPE_WINDOW, onWindowEvent);
    ClearUpState.CallbacksRegistered = True;
    LOG_INFO("Event callbacks registered");
    
    // Initialize platform
    if ( !platform_WindowInit() )
    {
        LOG_FATAL("Failed to initialize platform!");
        return EXIT_FAILURE;  // Cleanup will run automatically
    }
    ClearUpState.WindowInitialized = True;
    
    LOG_INFO("Platform: %s", platform_WindowGetCaption() );
    LOG_INFO("Window size: %dx%d", platform_WindowGetWidth(), platform_WindowGetHeight());
    LOG_INFO("Press ESC to exit");
    
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
    
    LOG_INFO("Application closing after %u frames", frameCount);
    LOG_INFO("Application closed successfully");
    
    return EXIT_SUCCESS;
    
    // Cleanup runs automatically here via __attribute__((cleanup))
}