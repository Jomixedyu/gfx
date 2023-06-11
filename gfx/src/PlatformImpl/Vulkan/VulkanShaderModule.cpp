#include "VulkanShaderModule.h"
#include "GFXVulkanApplication.h"
#include <stdexcept>

namespace gfx
{
    VkShaderModule _CreateShaderModule(GFXVulkanApplication* app, uint8_t* code, size_t len)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = len;
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(app->GetVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    VulkanShaderModule::VulkanShaderModule(GFXVulkanApplication* app, uint8_t* vertCode, size_t vertCodeLen, uint8_t* fragCode, size_t fragCodeLen)
        : m_app(app)
    {
        VertShaderModule = _CreateShaderModule(app, vertCode, vertCodeLen);
        FragShaderModule = _CreateShaderModule(app, fragCode, fragCodeLen);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = VertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = FragShaderModule;
        fragShaderStageInfo.pName = "main";

        ShaderStages[0] = vertShaderStageInfo;
        ShaderStages[1] = fragShaderStageInfo;
    }
    VulkanShaderModule::~VulkanShaderModule()
    {
        vkDestroyShaderModule(m_app->GetVkDevice(), FragShaderModule, nullptr);
        vkDestroyShaderModule(m_app->GetVkDevice(), VertShaderModule, nullptr);
    }
}