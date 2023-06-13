#pragma once
#include "VulkanInclude.h"

namespace gfx
{
    class GFXVulkanApplication;

    struct VulkanShaderModule
    {
    public:
        VulkanShaderModule(GFXVulkanApplication* app, uint8_t* vertCode, size_t vertCodeLen, uint8_t* fragCode, size_t fragCodeLen);
        VulkanShaderModule(const VulkanShaderModule&) = delete;
        VulkanShaderModule(VulkanShaderModule&&) = delete;
        ~VulkanShaderModule();

        VkPipelineShaderStageCreateInfo ShaderStages[2]{};

        VkShaderModule VertShaderModule;
        VkShaderModule FragShaderModule;
    protected:
        GFXVulkanApplication* m_app;
    };
}