#include <platform/window/factory.h>

#if PIPE_LINUX && defined(GLX_OPENGL)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define __namespace( func_name ) platform##_##Glx##func_name

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <core/debug.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <glad/glad.h>
#include <GL/glx.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int glx_attribs[] = {
    GLX_RGBA,
    GLX_DOUBLEBUFFER,
    GLX_DEPTH_SIZE,   24,
    GLX_STENCIL_SIZE, 8,
    GLX_RED_SIZE,     8,
    GLX_GREEN_SIZE,   8,
    GLX_BLUE_SIZE,    8,
    GLX_ALPHA_SIZE,   8,
    None
};

extern PlatformWindowRendererContext ctx;

GLXContext renderCtx;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 __namespace( Init ) ( void )
{ 
    if ( !ctx.display ) 
    {
        LOG_ERROR("Display not initialized before render init");
        return False;
    }
    
    ctx.visual = glXChooseVisual(ctx.display, ctx.screen, glx_attribs);
    
    if ( !ctx.visual ) 
    {
        LOG_ERROR("Failed to choose GLX visual");
        return False;
    }
    
    LOG_INFO( "GLX Visual selected: depth=%d", ctx.visual->depth );
    
    return True;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 __namespace( CreateContext) ( void )
{
    if ( !ctx.window )
    {
        LOG_ERROR( "Window must be created before GL context" );
        return False;
    }
    
    renderCtx = glXCreateContext( ctx.display, ctx.visual, NULL, GL_TRUE );
    
    if ( !renderCtx )
    {
        LOG_ERROR( "Failed to create GLX context" );
        return False;
    }
    
    if ( !glXMakeCurrent(ctx.display, ctx.window, renderCtx ) )
    {
        LOG_ERROR( "Failed to make GLX context current" );
        glXDestroyContext( ctx.display, renderCtx );
        renderCtx = NULL;
        return False;
    }

     if ( !gladLoadGL() )
     {
        fprintf( stderr, "Failed to load OpenGL\n" );
        return 1;
    }
    
    LOG_INFO( "OpenGL Vendor: %s", glGetString( GL_VENDOR ) );
    LOG_INFO( "OpenGL Renderer: %s", glGetString( GL_RENDERER ) );
    LOG_INFO( "OpenGL Version: %s", glGetString( GL_VERSION ) );
    
    return True;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace( CleanUp ) ( void )
{
    if ( renderCtx )
    {
        glXMakeCurrent( ctx.display, None, NULL );
        glXDestroyContext( ctx.display, renderCtx );
        renderCtx = NULL;
    }
    
    if ( ctx.visual )
    {
        XFree( ctx.visual );
        ctx.visual = NULL;
    }
    
    if ( ctx.colormap )
    {
        XFreeColormap( ctx.display, ctx.colormap );
        ctx.colormap = 0;
    }
    
    LOG_INFO( "OpenGL (Linux) cleaned up" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace( SwapBuffers ) ( void )
{
    if ( ctx.display && ctx.window )
    {
        glXSwapBuffers( ctx.display, ctx.window );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace( Resize ) ( i32 width, i32 height )
{
    glViewport( 0, 0, width, height );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef __namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* PLATFORM_LINUX && RENDER_OPENGL */