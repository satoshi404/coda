#include <platform/window/factory.h>
#include <pipe.h>
#include <core/debug.h>
#include <core/event.h>
#include <platform/window/setup.h>

#if PIPE_WINDOWS

#define __namespace( func_name ) platform##_##Window##func_name

static PlatformWindowState windowState = {0};

/* Window Procedure */
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            windowState.Running = False;
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        case WM_SIZE:
        {
            windowState.Width = LOWORD(lParam);
            windowState.Height = HIWORD(lParam);
            
            WindowEvent winEvent;
            winEvent.type = EVENT_TYPE_WINDOW;
            winEvent.timestamp = GetTickCount();
            winEvent.width = windowState.Width;
            winEvent.height = windowState.Height;
            
            core_EventDispache(&winEvent);
            
            platform_GlxResize(windowState.Width, windowState.Height);
            
            return 0;
        }
        
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            KeyboardEvent kbEvent;
            kbEvent.type = EVENT_TYPE_KEYBOARD;
            kbEvent.timestamp = GetTickCount();
            kbEvent.keycode = (u32)wParam;
            kbEvent.state = (uMsg == WM_KEYDOWN) ? KEY_STATE_DOWN : KEY_STATE_UP;
            
            core_EventDispache(&kbEvent);
            
            if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE)
                windowState.Running = False;
                
            return 0;
        }
        
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        {
            MouseEvent mouseEvent;
            mouseEvent.type = EVENT_TYPE_MOUSE;
            mouseEvent.timestamp = GetTickCount();
            mouseEvent.x = GET_X_LPARAM(lParam);
            mouseEvent.y = GET_Y_LPARAM(lParam);
            
            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP)
                mouseEvent.button = MOUSE_BUTTON_LEFT;
            else if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP)
                mouseEvent.button = MOUSE_BUTTON_RIGHT;
            else
                mouseEvent.button = MOUSE_BUTTON_MIDDLE;
                
            core_EventDispache(&mouseEvent);
            return 0;
        }
        
        case WM_MOUSEMOVE:
        {
            MouseEvent mouseEvent;
            mouseEvent.type = EVENT_TYPE_MOUSE;
            mouseEvent.timestamp = GetTickCount();
            mouseEvent.x = GET_X_LPARAM(lParam);
            mouseEvent.y = GET_Y_LPARAM(lParam);
            mouseEvent.button = MOUSE_BUTTON_LEFT;
            
            core_EventDispache(&mouseEvent);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 __namespace( Init ) ( void )
{
    PlatformContext* ctx = platform_GetContext();
    
    windowState.Width              = DEFAULT_WINDOW_WIDTH;
    windowState.Height             = DEFAULT_WINDOW_HEIGHT;
    windowState.Caption            = (i_char*)DEFAULT_CAPTION;
    windowState.Running            = True;
    windowState.NativeWindowHandle = NULL;

    if (!platform_ContextInit()) {
        return False;
    }

    ctx->hInstance = GetModuleHandle(NULL);
    
    /* Inicializa backend de renderização */
    if (!platform_RenderInit()) {
        LOG_ERROR("Failed to initialize render backend");
        return False;
    }
    
    /* Registra classe da window */
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = ctx->hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "PlatformWindowClass";
    
    if (!RegisterClassEx(&wc)) {
        LOG_ERROR("Failed to register window class");
        return False;
    }
    
    /* Cria window */
    DWORD style = WS_OVERLAPPEDWINDOW;
    
    RECT rect = {0, 0, windowState.Width, windowState.Height};
    AdjustWindowRect(&rect, style, FALSE);
    
    ctx->hwnd = CreateWindowEx(
        0,
        "PlatformWindowClass",
        (char*)windowState.Caption,
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        ctx->hInstance,
        NULL
    );
    
    if (!ctx->hwnd) {
        LOG_ERROR("Failed to create window");
        return False;
    }
    
    windowState.NativeWindowHandle = (ptr)ctx->hwnd;
    
    ShowWindow(ctx->hwnd, SW_SHOW);
    UpdateWindow(ctx->hwnd);
    
    /* Cria contexto de renderização */
    if (!platform_RenderCreateContext()) {
        LOG_ERROR("Failed to create render context");
        DestroyWindow(ctx->hwnd);
        return False;
    }
    
    LOG_INFO("Window initialized: %dx%d", windowState.Width, windowState.Height);
    
    return True;
}

void __namespace( Free ) ( void )
{
    windowState.Running = False;
    platform_ContextFree();
    LOG_INFO("Window_Platform (Free)");
}

void __namespace( Poll ) ( void )
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
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

PlatformWindowState __namespace( GetState ) ( void )
{
    return windowState;
}

void __namespace( Shutdown ) ( void )
{
    PlatformContext* ctx = platform_GetContext();
    
    platform_RenderCleanUp();
    
    if (ctx->hwnd) {
        DestroyWindow(ctx->hwnd);
    }
    
    LOG_INFO("Platform Window (ShutDown)");
}

u8 __namespace( IsRunning ) ( void )
{
    return windowState.Running;
}

void __namespace( SwapBuffers ) ( void )
{
    platform_RenderSwapBuffers();
}

void __namespace( SetWindowSize ) ( const i32 width, const i32 height )
{
    PlatformContext* ctx = platform_GetContext();
    
    windowState.Width = width;
    windowState.Height = height;
    
    if (ctx->hwnd) {
        RECT rect = {0, 0, width, height};
        AdjustWindowRect(&rect, GetWindowLong(ctx->hwnd, GWL_STYLE), FALSE);
        
        SetWindowPos(ctx->hwnd, NULL, 0, 0,
                    rect.right - rect.left,
                    rect.bottom - rect.top,
                    SWP_NOMOVE | SWP_NOZORDER);
    }
    
    LOG_INFO("Window size set to %dx%d", width, height);
}

void __namespace( GetNativeWindowHandle ) ( ptr* handle )
{
    if (handle) {
        *handle = windowState.NativeWindowHandle;
    }
}

#undef __namespace

#endif /* PIPE_WINDOWS */