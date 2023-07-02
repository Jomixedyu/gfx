#include "GFXVulkanRenderTarget.h"
#include "GFXVulkanApplication.h"
#include "PhysicalDeviceHelper.h"
#include "BufferHelper.h"
#include <array>
#include <cassert>

namespace gfx
{
    GFXVulkanRenderTarget::GFXVulkanRenderTarget(
        GFXVulkanApplication* app, 
        GFXVulkanTexture2D* tex, VkFormat format, GFXVulkanTexture2D* depth, VkFormat depthFormat,
        const std::shared_ptr<GFXVulkanRenderPass>& renderPass)
        : m_app(app), m_tex2d(tex), m_depthTex(depth), m_renderPass(renderPass)
    {
        auto usable = tex ? tex : depth;
        assert(usable);
        m_width = usable->GetWidth();
        m_height = usable->GetHeight();
        
        this->InitRenderPass();
    }

    void GFXVulkanRenderTarget::InitRenderPass()
    {
        //create framebuffer
        std::array<VkImageView, 2> fbAttachments =
        {
            m_tex2d->GetVkImageView(),
            m_depthTex->GetVkImageView()
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass->GetVkRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
        framebufferInfo.pAttachments = fbAttachments.data();
        framebufferInfo.width = m_width;
        framebufferInfo.height = m_height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_app->GetVkDevice(), &framebufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }

    }

    void GFXVulkanRenderTarget::TermRenderPass()
    {
        vkDestroyFramebuffer(m_app->GetVkDevice(), m_frameBuffer, nullptr);
        m_frameBuffer = VK_NULL_HANDLE;
    }

    GFXVulkanRenderTarget::~GFXVulkanRenderTarget()
    {
        TermRenderPass();
    }
}