#include <platform/window/factory.h>

#if PIPE_WINDOWS && defined(GLX_OPENGL)

#include <core/debug.h>
#include <glad/glad.h>

typedef struct
{
    HDC   hdc;
    HGLRC hglrc;
} RenderCtx;

extern PlatformWindowRendererContext ctx;

RenderCtx ctx = {0};

u8 platform_RenderInit(void)
{
    return True;
}

u8 platform_RenderCreateContext(void)
{
    if (!ctx->hwnd) {
        LOG_ERROR("Window must be created before GL context");
        return False;
    }
    
    ctx->hdc = GetDC(ctx->hwnd);
    if (!ctx->hdc) {
        LOG_ERROR("Failed to get device context");
        return False;
    }

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,  // Color bits
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,  // Depth bits
        8,   // Stencil bits
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    
    int pixelFormat = ChoosePixelFormat(ctx->hdc, &pfd);
    if (!pixelFormat) {
        LOG_ERROR("Failed to choose pixel format");
        ReleaseDC(ctx->hwnd, ctx->hdc);
        return False;
    }
    
    if (!SetPixelFormat(ctx->hdc, pixelFormat, &pfd)) {
        LOG_ERROR("Failed to set pixel format");
        ReleaseDC(ctx->hwnd, ctx->hdc);
        return False;
    }
    
    /* Cria contexto OpenGL */
    ctx->renderContext.hglrc = wglCreateContext(ctx->hdc);
    if (!ctx->renderContext.hglrc) {
        LOG_ERROR("Failed to create WGL context");
        ReleaseDC(ctx->hwnd, ctx->hdc);
        return False;
    }
    
    ctx->renderContext.hdc = ctx->hdc;
    
    /* Ativa o contexto */
    if (!wglMakeCurrent(ctx->renderContext.hdc, ctx->renderContext.hglrc)) {
        LOG_ERROR("Failed to make WGL context current");
        wglDeleteContext(ctx->renderContext.hglrc);
        ReleaseDC(ctx->hwnd, ctx->hdc);
        return False;
    }
    
    /* Log info OpenGL */
    LOG_INFO("OpenGL Vendor: %s", glGetString(GL_VENDOR));
    LOG_INFO("OpenGL Renderer: %s", glGetString(GL_RENDERER));
    LOG_INFO("OpenGL Version: %s", glGetString(GL_VERSION));
    
    return True;
}

void platform_RenderCleanUp(void)
{
    
    if (ctx->renderContext.hglrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(ctx->renderContext.hglrc);
        ctx->renderContext.hglrc = NULL;
    }
    
    if (ctx->hdc) {
        ReleaseDC(ctx->hwnd, ctx->hdc);
        ctx->hdc = NULL;
    }
    
    LOG_INFO("OpenGL (Windows) cleaned up");
}

void platform_RenderSwapBuffers(void)
{
    
    if (ctx->renderContext.hdc) {
        SwapBuffers(ctx->renderContext.hdc);
    }
}

void platform_RenderResize(i32 width, i32 height)
{
    glViewport(0, 0, width, height);
}

#endif /* PLATFORM_WINDOWS && RENDER_OPENGL */