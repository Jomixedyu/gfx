#include "GFXVulkanBuffer.h"
#include "GFXVulkanBuffer.h"
#include "GFXVulkanBuffer.h"
#include "GFXVulkanApplication.h"
#include <cassert>
#include <stdexcept>

namespace gfx
{
    static uint32_t _FindMemoryType(GFXVulkanApplication* app, uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(app->GetVkPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    static void _CreateBuffer(
        GFXVulkanApplication* app,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(app->GetVkDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(app->GetVkDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = _FindMemoryType(app, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(app->GetVkDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(app->GetVkDevice(), buffer, bufferMemory, 0);
    }
    static void _CopyBuffer(GFXVulkanApplication* app, VkBuffer src, VkBuffer dest, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer;
        {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = app->GetVkCommandPool();
            allocInfo.commandBufferCount = 1;
            vkAllocateCommandBuffers(app->GetVkDevice(), &allocInfo, &commandBuffer);
        }

        VkCommandBufferBeginInfo beginInfo{};
        {
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        {
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, src, dest, 1, &copyRegion);
        }
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        {
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;
        }

        vkQueueSubmit(app->GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(app->GetVkGraphicsQueue());

        vkFreeCommandBuffers(app->GetVkDevice(), app->GetVkCommandPool(), 1, &commandBuffer);
    }

    GFXVulkanBuffer::GFXVulkanBuffer(GFXVulkanApplication* app, GFXBufferUsage usage, size_t bufferSize)
        : m_app(app), base(usage, bufferSize)
    {
        if (IsGpuLocalMemory())
        {
            _CreateBuffer(m_app, m_bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | GetVkUsage(),
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_vkBuffer, m_vkBufferMemory
            );
        }
        else
        {
            _CreateBuffer(m_app, m_bufferSize,
                GetVkUsage(),
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_vkBuffer, m_vkBufferMemory);
        }

    }

    GFXVulkanBuffer::~GFXVulkanBuffer()
    {
        if (this->IsValid())
        {
            this->Release();
        }
    }



    void GFXVulkanBuffer::Fill(const void* data)
    {
        if (IsGpuLocalMemory())
        {
            //create staging buffer
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            _CreateBuffer(m_app, m_bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory
            );

            void* memData;
            vkMapMemory(m_app->GetVkDevice(), stagingBufferMemory, 0, m_bufferSize, 0, &memData);
            memcpy(memData, data, m_bufferSize);
            vkUnmapMemory(m_app->GetVkDevice(), stagingBufferMemory);

            //transfer
            _CopyBuffer(m_app, stagingBuffer, m_vkBuffer, m_bufferSize);
            vkDestroyBuffer(m_app->GetVkDevice(), stagingBuffer, nullptr);
            vkFreeMemory(m_app->GetVkDevice(), stagingBufferMemory, nullptr);
        }
        else
        {
            void* gpuData;
            vkMapMemory(m_app->GetVkDevice(), m_vkBufferMemory, 0, m_bufferSize, 0, &gpuData);
            memcpy(gpuData, data, m_bufferSize);
            vkUnmapMemory(m_app->GetVkDevice(), m_vkBufferMemory);
        }

    }

    void GFXVulkanBuffer::Release()
    {
        if (m_hasData)
        {
            vkDestroyBuffer(m_app->GetVkDevice(), m_vkBuffer, nullptr);
            vkFreeMemory(m_app->GetVkDevice(), m_vkBufferMemory, nullptr);
            m_hasData = false;
        }
    }

    VkBufferUsageFlags GFXVulkanBuffer::GetVkUsage() const
    {
        VkBufferUsageFlags vkUsage;
        switch (m_usage)
        {
        case gfx::GFXBufferUsage::Vertex:
            vkUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case gfx::GFXBufferUsage::Index:
            vkUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case GFXBufferUsage::ConstantBuffer:
            vkUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        default:
            assert(false);
            break;
        }
        return vkUsage;
    }

    bool GFXVulkanBuffer::IsGpuLocalMemory() const
    {
        return m_usage != GFXBufferUsage::ConstantBuffer;
    }

    bool GFXVulkanBuffer::IsValid() const
    {
        return m_hasData;
    }
}