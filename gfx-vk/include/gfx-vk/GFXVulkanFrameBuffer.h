#pragma once
#include <gfx/GFXFrameBuffer.h>
#include "VulkanInclude.h"
#include "GFXVulkanRenderTarget.h"
#include <vector>

namespace gfx
{
    class GFXVulkanFrameBuffer : public GFXFrameBuffer
    {
    public:
        GFXVulkanFrameBuffer(const std::vector<GFXVulkanRenderTarget*>& renderTargets)
        {
            //std::vector<VkImageView> attachments;
            //for (auto& rt : renderTargets)
            //{
            //    attachments.push_back(rt->GetVkImageView());
            //}

            //VkFramebufferCreateInfo framebufferInfo{};
            //framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            //framebufferInfo.renderPass = m_renderPass;
            //framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            //framebufferInfo.pAttachments = attachments.data();
            //framebufferInfo.width = m_swapChainExtent.width;
            //framebufferInfo.height = m_swapChainExtent.height;
            //framebufferInfo.layers = 1;

            //if (vkCreateFramebuffer(m_app->GetVkDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS)
            //{
            //    throw std::runtime_error("failed to create framebuffer!");
            //}
        }

        GFXVulkanFrameBuffer(const GFXVulkanFrameBuffer&) = delete;
        virtual ~GFXVulkanFrameBuffer() override {}

    protected:
        VkFramebuffer m_frameBuffer;
    };
}