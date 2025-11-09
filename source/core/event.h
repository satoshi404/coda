#ifndef  __event_h__
#define  __event_h__

#include <stdint.h>

typedef enum
{
    EVENT_TYPE_NONE       = 0,
    EVENT_TYPE_KEYBOARD   = 1,
    EVENT_TYPE_MOUSE      = 2,
    EVENT_TYPE_WINDOW     = 3,
} EventType;

typedef enum
{
    KEY_STATE_UP     = 0,
    KEY_STATE_DOWN   = 1,
} KeyState;

typedef enum
{
    MOUSE_BUTTON_LEFT     = 0,
    MOUSE_BUTTON_RIGHT    = 1,
    MOUSE_BUTTON_MIDDLE   = 2,
} MouseButton;

typedef struct 
{
    EventType type;
    uint32_t timestamp;
    int x;
    int y;
    MouseButton button;
} MouseEvent;

typedef struct 
{
    EventType type;
    uint32_t timestamp;
    int keycode;
    KeyState state;
} KeyboardEvent;

typedef struct
{
    EventType type;
    uint32_t timestamp;
    int width;
    int height;
} WindowEvent;

typedef struct 
{
    EventType type;
    void (*callback)(const void* event);
} CallbackEntry;

extern void event_registerCallback(EventType type, void (*callback)(const void* event));
extern void event_unregisterCallback(EventType type, void (*callback)(const void* event));
extern void event_dispatch(const void* event);
extern void event_init();
extern void event_shutdown();


#endif // ! __event_h__