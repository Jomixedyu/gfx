#pragma once
#include <gfx/GFXRenderPass.h>
#include "VulkanInclude.h"
#include "GFXVulkanCommandBuffer.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanRenderPass : public GFXRenderPassLayout
    {

    public:
        GFXVulkanRenderPass(GFXVulkanApplication* app);
        virtual ~GFXVulkanRenderPass() override;
    public:
        const VkRenderPass& GetVkRenderPass() const { return m_renderPass; }
    protected:
        GFXVulkanApplication* m_app;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;
    };
}