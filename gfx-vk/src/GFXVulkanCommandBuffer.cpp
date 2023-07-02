#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanCommandBufferPool.h"
#include "GFXVulkanGraphicsPipeline.h"

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
    }

    void GFXVulkanCommandBuffer::CmdBindShaderPass(GFXShaderPass* pipeline)
    {
        auto vkpipeline = static_cast<GFXVulkanGraphicsPipeline*>(pipeline);
        vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkpipeline->GetVkPipeline());
    }


}