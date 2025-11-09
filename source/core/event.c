#include "event.h"
#include <stdlib.h>
#include <string.h>

#include <core/debug.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define __namespace( func_name ) core##_##Event##func_name

#define MAX_CALLBACKS 32

static struct 
{
    CallbackEntry callbacks[MAX_CALLBACKS];
    i32 count;
    u8 initialized;
} eventSystem = {0};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 __namespace( Init ) ( )
{
    ASSERT( eventSystem.initialized != True );
    
    memset( &eventSystem, 0, sizeof( eventSystem ) );
    eventSystem.initialized = True;

    return True;
}

void __namespace( Shutdown ) ()
{
 //   ASSERT( eventSystem.initialized != True );
   
    memset(&eventSystem, 0, sizeof(eventSystem));
    eventSystem.initialized = False;
}

void __namespace( RegisterCallback ) ( EventType type, void ( *callback ) ( const void* event ) )
{
    ASSERT( eventSystem.initialized != True || callback );
   
    
    if (eventSystem.count >= MAX_CALLBACKS) {
        return; // Max callbacks reached
    }
    
    // Check if callback already registered for this type
    for (i32 i = 0; i < eventSystem.count; i++) 
    {
        if (eventSystem.callbacks[i].type == type && 
            eventSystem.callbacks[i].callback == callback) {
            return; // Already registered
        }
    }
    
    // Add new callback
    eventSystem.callbacks[eventSystem.count].type = type;
    eventSystem.callbacks[eventSystem.count].callback = callback;
    eventSystem.count++;
}

void __namespace( UnregisterCallback ) ( EventType type, void ( *callback ) ( const void* event ) )
{
    ASSERT( eventSystem.initialized != True || callback  );

    // Find and remove callback
    for ( i32 i = 0; i < eventSystem.count; i++ ) 
    {
        if ( eventSystem.callbacks[i].type == type && 
            eventSystem.callbacks[i].callback == callback ) 
            {
            
            // Shift remaining callbacks
            for ( i32 j = i; j < eventSystem.count - 1; j++ ) 
            {
                eventSystem.callbacks[j] = eventSystem.callbacks[j + 1];
            }
            
            eventSystem.count--;
            return;
        }
    }
}

void __namespace( Dispache )  ( const void* event )
{
    ASSERT( eventSystem.initialized != True || event );

    EventType type = *( ( EventType* )event );
    
    // Call all registered callbacks for this event type
    for ( i32 i = 0; i < eventSystem.count; i++ )
    {
        if ( eventSystem.callbacks[i].type == type ) 
        {
            eventSystem.callbacks[i].callback( event );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef __namespace

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////