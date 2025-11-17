#ifdef GLX_VULKAN

#include <platform/glx/vulkan/helpers.h>
#include <core/debug.h>
#include <string.h>


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern struct 
{
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    VkRenderPass renderPass;
    VkCommandPool commandPool;
} g_vkContext;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkDevice vk_get_device( void ) 
{
    return g_vkContext.device;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkPhysicalDevice vk_get_physical_device( void ) 
{
    return g_vkContext.physicalDevice;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkCommandPool vk_get_command_pool( void )
{
    return g_vkContext.commandPool;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkQueue vk_get_graphics_queue( void )
{
    return g_vkContext.graphicsQueue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkSwapchainKHR vk_get_swapchain( void )
{
    return g_vkContext.swapchain;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkRenderPass vk_get_render_pass( void )
{
    return g_vkContext.renderPass;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkExtent2D vk_get_swapchain_extent( void )
{
    return g_vkContext.swapchainExtent;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkFormat vk_get_swapchain_format( void )
{
    return g_vkContext.swapchainImageFormat;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u32 vk_find_memory_type( u32 typeFilter, VkMemoryPropertyFlags properties )
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(g_vkContext.physicalDevice, &memProperties);
    
    for (u32 i = 0; i < memProperties.memoryTypeCount; i++) 
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
            { return i; }
    }
    
    LOG_ERROR( "Failed to find suitable memory type!" );
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 vk_create_buffer(VkDeviceSize size, 
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkBuffer* buffer, 
                    VkDeviceMemory* bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(g_vkContext.device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan buffer!");
        return 0;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_vkContext.device, *buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vk_find_memory_type(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(g_vkContext.device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate Vulkan buffer memory!");
        vkDestroyBuffer(g_vkContext.device, *buffer, NULL);
        return 0;
    }
    
    vkBindBufferMemory(g_vkContext.device, *buffer, *bufferMemory, 0);
    
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer vk_begin_single_time_commands(void)
{
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = g_vkContext.commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(g_vkContext.device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    return commandBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void vk_end_single_time_commands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(g_vkContext.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_vkContext.graphicsQueue);
    
    vkFreeCommandBuffers(g_vkContext.device, g_vkContext.commandPool, 1, &commandBuffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void vk_copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = vk_begin_single_time_commands();
    
    VkBufferCopy copyRegion = {0};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    vk_end_single_time_commands(commandBuffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* GLX_VULKAN */
