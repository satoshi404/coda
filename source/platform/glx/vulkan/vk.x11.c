#include <platform/glx/factory.h>
#include <platform/window/factory.h>

//#define GLX_VULKAN

#if PIPE_LINUX && defined(GLX_VULKAN)

#include <core/debug.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define __namespace( func_name ) platform##_##Glx##func_name

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    VkInstance       instance;
    VkSurfaceKHR     surface;
    VkPhysicalDevice physicalDevice;
    VkDevice         device;
    VkQueue          graphicsQueue;
    VkSwapchainKHR   swapchain;
} RenderGlx;


extern PlatformWindowRendererContext ctx;

RenderGlx renderGlx = {0};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 __namespace( Init ) ( void ) 
{
    
    if (!ctx.display) {
        LOG_ERROR("Display not initialized");
        return False;
    }
    
    /* Para Vulkan, usa visual padrão */
    XVisualInfo vinfo_template = {0};
    int nitems = 0;
    
    vinfo_template.screen = ctx.screen;
    ctx.visual = XGetVisualInfo(ctx.display, VisualScreenMask, &vinfo_template, &nitems);
    
    if (!ctx.visual && nitems > 0) {
        LOG_ERROR("Failed to get default visual");
        return False;
    }
    
    /* Cria instância Vulkan */
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Platform App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Custom Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME
    };
    
    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = extensions;
    
    if (vkCreateInstance(&createInfo, NULL, &renderGlx.instance) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan instance");
        return False;
    }
    
    LOG_INFO("Vulkan instance created (Linux)");
    return True;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 __namespace( CreateContext ) ( void )
{
 
    if (!ctx.window) {
        LOG_ERROR("Window must be created before Vulkan surface");
        return False;
    }
    
    /* Cria surface Xlib */
    VkXlibSurfaceCreateInfoKHR surfaceInfo = {0};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.dpy = ctx.display;
    surfaceInfo.window = ctx.window;
    
    if (vkCreateXlibSurfaceKHR(renderGlx.instance, &surfaceInfo,
        NULL, &renderGlx.surface) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan Xlib surface");
        return False;
    }
    
    // TODO: ..

    LOG_INFO("Vulkan surface created (Linux)");
    return True;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: BUG AO FINALIZAR ENGINE
void __namespace( CleanUp ) ( void )
{
    
    if ( renderGlx.swapchain )
    {
        vkDestroySwapchainKHR( renderGlx.device,
            renderGlx.swapchain, NULL );
    }
    
    if ( renderGlx.device )
    {
        vkDestroyDevice( renderGlx.device, NULL );
    }
    
    if ( renderGlx.surface )
    {
        vkDestroySurfaceKHR( renderGlx.instance,
            renderGlx.surface, NULL );
    }
    
    if (renderGlx.instance) {
        vkDestroyInstance( renderGlx.instance, NULL );
    }
    
    if ( ctx.visual )
    {
        XFree( ctx.visual );
        ctx.visual = NULL;
    }
    
    LOG_INFO( "Vulkan ( Linux ) cleaned up" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace( SwapBuffers ) ( void )
{
    /* vkQueuePresentKHR seria chamado aqui */
    /* Requer setup completo do swapchain */
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace( Resize ) ( i32 width, i32 height )
{
    /* Recriar swapchain com novo tamanho */
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef __namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* PLATFORM_LINUX && RENDER_VULKAN */
