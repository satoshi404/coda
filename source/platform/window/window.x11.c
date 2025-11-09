
#include <platform/window/factory.h>

#include <pipe.h>
#include <core/debug.h>
#include <core/event.h>
#include <platform/window/setup.h>

#if PIPE_LINUX

/* Use namespace */
#define __namespace( func_name ) platform##_##Window##func_name

static PlatformWindowState windowState = {0};

static Display*     display         = NULL;
static XVisualInfo *visual          = NULL;
static Window       window          = 0;
static Atom         wmDeleteMessage = 0;

//static GLXContext glContext = NULL;

#include <string.h>

#define CLOCK_MONOTONIC ( 1 )
#include <time.h>

static u32 getTimestamp( void )
{
    struct timespec ts;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    return ( u32 )( ts.tv_sec * 1000 + ts.tv_nsec / 1000000 );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 __namespace( Init ) ( void )
{
    windowState.Width              = DEFAULT_WINDOW_WIDTH;
    windowState.Height             = DEFAULT_WINDOW_HEIGHT;
    windowState.Caption            = ( i_char* ) DEFAULT_CAPTION;
    windowState.Running            = True;
    windowState.NativeWindowHandle = NULL;

    if ( !display ) {
        display = XOpenDisplay( NULL );
        CHECK_NULL_RET( display, False );   }
    
    // TODO: Support OPENGL || VULKAN
    visual = ( XVisualInfo* ) malloc( sizeof( XVisualInfo ) );
    visual->depth = 0;
    visual->visual = NULL;

    int screen = DefaultScreen( display );
    visual     = XGetVisualInfo(
        display, 
        VisualScreenMask,
        &(XVisualInfo) { .screen = screen }, 
        &(int){1}
    );
    
    if (!visual) { LOG_ERROR( "Failed to get default visual" ); return False; }

    /* ---- window attributes ------------------------------------------ */
    XSetWindowAttributes attrs = {0};
    
    attrs.event_mask = 
        ExposureMask       | KeyPressMask   | KeyReleaseMask   |
        StructureNotifyMask| ButtonPressMask| ButtonReleaseMask|
        PointerMotionMask  | FocusChangeMask;

    attrs.background_pixel = WhitePixel(display, screen);
    attrs.border_pixel     = BlackPixel(display, screen);
    attrs.colormap         = XCreateColormap(
                                            display,
                                            RootWindow(display, screen),
                                            visual->visual, 
                                            AllocNone
                            );
    
    window = XCreateWindow( 
        display, 
        DefaultRootWindow(display), 
        0, 0, 
        windowState.Width, 
        windowState.Height, 
        0, 
        visual->depth,
        InputOutput, 
        visual->visual,
        CWBackPixel | CWBorderPixel | CWColormap | CWEventMask,
        &attrs
    );
    
    windowState.NativeWindowHandle = ( ptr ) ( uintptr_t )window;
    
    XSelectInput(   
        display, window, 
        KeyPressMask        | KeyReleaseMask    |        // Keyboard events
        StructureNotifyMask |                            // Window resize events
        ExposureMask        |                            // Window expose events
        ButtonPressMask     | ButtonReleaseMask |        // Mouse button events
        PointerMotionMask 
    );

    wmDeleteMessage = XInternAtom( display, "WM_DELETE_WINDOW", False );
    XSetWMProtocols( 
        display, 
        window, 
        &wmDeleteMessage, 
        1
    );

    XStoreName( display, window, (char*)windowState.Caption );
    XMapWindow( display, window );

    return True;
}

void __namespace( Free ) ( void )
{
    ASSERT( display != NULL );

    windowState.Running = False;
    
    XCloseDisplay( display) ;
    display = NULL;

    /* Check again  */
    if ( visual != NULL ) XFree( visual );
    
    LOG_INFO( "Window_Platform ( Free )" );
}

void __namespace( Poll ) ( void )
{
    ASSERT( display != NULL );

    XEvent event;
    while ( XPending( display ) ) 
    {
        XNextEvent( display, &event );

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

                if ( event.type == KeyPress && event.xkey.keycode == 9 ) windowState.Running = False;
                
                break;
            }

            case ConfigureNotify:
            {
                if (event.xconfigure.width != windowState.Width ||
                    event.xconfigure.height != windowState.Height)
                {
                    windowState.Width = event.xconfigure.width;
                    windowState.Height = event.xconfigure.height;
                    
                    WindowEvent winEvent;
                    winEvent.type = EVENT_TYPE_WINDOW;
                    winEvent.timestamp = getTimestamp();
                    winEvent.width = event.xconfigure.width;
                    winEvent.height = event.xconfigure.height;
                    
                    core_EventDispache( &winEvent );
                    
                    // TODO: glx make this
                        // ** Update viewport
                        // ** glViewport(0, 0, windowState.Width, windowState.Height);
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
                if ((Atom)event.xclient.data.l[0] == wmDeleteMessage) {
                    windowState.Running = False;
                }
                break;
            }
            
            default:
                break;
        }
    }

    //LOG_INFO( "Window_Platform ( Poll )" );
}

const i_char* __namespace( GetCaption ) ( void )
{
    return windowState.Caption;
}

i32 __namespace( GetHeight ) ( void ) 
{
    ASSERT( display != NULL );
    return windowState.Height;
}

i32 __namespace( GetWidth ) ( void ) 
{
    ASSERT( display != NULL );
    return windowState.Width;
}

PlatformWindowState __namespace( GetState )  ( void )
{
    ASSERT( display != NULL );
    return windowState;
}

void __namespace( Shutdown ) ( void )
{
    ASSERT( display != NULL );    

    if (display) {
        XDestroyWindow(display, window);
        XFlush(display);
    }

    if ( visual != NULL ) 
    {
        XFree( visual );
        visual = NULL;
    }

    LOG_INFO("Platform Window ( ShutDown )");
}

u8  __namespace( IsRunning ) ( void ) 
{
    ASSERT( display != NULL );
    return windowState.Running; 
}

void __namespace( SwapBuffers ) ( void )
{
    ASSERT( display != NULL );
    // /glXSwapBuffers( display, window );
}

void __namespace( SetWindowSize ) ( const i32 width, const i32 height )
{
    ASSERT( display != NULL );

    windowState.Width  = width;
    windowState.Height = height;

     if (display && window) {
        XResizeWindow(display, window, width, height);
        XFlush(display);
    }
    
    LOG_INFO("Window size set to %dx%d", width, height);
}

void __namespace( GetNativeWindowHandle ) ( ptr* handle )
{
    ASSERT( display != NULL );

    if ( handle )
    {
        *handle = windowState.NativeWindowHandle;
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef __namespace

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif