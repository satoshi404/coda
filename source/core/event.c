#include "event.h"
#include <stdlib.h>
#include <string.h>

#define MAX_CALLBACKS 32

static struct {
    CallbackEntry callbacks[MAX_CALLBACKS];
    int count;
    int initialized;
} eventSystem = {0};

void event_init()
{
    if (eventSystem.initialized) {
        return;
    }
    
    memset(&eventSystem, 0, sizeof(eventSystem));
    eventSystem.initialized = 1;
}

void event_shutdown()
{
    if (!eventSystem.initialized) {
        return;
    }
    
    memset(&eventSystem, 0, sizeof(eventSystem));
    eventSystem.initialized = 0;
}

void event_registerCallback(EventType type, void (*callback)(const void* event))
{
    if (!eventSystem.initialized || !callback) {
        return;
    }
    
    if (eventSystem.count >= MAX_CALLBACKS) {
        return; // Max callbacks reached
    }
    
    // Check if callback already registered for this type
    for (int i = 0; i < eventSystem.count; i++) {
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

void event_unregisterCallback(EventType type, void (*callback)(const void* event))
{
    if (!eventSystem.initialized || !callback) {
        return;
    }
    
    // Find and remove callback
    for (int i = 0; i < eventSystem.count; i++) {
        if (eventSystem.callbacks[i].type == type && 
            eventSystem.callbacks[i].callback == callback) {
            
            // Shift remaining callbacks
            for (int j = i; j < eventSystem.count - 1; j++) {
                eventSystem.callbacks[j] = eventSystem.callbacks[j + 1];
            }
            
            eventSystem.count--;
            return;
        }
    }
}

void event_dispatch(const void* event)
{
    if (!eventSystem.initialized || !event) {
        return;
    }
    
    // Get event type from the event structure
    // All event types have 'type' as first member
    EventType type = *((EventType*)event);
    
    // Call all registered callbacks for this event type
    for (int i = 0; i < eventSystem.count; i++) {
        if (eventSystem.callbacks[i].type == type) {
            eventSystem.callbacks[i].callback(event);
        }
    }
}