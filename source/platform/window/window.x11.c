#include <platform/window/factory.h>
#include <platform/glx/factory.h>
#include <pipe.h>
#include <core/debug.h>
#include <core/event.h>
#include <platform/window/setup.h>

#if PIPE_LINUX

#define __namespace( func_name ) platform##_##Window##func_name

static PlatformWindowState windowState = {0};

#include <string.h>
#include <time.h>

#define CLOCK_MONOTONIC ( 1 )

static u32 getTimestamp( void )
{
    struct timespec ts;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    return ( u32 )( ts.tv_sec * 1000 + ts.tv_nsec / 1000000 );
}

//extern u8 platform_RenderCreateContext(void);

PlatformWindowRendererContext ctx = {0};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 __namespace( Init ) ( void )
{
    //PlatformContext* ctx = platform_GetContext();

    windowState.Width              = DEFAULT_WINDOW_WIDTH;
    windowState.Height             = DEFAULT_WINDOW_HEIGHT;
    windowState.Caption            = ( i_char* ) DEFAULT_CAPTION;
    windowState.Running            = True;
    windowState.NativeWindowHandle = NULL;

    //if (!platform_ContextInit() ) {
    //    return False;
    //}

    if ( !ctx.display ) {
        ctx.display = XOpenDisplay( NULL );
        CHECK_NULL_RET( ctx.display, False );
    }
    
    ctx.screen = DefaultScreen( ctx.display );
    
    if (!platform_GlxInit()) {
        LOG_ERROR("Failed to initialize render backend");
        XCloseDisplay(ctx.display);
        ctx.display = NULL;
        return False;
    }

    ctx.colormap = XCreateColormap(
        ctx.display,
        RootWindow(ctx.display, ctx.screen),
        ctx.visual->visual, 
        AllocNone
    );

    XSetWindowAttributes attrs = {0};
    
    attrs.event_mask =
        ExposureMask       | KeyPressMask   | KeyReleaseMask   |
        StructureNotifyMask| ButtonPressMask| ButtonReleaseMask|
        PointerMotionMask  | FocusChangeMask;

    attrs.background_pixel = BlackPixel(ctx.display, ctx.screen);
    attrs.border_pixel     = BlackPixel(ctx.display, ctx.screen);
    attrs.colormap         = ctx.colormap;
    
    ctx.window = XCreateWindow( 
        ctx.display, 
        RootWindow(ctx.display, ctx.screen), 
        windowState.x, windowState.y, 
        windowState.Width, 
        windowState.Height, 
        0, 
        ctx.visual->depth,
        InputOutput, 
        ctx.visual->visual,
        CWBackPixel | CWBorderPixel | CWColormap | CWEventMask,
        &attrs
    );
    
    windowState.NativeWindowHandle = ( ptr )( uintptr_t )ctx.window;
    
    ctx.wmDeleteMessage = XInternAtom( ctx.display, "WM_DELETE_WINDOW", False );
    XSetWMProtocols( ctx.display, ctx.window, &ctx.wmDeleteMessage, 1 );
    
    XStoreName( ctx.display, ctx.window, (char*)windowState.Caption );
    XMapWindow( ctx.display, ctx.window );
    XFlush( ctx.display );

    if (!platform_GlxCreateContext()) {
        LOG_ERROR("Failed to create render context");
        XDestroyWindow(ctx.display, ctx.window);
        XCloseDisplay(ctx.display);
        return False;
    }
    
    LOG_INFO("Window initialized: %dx%d", windowState.Width, windowState.Height);
    
    return True;
}

void __namespace( Free ) ( void )
{
   // PlatformContext* ctx = platform_GetContext();
    
    windowState.Running = False;
    
    if (ctx.display) {
        XCloseDisplay( ctx.display );
        ctx.display = NULL;
    }
    
    platform_GlxCleanUp();
    //platform_ContextFree();
    
    LOG_INFO( "Window_Platform ( Free )" );
}

void __namespace( Poll ) ( void )
{
    //PlatformContext* ctx = platform_GetContext();
    ASSERT( ctx.display != NULL );

    XEvent event;
    while ( XPending( ctx.display ) ) 
    {
        XNextEvent( ctx.display, &event );

        switch ( event.type )
        {
            case KeyPress:
            case KeyRelease:
            {
                KeyboardEvent kbEvent;
                kbEvent.type = EVENT_TYPE_KEYBOARD;
                kbEvent.timestamp = getTimestamp();
                kbEvent.keycode = event.xkey.keycode;
                kbEvent.state = (event.type == KeyPress) ? KEY_STATE_DOWN : KEY_STATE_UP;

                core_EventDispache( &kbEvent );

                if ( event.type == KeyPress && event.xkey.keycode == 9 ) 
                    windowState.Running = False;

                break;
            }

            case ConfigureNotify:
            {
                if (event.xconfigure.width != windowState.Width ||
                    event.xconfigure.height != windowState.Height)
                {
                    windowState.Width = event.xconfigure.width;
                    windowState.Height = event.xconfigure.height;
                    windowState.x = event.xconfigure.x;
                    windowState.y = event.xconfigure.y;
                    
                    WindowEvent winEvent;
                    winEvent.type = EVENT_TYPE_WINDOW;
                    winEvent.timestamp = getTimestamp();
                    winEvent.width = event.xconfigure.width;
                    winEvent.height = event.xconfigure.height;
                    
                    core_EventDispache( &winEvent );
                    
                    platform_GlxResize(windowState.Width, windowState.Height);
                }
                break;
            }

            case ButtonPress:
            case ButtonRelease:
            {
                MouseEvent mouseEvent;
                mouseEvent.type = EVENT_TYPE_MOUSE;
                mouseEvent.timestamp = getTimestamp();
                mouseEvent.x = event.xbutton.x;
                mouseEvent.y = event.xbutton.y;
                
                switch ( event.xbutton.button )
                {
                    case Button1:
                        mouseEvent.button = MOUSE_BUTTON_LEFT;
                        break;
                    case Button2:
                        mouseEvent.button = MOUSE_BUTTON_MIDDLE;
                        break;
                    case Button3:
                        mouseEvent.button = MOUSE_BUTTON_RIGHT;
                        break;
                    default:
                        mouseEvent.button = MOUSE_BUTTON_LEFT;
                        break;
                }
                
                core_EventDispache( &mouseEvent );
                break;
            }
            
            case MotionNotify:
            {
                MouseEvent mouseEvent;
                mouseEvent.type = EVENT_TYPE_MOUSE;
                mouseEvent.timestamp = getTimestamp();
                mouseEvent.x = event.xmotion.x;
                mouseEvent.y = event.xmotion.y;
                mouseEvent.button = MOUSE_BUTTON_LEFT; 
                
                core_EventDispache( &mouseEvent );
                break;
            }
            
            case ClientMessage:
            {
                if ((Atom)event.xclient.data.l[0] == ctx.wmDeleteMessage) {
                    windowState.Running = False;
                }
                break;
            }
            
            default:
                break;
        }
    }
}

const i_char* __namespace( GetCaption ) ( void )
{
    return windowState.Caption;
}

i32 __namespace( GetHeight ) ( void ) 
{
    return windowState.Height;
}

i32 __namespace( GetWidth ) ( void ) 
{
    return windowState.Width;
}

PlatformWindowState __namespace( GetState )  ( void )
{
    return windowState;
}

void __namespace( Shutdown ) ( void )
{
    //PlatformContext* ctx = platform_GetContext();
    
    platform_GlxCleanUp();
    //platform_RenderCleanUp();
    
    if (ctx.display && ctx.window) {
        XDestroyWindow(ctx.display, ctx.window);
        XFlush(ctx.display);
    }

    LOG_INFO("Platform Window ( ShutDown )");
}

u8  __namespace( IsRunning ) ( void ) 
{
    return windowState.Running; 
}

void __namespace( SwapBuffers ) ( void )
{

    platform_GlxSwapBuffers();
    //platform_RenderSwapBuffers();
}

void __namespace( SetSize ) ( const i32 width, const i32 height )
{
   // PlatformContext* ctx = platform_GetContext();
    
    windowState.Width  = width;
    windowState.Height = height;
    
    if (ctx.display && ctx.window) {
        XResizeWindow(ctx.display, ctx.window, width, height);
        XFlush(ctx.display);
    }
    
    LOG_INFO("Window size set to %dx%d", width, height);
}

void __namespace( GetNativeWindowHandle ) ( ptr* handle )
{
    if ( handle ) {
        *handle = windowState.NativeWindowHandle;
    }
}

void __namespace( SetHeight ) ( u32 height )
{
    windowState.Height = height;
}

void __namespace( SetWidth ) ( u32 width )
{
    windowState.Width = width;
}

void __namespace( SetPos ) ( const u32 x, const u32 y )
{
    windowState.x = x;
    windowState.y = y;
}

#undef __namespace

#endif /* PIPE_LINUX */