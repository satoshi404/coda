#include <platform/window/factory.h>

#if  PIPE_WINDOWS && defined(GLX_VULKAN)

#define __namespace( func_name ) platform##_##Glx##func_name

#include <core/debug.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    VkInstance       instance;
    VkSurfaceKHR     surface;
    VkPhysicalDevice physicalDevice;
    VkDevice         device;
    VkQueue          graphicsQueue;
    VkSwapchainKHR   swapchain;
} RenderContext;

extern PlatformWindowRendererContext ctx;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 __namespace( Init ) ( void )
{

    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Platform App";
    appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.pEngineName = "Custom Engine";
    appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    const char* extensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };
    
    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = extensions;
    
    if (vkCreateInstance(&createInfo, NULL, &ctx->renderContext.instance) != VK_SUCCESS) 
    {
        LOG_ERROR("Failed to create Vulkan instance");
        return False;
    }
    
    LOG_INFO("Vulkan instance created (Windows)");
    return True;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 __namespace( CreateContext ) ( void )
{
    if (!ctx->hwnd)
    {
        LOG_ERROR("Window must be created before Vulkan surface");
        return False;
    }
    
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {0};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hinstance = ctx->hInstance;
    surfaceInfo.hwnd = ctx->hwnd;
    
    if ( vkCreateWin32SurfaceKHR( ctx->renderContext.instance, &surfaceInfo,
                                NULL, &ctx->renderContext.surface ) != VK_SUCCESS ) {
        LOG_ERROR("Failed to create Vulkan Win32 surface");
        return False;
    }
    
    // TODO: ..

    LOG_INFO( "Vulkan surface created (Windows)" );
    return True;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace( CleanUp ) ( void )
{
    if ( ctx->renderContext.swapchain )
    {
        vkDestroySwapchainKHR( ctx->renderContext.device,
            ctx->renderContext.swapchain, NULL );
    }
    
    if ( ctx->renderContext.device )
    {
        vkDestroyDevice( ctx->renderContext.device, NULL );
    }
    
    if ( ctx->renderContext.surface )
    {
        vkDestroySurfaceKHR( ctx->renderContext.instance,
            ctx->renderContext.surface, NULL );
    }
    
    if ( ctx->renderContext.instance )
    {
        vkDestroyInstance( ctx->renderContext.instance, NULL );
    }
    
    LOG_INFO( "Vulkan (Windows) cleaned up" );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace( SwapBuffers ) ( void )
{
    /* vkQueuePresentKHR seria chamado aqui */
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace( Resize ) ( i32 width, i32 height )
{
    /* Recriar swapchain com novo tamanho */
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef __namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* PLATFORM_WINDOWS && RENDER_VULKAN */