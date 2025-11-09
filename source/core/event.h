#ifndef  __event_h__
#define  __event_h__

#include <core/types.h>

#define __namespace( func_name ) core##_##Event##func_name

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
    EVENT_TYPE_NONE       = 0,
    EVENT_TYPE_KEYBOARD   = 1,
    EVENT_TYPE_MOUSE      = 2,
    EVENT_TYPE_WINDOW     = 3,
} EventType;

typedef enum
{
    KEY_STATE_UP         = 0,
    KEY_STATE_DOWN       = 1,
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
    u32 timestamp;
    i32 x;
    i32 y;
    MouseButton button;
} MouseEvent;

typedef struct 
{
    EventType type;
    u32 timestamp;
    i32 keycode;
    KeyState state;
} KeyboardEvent;

typedef struct
{
    EventType type;
    u32 timestamp;
    i32 width;
    i32 height;
} WindowEvent;

typedef struct 
{
    EventType type;
    void ( *callback ) ( const void* event );
} CallbackEntry;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern u8   __namespace( Init )      ( void );
extern void __namespace( Shutdown )  ( void );

extern void __namespace( Dispache )           ( const void* event );
extern void __namespace( RegisterCallback )   ( EventType type, void ( *callback ) ( const void* event ) );
extern void __namespace( UnregisterCallback ) ( EventType type, void ( *callback ) ( const void* event ) );

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef __namespace

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ! __event_h__