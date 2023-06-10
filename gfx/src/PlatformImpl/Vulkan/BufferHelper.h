#pragma once
#include "GFXVulkanApplication.h"

namespace gfx
{
    class BufferHelper
    {
    public:
        static uint32_t FindMemoryType(GFXVulkanApplication* app, uint32_t typeFilter, VkMemoryPropertyFlags properties);

        static void CreateBuffer(
            GFXVulkanApplication* app,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);

        static void TransferBuffer(GFXVulkanApplication* app, VkBuffer src, VkBuffer dest, VkDeviceSize size);

        static void DestroyBuffer(GFXVulkanApplication* app, VkBuffer buffer, VkDeviceMemory mem);
    };
}