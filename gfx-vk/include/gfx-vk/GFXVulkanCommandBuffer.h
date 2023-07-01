#pragma once
#include <gfx/GFXCommandBuffer.h>
#include "VulkanInclude.h"
#include "GFXVulkanRenderTarget.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanCommandBuffer : public GFXCommandBuffer
    {

    public:
        GFXVulkanCommandBuffer(GFXVulkanApplication* app);
        GFXVulkanCommandBuffer(GFXVulkanCommandBuffer&& r) noexcept;

        virtual ~GFXVulkanCommandBuffer() override;

        virtual void Begin() override;
        virtual void End() override;


        virtual void CmdBindPipeline(GFXShaderPass* pipeline) override;
        virtual void CmdBindVertexBuffers() override;
        virtual void CmdBindIndexBuffer() override;
        virtual void CmdBindDescriptorSets() override;
        virtual void CmdDrawIndexed() override;
        void SetRenderTarget(GFXVulkanRenderTarget* renderTarget)
        {
            m_rt = renderTarget;
        }
        void CmdClearColor(float r, float g, float b, float a)
        {
            VkClearColorValue color{ r,g,b,a };
            VkImageSubresourceRange srRange{};
            srRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            srRange.baseMipLevel = 0;
            srRange.levelCount = 1;
            srRange.baseArrayLayer = 0;
            srRange.layerCount = 1;

            VkImageLayout clearLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            {
                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = clearLayout;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = m_rt->GetVkColorImage();
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                vkCmdPipelineBarrier(
                    m_cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0, 0, nullptr, 0, nullptr, 1, &barrier);
            }
            auto image = m_rt->GetVkColorImage();

            vkCmdClearColorImage(m_cmdBuffer, image, clearLayout, &color, 1, &srRange);

            {
                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = clearLayout;
                barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = m_rt->GetVkColorImage();
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                vkCmdPipelineBarrier(
                    m_cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0, 0, nullptr, 0, nullptr, 1, &barrier);
            }

        }
        void CmdBeginRenderTarget()
        {
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_rt->GetVkRenderPass();
            renderPassInfo.framebuffer = m_rt->GetVkFrameBuffer();
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = m_rt->GetVkExtent();

            //std::array<VkClearValue, 2> clearValues{};
            //clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            //clearValues[1].depthStencil = { 1.0f, 0 };

            renderPassInfo.clearValueCount = 0;
            //renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            //renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(m_cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        }
        void CmdEndRenderTarget()
        {
            vkCmdEndRenderPass(m_cmdBuffer);
        }
        void CmdSetViewport(float x, float y, float width, float height)
        {
            VkViewport viewport{};
#if 1
            viewport.x = x;
            viewport.y = y + height;
            viewport.width = width;
            viewport.height = -height;
#else
            viewport.x = x;
            viewport.y = y;
            viewport.width = width;
            viewport.height = height;
#endif
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            vkCmdSetViewport(m_cmdBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = { (uint32_t)width, (uint32_t)height };
            vkCmdSetScissor(m_cmdBuffer, 0, 1, &scissor);

        }

        void CmdBindShaderPass(GFXShaderPass* shaderPass)
        {

        }
    public:
        virtual GFXApplication* GetApplication() const override;
        const VkCommandBuffer& GetVkCommandBuffer() const { return m_cmdBuffer; }
    protected:
        VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;
        GFXVulkanApplication* m_app;
        GFXVulkanRenderTarget* m_rt = nullptr;
    public:

    };
}