#pragma once
#include <gfx/GFXGraphicsPipeline.h>
#include <gfx/GFXDescriptorSet.h>
#include <gfx-vk/VulkanInclude.h>
#include "GFXVulkanRenderPass.h"

namespace gfx
{
    class GFXVulkanApplication;
    class GFXVulkanGraphicsPipeline : public GFXGraphicsPipeline
    {
    public:
        GFXVulkanGraphicsPipeline(
            GFXVulkanApplication* app, 
            const GFXGraphicsPipelineConfig& config,
            std::shared_ptr<GFXVertexLayoutDescription> vertexLayout,
            std::shared_ptr<GFXShaderModule> shaderModule,
            const std::shared_ptr<GFXDescriptorSetLayout>& descSetLayout);
        virtual ~GFXVulkanGraphicsPipeline() override;

    public:
        const VkPipelineLayout& GetVkPipelineLayout() const { return m_pipelineLayout; }
        const VkPipeline& GetVkPipeline() const { return m_graphicsPipeline; }
    protected:
        GFXVulkanApplication* m_app;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    };
}