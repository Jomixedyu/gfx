#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanCommandBufferPool.h"

namespace gfx
{
    GFXVulkanCommandBuffer::GFXVulkanCommandBuffer(GFXVulkanApplication* app)
        : m_app(app)
    {
        m_cmdBuffer = app->GetCommandBufferPool()->GetCommandBuffer();
    }
    GFXVulkanCommandBuffer::~GFXVulkanCommandBuffer()
    {
        if (m_cmdBuffer != VK_NULL_HANDLE)
        {
            m_app->GetCommandBufferPool()->ReleaseCommandBuffer(m_cmdBuffer);
            m_cmdBuffer = VK_NULL_HANDLE;
        }
    }
    GFXVulkanCommandBuffer::GFXVulkanCommandBuffer(GFXVulkanCommandBuffer&& r) noexcept
    {
        m_app = r.m_app;
        m_cmdBuffer = r.m_cmdBuffer;
        m_rt = r.m_rt;

        r.m_cmdBuffer = VK_NULL_HANDLE;
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


        //VkSubmitInfo submitInfo{};
        //submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        //submitInfo.commandBufferCount = 1;
        //submitInfo.pCommandBuffers = &m_cmdBuffer;

        //VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        //if (m_imageSemaphore)
        //{
        //    submitInfo.waitSemaphoreCount = 1;
        //    submitInfo.pWaitSemaphores = &m_imageSemaphore;
        //    submitInfo.pWaitDstStageMask = waitStages;
        //}
        //if (m_renderSemaphore)
        //{
        //    submitInfo.signalSemaphoreCount = 1;
        //    submitInfo.pSignalSemaphores = &m_renderSemaphore;
        //}

        //vkQueueSubmit(m_app->GetVkGraphicsQueue(), 1, &submitInfo, m_inFlightFence);

        //vkQueueWaitIdle(m_app->GetVkGraphicsQueue());

    }

    void GFXVulkanCommandBuffer::CmdBindPipeline(GFXShaderPass* pipeline) {}
    void GFXVulkanCommandBuffer::CmdBindVertexBuffers() {}
    void GFXVulkanCommandBuffer::CmdBindIndexBuffer() {}
    void GFXVulkanCommandBuffer::CmdBindDescriptorSets() {}
    void GFXVulkanCommandBuffer::CmdDrawIndexed() {}


}