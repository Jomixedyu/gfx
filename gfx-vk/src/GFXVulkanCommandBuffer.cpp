#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanCommandBuffer.h"
#include <gfx-vk/GFXVulkanCommandBuffer.h>
#include <gfx-vk/GFXVulkanApplication.h>

namespace gfx
{
    GFXVulkanCommandBuffer::GFXVulkanCommandBuffer(GFXVulkanApplication* app)
        : m_app(app)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_app->GetVkCommandPool();
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(m_app->GetVkDevice(), &allocInfo, &m_cmdBuffer);
    }
    GFXVulkanCommandBuffer::~GFXVulkanCommandBuffer()
    {
        vkFreeCommandBuffers(m_app->GetVkDevice(), m_app->GetVkCommandPool(), 1, &m_cmdBuffer);
    }
    GFXApplication* GFXVulkanCommandBuffer::GetApplication() const
    {
        return m_app;
    }
    void GFXVulkanCommandBuffer::Begin()
    {
        vkResetCommandBuffer(m_cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(m_cmdBuffer, &beginInfo);
    }
    void GFXVulkanCommandBuffer::End()
    {
        vkEndCommandBuffer(m_cmdBuffer);


        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_cmdBuffer;

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        if (m_imageSemaphore)
        {
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &m_imageSemaphore;
            submitInfo.pWaitDstStageMask = waitStages;
        }
        if (m_renderSemaphore)
        {
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &m_renderSemaphore;
        }

        vkQueueSubmit(m_app->GetVkGraphicsQueue(), 1, &submitInfo, m_inFlightFence);

        vkQueueWaitIdle(m_app->GetVkGraphicsQueue());

    }

    void GFXVulkanCommandBuffer::CmdClear(float r, float g, float b, float a, bool depth, bool stencil)
    {
        VkClearValue colorValue;
        colorValue.color = { {r, g, b, a} };
        VkClearValue depthValue;

    }
    void GFXVulkanCommandBuffer::CmdBindPipeline(GFXShaderPass* pipeline) {}
    void GFXVulkanCommandBuffer::CmdBindVertexBuffers() {}
    void GFXVulkanCommandBuffer::CmdBindIndexBuffer() {}
    void GFXVulkanCommandBuffer::CmdBindDescriptorSets() {}
    void GFXVulkanCommandBuffer::CmdDrawIndexed() {}


}