#include <platform/window/factory.h>
#include <platform/glx/factory.h>

#include <core/event.h>
#include <core/debug.h>
#include <core/types.h>
#include <core/math.h>
#include <render/shader.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#ifdef GLX_OPENGL
    #include <glad/glad.h>
#elif defined(GLX_VULKAN)
    #include <vulkan/vulkan.h>
    #include <platform/glx/vulkan/helpers.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct
{
    u8 EventsInitialized;
    u8 WindowInitialized;
    u8 CallbacksRegistered;
    u8 RenderInitialized;
} ClearUpState = {0};

static struct
{
    Shader*        shader;
    ShaderLibrary* shaderLib;
    
    #ifdef GLX_OPENGL
    u32 vao;
    u32 vbo;
    u32 ebo;
    #elif defined(GLX_VULKAN)
    /* Vulkan resources */
    VkBuffer       vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer       indexBuffer;
    VkDeviceMemory indexBufferMemory;
    
    VkCommandBuffer commandBuffer;
    VkSemaphore     imageAvailableSemaphore;
    VkSemaphore     renderFinishedSemaphore;
    VkFence         inFlightFence;
    #endif

    f32 dt;
    
} RenderState = {0};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec3 color;
} Vertex;

/* Dados do Cubo usando estrutura Vertex */
static Vertex cubeVertices[] = {
    /* Face Frontal (Z+) - Vermelho */
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    
    /* Face Traseira (Z-) - Verde */
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    
    /* Face Direita (X+) - Azul */
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    
    /* Face Esquerda (X-) - Amarelo */
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
    
    /* Face Superior (Y+) - Magenta */
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
    
    /* Face Inferior (Y-) - Ciano */
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
    {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
};

static u32 cubeIndices[] = {
    0,  1,  2,   2,  3,  0,   /* Frontal */
    4,  5,  6,   6,  7,  4,   /* Traseira */
    8,  9,  10,  10, 11, 8,   /* Direita */
    12, 13, 14,  14, 15, 12,  /* Esquerda */
    16, 17, 18,  18, 19, 16,  /* Superior */
    20, 21, 22,  22, 23, 20   /* Inferior */
};

#define RESOLUTION_W 1920 
#define RESOLUTION_H 1044

static u8 is_resize = False;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <platform/window/setup.h>

void onKeyboardEvent(const void* event)
{
    const KeyboardEvent* kbEvent = (const KeyboardEvent*)event;
    if (kbEvent->state == KEY_STATE_DOWN) 
    {

        if ( kbEvent->keycode == 41 )
        {
            int w = DEFAULT_WINDOW_WIDTH;
            int h = DEFAULT_WINDOW_HEIGHT;

            if ( is_resize == False )
            {
                w = RESOLUTION_W;
                h = RESOLUTION_H;
                is_resize = True;
            }
            else
            {
                w = DEFAULT_WINDOW_WIDTH;
                h = DEFAULT_WINDOW_HEIGHT;
                is_resize = False;

            }

            platform_GlxResize( w, h );
            platform_WindowSetSize( w, h );

            platform_WindowSetPos( w/2, h/2 );
        }

        LOG_DEBUG("Key pressed: %d (timestamp: %u)",
            kbEvent->keycode, kbEvent->timestamp);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void onWindowEvent(const void* event)
{
    const WindowEvent* winEvent = (const WindowEvent*)event;
    LOG_DEBUG("Window resized: %dx%d", winEvent->width, winEvent->height);
    
    #ifdef GLX_OPENGL
    glViewport(0, 0, winEvent->width, winEvent->height);
    #elif defined(GLX_VULKAN)
    /* Vulkan precisa recriar swapchain no resize */
    LOG_INFO("Vulkan: Swapchain recreation needed");
    #endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GLX_VULKAN

/* Função externa do seu sistema Vulkan */
extern VkDevice vk_get_device(void);
extern VkPhysicalDevice vk_get_physical_device(void);
extern VkCommandPool vk_get_command_pool(void);
extern VkQueue vk_get_graphics_queue(void);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Find memory type */
static u32 vk_find_memory_type(u32 typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vk_get_physical_device(), &memProperties);
    
    for (u32 i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    LOG_ERROR("Failed to find suitable memory type!");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Create buffer */
static u8 vk_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
    VkDevice device = vk_get_device();
    
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan buffer!");
        return 0;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vk_find_memory_type(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate Vulkan buffer memory!");
        return 0;
    }
    
    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
    
    return 1;
}

#endif /* GLX_VULKAN */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GLX_OPENGL
static u8 coda_InitOpenGL(void)
{
    /* Habilita depth test */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    /* Face culling */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    /* Cria VAO */
    glGenVertexArrays(1, &RenderState.vao);
    glBindVertexArray(RenderState.vao);
    
    /* Cria VBO */
    glGenBuffers(1, &RenderState.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, RenderState.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    
    /* Cria EBO */
    glGenBuffers(1, &RenderState.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderState.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
    
    /* Atributo 0: Posição */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    
    /* Atributo 1: Normal */
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    
    /* Atributo 2: Cor */
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    LOG_INFO("OpenGL resources created (VAO: %u, VBO: %u, EBO: %u)",
        RenderState.vao, RenderState.vbo, RenderState.ebo);
    
    return True;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GLX_VULKAN
static u8 coda_InitVulkan(void)
{
    VkDevice device = vk_get_device();
    
    LOG_INFO("Initializing Vulkan resources...");
    
    /* 1. Cria Vertex Buffer */
    VkDeviceSize vertexBufferSize = sizeof(cubeVertices);
    
    /* Staging buffer (CPU -> GPU) */
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    if (!vk_create_buffer(vertexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory))
    {
        LOG_ERROR("Failed to create vertex staging buffer!");
        return False;
    }
    
    /* Copia dados para staging buffer */
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, vertexBufferSize, 0, &data);
    memcpy(data, cubeVertices, (size_t)vertexBufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    
    /* Cria vertex buffer final (device local - mais rápido) */
    if (!vk_create_buffer(vertexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &RenderState.vertexBuffer, &RenderState.vertexBufferMemory))
    {
        LOG_ERROR("Failed to create vertex buffer!");
        vkDestroyBuffer(device, stagingBuffer, NULL);
        vkFreeMemory(device, stagingBufferMemory, NULL);
        return False;
    }
    
    /* Copy staging -> device local (você precisa implementar copy_buffer) */
    /* vk_copy_buffer(stagingBuffer, RenderState.vertexBuffer, vertexBufferSize); */
    
    /* Cleanup staging */
    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
    
    LOG_INFO("Vertex buffer created");
    
    /* 2. Cria Index Buffer */
    VkDeviceSize indexBufferSize = sizeof(cubeIndices);
    
    if (!vk_create_buffer(indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory))
    {
        LOG_ERROR("Failed to create index staging buffer!");
        return False;
    }
    
    vkMapMemory(device, stagingBufferMemory, 0, indexBufferSize, 0, &data);
    memcpy(data, cubeIndices, (size_t)indexBufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    
    if (!vk_create_buffer(indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &RenderState.indexBuffer, &RenderState.indexBufferMemory))
    {
        LOG_ERROR("Failed to create index buffer!");
        vkDestroyBuffer(device, stagingBuffer, NULL);
        vkFreeMemory(device, stagingBufferMemory, NULL);
        return False;
    }
    
    /* Copy staging -> device local */
    /* vk_copy_buffer(stagingBuffer, RenderState.indexBuffer, indexBufferSize); */
    
    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
    
    LOG_INFO("Index buffer created");
    
    /* 3. Cria Semaphores e Fences */
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &RenderState.imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, NULL, &RenderState.renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, NULL, &RenderState.inFlightFence) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create synchronization objects!");
        return False;
    }
    
    LOG_INFO("Vulkan resources initialized successfully");
    
    return True;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define PI		3.14159265358979323846	/* pi */

static Mat4 model;

static void coda_RenderFrame(void)
{

    /* Cria matrizes */
    model = core_MathMat4Identity();
    Mat4 rotY = core_MathMat4Rotate(
        RenderState.dt * PI / 180.0f,
        core_MathVec3Create(0.0f, 1.0f, 0.0f)
    );
    Mat4 rotX = core_MathMat4Rotate(
        RenderState.dt * PI / 180.0f,
        core_MathVec3Create(1.0f, 0.0f, 0.0f)
    );
    model = core_MathMat4Multiply(rotY, rotX);
    
    Mat4 view = core_MathMat4LookAt(
        core_MathVec3Create(0.0f, 0.0f, 4.0f),
        core_MathVec3Create(0.0f, 0.0f, 0.0f),
        core_MathVec3Create(0.0f, 1.0f, 0.0f)
    );
    
    f32 aspect = (f32)platform_WindowGetWidth() / platform_WindowGetHeight();
    Mat4 proj = core_MathMat4Perspective(45.0f * PI / 180.0f, aspect, 0.1f, 100.0f);
    
    #ifdef GLX_OPENGL
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Mat4 vp = core_MathMat4Multiply(view, proj);
        Mat4 mvp = core_MathMat4Multiply(model, vp);
        
        /* Bind shader e define uniforms */
        renderer_ShaderBind(RenderState.shader);
        
        renderer_ShaderSetMat4(RenderState.shader, "uMVP", &mvp);
        renderer_ShaderSetMat4(RenderState.shader, "uModel", &model);
        
        glBindVertexArray(RenderState.vao);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        renderer_ShaderUbind();
    
    #elif defined(GLX_VULKAN)
        /* Aguarda frame anterior */
        vkWaitForFences(vk_get_device(), 1, &RenderState.inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(vk_get_device(), 1, &RenderState.inFlightFence);
        
        /* Atualiza uniforms (UBO) */
        renderer_ShaderSetMvp(RenderState.shader, &model, &view, &proj);
        
        /* Acquire swapchain image */
        u32 imageIndex;
        /* vkAcquireNextImageKHR(..., RenderState.imageAvailableSemaphore, ..., &imageIndex); */
        
        /* Record command buffer */
        /* vkBeginCommandBuffer(RenderState.commandBuffer, ...); */
        /* vkCmdBindPipeline(RenderState.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline); */
        /* vkCmdBindVertexBuffers(RenderState.commandBuffer, 0, 1, &RenderState.vertexBuffer, offsets); */
        /* vkCmdBindIndexBuffer(RenderState.commandBuffer, RenderState.indexBuffer, 0, VK_INDEX_TYPE_UINT32); */
        /* vkCmdDrawIndexed(RenderState.commandBuffer, 36, 1, 0, 0, 0); */
        /* vkEndCommandBuffer(RenderState.commandBuffer); */
        
        /* Submit */
        /* vkQueueSubmit(..., RenderState.imageAvailableSemaphore, ..., RenderState.renderFinishedSemaphore, RenderState.inFlightFence); */
        
        /* Present */
        /* vkQueuePresentKHR(..., RenderState.renderFinishedSemaphore, ...); */
        
        LOG_TRACE("Vulkan frame rendered");
    #endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void coda_load(void)
{
    /* Eventos */
    if (!core_EventInit()) {
        LOG_FATAL("Failed to initialize event system");
    }
    ClearUpState.EventsInitialized = True;
    
    core_EventRegisterCallback(EVENT_TYPE_KEYBOARD, onKeyboardEvent);
    core_EventRegisterCallback(EVENT_TYPE_WINDOW, onWindowEvent);
    ClearUpState.CallbacksRegistered = True;

    /* Window */
    if (!platform_WindowInit()) {
        LOG_FATAL("Failed to initialize window!");
    }
    ClearUpState.WindowInitialized = True;
    
    LOG_INFO("Name: %s", platform_WindowGetCaption());
    LOG_INFO("Window size: %dx%d", platform_WindowGetWidth(), platform_WindowGetHeight());
    
    #ifdef GLX_OPENGL
        LOG_INFO("Render Backend: OpenGL");
        if (!coda_InitOpenGL()) {
            LOG_FATAL("Failed to initialize OpenGL!");
        }
    #elif defined(GLX_VULKAN)
        LOG_INFO("Render Backend: Vulkan");
        if (!coda_InitVulkan()) {
            LOG_FATAL("Failed to initialize Vulkan!");
        }
    #endif
    
    ClearUpState.RenderInitialized = True;

    /* Shaders */
    RenderState.shaderLib = renderer_ShaderLibraryCreate();

    #ifdef GLX_OPENGL
        renderer_ShaderLibraryLoadDefaults(RenderState.shaderLib);
        RenderState.shader = renderer_ShaderLoadFromFile("cube", 
            "assets/shaders/cube/cube.vert", 
            "assets/shaders/cube/cube.frag");
        
        if (!RenderState.shader) {
            RenderState.shader = renderer_ShaderLibraryGet(RenderState.shaderLib, "basic");
        }
    #elif defined(GLX_VULKAN)
        RenderState.shader = renderer_ShaderLoadFromFile("cube",
            "assets/shaders/cube/cube.vert.spv",
            "assets/shaders/cube/cube.frag.spv");
        
        if (!RenderState.shader) {
            LOG_FATAL("Failed to load Vulkan shaders!");
        }
        renderer_ShaderLibraryAdd(RenderState.shaderLib, RenderState.shader);
    #endif
    
    LOG_INFO("Shader '%s' loaded", RenderState.shader->name);
    LOG_INFO("Resources loaded successfully");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline Bool coda_runtime(void)
{
 
    while (platform_WindowIsRunning())
    {
        
        RenderState.dt += 0.3;

        platform_WindowPoll();
        coda_RenderFrame();
        platform_WindowSwapBuffers();

        usleep(16666); /* ~60 FPS */
    }

    return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cleanUp(char* _cleanup_var)
{
    UNUSED(_cleanup_var);
    LOG_INFO("Starting cleanup...");
    
    if (ClearUpState.RenderInitialized)
    {
        #ifdef GLX_OPENGL
            if (RenderState.ebo) glDeleteBuffers(1, &RenderState.ebo);
            if (RenderState.vbo) glDeleteBuffers(1, &RenderState.vbo);
            if (RenderState.vao) glDeleteVertexArrays(1, &RenderState.vao);
        #elif defined(GLX_VULKAN)
            VkDevice device = vk_get_device();
            
            vkDeviceWaitIdle(device);
            
            if (RenderState.inFlightFence) 
                vkDestroyFence(device, RenderState.inFlightFence, NULL);
            if (RenderState.renderFinishedSemaphore) 
                vkDestroySemaphore(device, RenderState.renderFinishedSemaphore, NULL);
            if (RenderState.imageAvailableSemaphore) 
                vkDestroySemaphore(device, RenderState.imageAvailableSemaphore, NULL);
            
            if (RenderState.indexBuffer) 
                vkDestroyBuffer(device, RenderState.indexBuffer, NULL);
            if (RenderState.indexBufferMemory) 
                vkFreeMemory(device, RenderState.indexBufferMemory, NULL);
            
            if (RenderState.vertexBuffer) 
                vkDestroyBuffer(device, RenderState.vertexBuffer, NULL);
            if (RenderState.vertexBufferMemory) 
                vkFreeMemory(device, RenderState.vertexBufferMemory, NULL);
        #endif
        
        if (RenderState.shaderLib) {
            renderer_ShaderLibraryDestroy(RenderState.shaderLib);
        }
    }
    
    if (ClearUpState.CallbacksRegistered) {
        core_EventUnregisterCallback(EVENT_TYPE_KEYBOARD, onKeyboardEvent);
        core_EventUnregisterCallback(EVENT_TYPE_WINDOW, onWindowEvent);
    }
    
    if (ClearUpState.WindowInitialized) {
        platform_WindowShutdown();
        platform_WindowFree();
    }
    
    if (ClearUpState.EventsInitialized) {
        core_EventShutdown();
    }
    
    LOG_INFO("Cleanup complete");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    UNUSED(argc);
    UNUSED(argv);
    
    char _auto_cleanup_var __attribute__((cleanup(cleanUp)));
    UNUSED(_auto_cleanup_var);

    LOG_INFO("=== 3D Cube Demo Starting ===");

    coda_load();

    LOG_INFO("Entering main loop...");
    
    return coda_runtime();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////