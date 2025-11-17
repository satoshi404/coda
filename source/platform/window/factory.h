#ifndef __platform_window_factory_h__
#define __platform_window_factory_h__


#include <pipe.h>

typedef struct 
{
    #if PIPE_LINUX
        Display*       display;
        Window         window;
        int            screen;
        XVisualInfo*   visual;
        Colormap       colormap;
        Atom           wmDeleteMessage;
    #else
        HINSTANCE      hInstance;
        HWND           hwnd;
        HDC            hdc;
    #endif
} PlatformWindowRendererContext;


#include <core/types.h>

#define __namespace( func_name ) platform##_##Window##func_name

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct_name ( PlatformWindowState )
{
    i_char*  Caption;
    i32      Width;
    i32      Height;
    u32      x;
    u32      y;
    u8       Running;
    ptr      NativeWindowHandle;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern u8                  __namespace( Init )                 ( void );
extern void                __namespace( Free )                 ( void );
extern void                __namespace( Poll )                 ( void );
extern PlatformWindowState __namespace( GetState )             ( void );
extern void                __namespace( Shutdown )             ( void );
extern u8                  __namespace( IsRunning )            ( void );
extern const i_char*       __namespace( GetCaption )           ( void );
extern i32                 __namespace( GetHeight )            ( void );
extern i32                 __namespace( GetWidth )             ( void );
extern void                __namespace( SetHeight )            ( u32  );
extern void                __namespace( SetWidth )             ( u32  );
extern void                __namespace( SwapBuffers )          ( void );

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* State */
extern void __namespace( SetState )              ( const PlatformWindowState );

/* IsRunning */
extern void __namespace( SetRunning )            ( const u8 );

/* Width, Height */
extern void __namespace( SetSize )         ( const i32, const i32 );

/* X, Y */
extern void __namespace( SetPos )         ( const u32, const u32 );

/* Handle */
extern void __namespace( GetNativeWindowHandle ) ( ptr* );


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef __namespace

#endif /* __platform_window_factory_h__ */