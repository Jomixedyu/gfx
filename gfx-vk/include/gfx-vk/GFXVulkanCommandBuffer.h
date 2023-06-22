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
        virtual ~GFXVulkanCommandBuffer() override;

        virtual void Begin() override;
        virtual void End() override;


        virtual void CmdClear(float r, float g, float b, float a, bool depth, bool stencil) override;
        virtual void CmdBindPipeline(GFXShaderPass* pipeline) override;
        virtual void CmdBindVertexBuffers() override;
        virtual void CmdBindIndexBuffer() override;
        virtual void CmdBindDescriptorSets() override;
        virtual void CmdDrawIndexed() override;
        void CmdClearColor(float r, float g, float b, float a)
        {
            VkClearColorValue color{ r,g,b,a };
            VkImageSubresourceRange srRange{};
            srRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            srRange.baseMipLevel = 0;
            srRange.levelCount = VK_REMAINING_MIP_LEVELS;
            srRange.baseArrayLayer = 0;
            srRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
            vkCmdClearColorImage(m_cmdBuffer, m_rt->GetVkImage(), m_rt->GetVkImageLayout(), &color, 1, &srRange);
        }
        void BeginRenderTarget(GFXVulkanRenderTarget* renderTarget)
        {
            m_rt = renderTarget;
        }
        void EndRenderTarget()
        {
            m_rt = nullptr;
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
        }

    public:
        virtual GFXApplication* GetApplication() const override;
        VkCommandBuffer GetVkCommandBuffer() const { return m_cmdBuffer; }
    protected:
        VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;
        GFXVulkanApplication* m_app;
        GFXVulkanRenderTarget* m_rt = nullptr;
    public:
        VkSemaphore m_imageSemaphore = VK_NULL_HANDLE;
        VkSemaphore m_renderSemaphore = VK_NULL_HANDLE;
        VkFence m_inFlightFence = VK_NULL_HANDLE;

    };
}