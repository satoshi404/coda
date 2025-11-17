#ifndef __vulkan_helpers_h__
#define __vulkan_helpers_h__

#ifdef GLX_VULKAN

#include <vulkan/vulkan.h>
#include <core/types.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkDevice         vk_get_device(void);
VkPhysicalDevice vk_get_physical_device(void);
VkCommandPool    vk_get_command_pool(void);
VkQueue          vk_get_graphics_queue(void);
VkSwapchainKHR   vk_get_swapchain(void);
VkRenderPass     vk_get_render_pass(void);
VkExtent2D       vk_get_swapchain_extent(void);
VkFormat         vk_get_swapchain_format(void);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 vk_create_buffer(VkDeviceSize size, 
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkBuffer* buffer, 
                    VkDeviceMemory* bufferMemory);
                    
void vk_copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

u32 vk_find_memory_type(u32 typeFilter, VkMemoryPropertyFlags properties);

/* Command buffer helpers */
VkCommandBuffer vk_begin_single_time_commands(void);
void vk_end_single_time_commands(VkCommandBuffer commandBuffer);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* GLX_VULKAN */

#endif