
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
//#include <vulkan/vulkan_win32.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <gfx/GFXThirdParty/glfw/include/GLFW/glfw3.h>
#include <gfx/GFXThirdParty/glfw/include/GLFW/glfw3native.h>

#undef max
#undef min

#include <gfx/GFXDefined.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gfx/GFXApplication.h>
#include <GFXVulkanApplication.h>
#include <GFXVulkanBuffer.h>
#include <chrono>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace gfx
{
    struct VertexData
    {
        static constexpr int MAX_COORD_NUM = 4;

        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec4 VertColor;
        glm::vec2 Coords[MAX_COORD_NUM];
    };

    static VkVertexInputBindingDescription GetBindingDescription() 
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(VertexData);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        int location = 0;
        {
            VkVertexInputAttributeDescription description;
            description.binding = 0;
            description.location = location++;
            description.format = VK_FORMAT_R32G32B32_SFLOAT;
            description.offset = offsetof(VertexData, Position);
            attributeDescriptions.push_back(description);
        }
        {
            VkVertexInputAttributeDescription description;
            description.binding = 0;
            description.location = location++;
            description.format = VK_FORMAT_R32G32B32_SFLOAT;
            description.offset = offsetof(VertexData, Normal);
            attributeDescriptions.push_back(description);
        }
        {
            VkVertexInputAttributeDescription description;
            description.binding = 0;
            description.location = location++;
            description.format = VK_FORMAT_R32G32B32_SFLOAT;
            description.offset = offsetof(VertexData, Tangent);
            attributeDescriptions.push_back(description);
        }
        {
            VkVertexInputAttributeDescription description;
            description.binding = 0;
            description.location = location++;
            description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            description.offset = offsetof(VertexData, VertColor);
            attributeDescriptions.push_back(description);
        }

        for (size_t i = 0; i < VertexData::MAX_COORD_NUM; i++)
        {
            VkVertexInputAttributeDescription description;
            description.binding = 0;
            description.location = location++;
            description.format = VK_FORMAT_R32G32_SFLOAT;
            description.offset = offsetof(VertexData, Coords[i]);
            attributeDescriptions.push_back(description);
        }

        return attributeDescriptions;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


class HelloTriangleApplication {
public:
    const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
    };
    const std::vector<gfx::VertexData> vertices =
    {
        {{-0.5f, -0.5f, 0.f}, {0.f,0.f,0.f}, {0.f,0.f,0.f}, {1.0f, 0.0f, 0.0f, 1.f}, {}},
        {{0.5f, -0.5f, 0.f}, {0.f,0.f,0.f}, {0.f,0.f,0.f}, {0.0f, 1.0f, 0.0f, 1.f}, {}},
        {{0.5f, 0.5f, 0.f}, {0.f,0.f,0.f}, {0.f,0.f,0.f}, {0.0f, 0.0f, 1.0f, 1.f}, {}},
        {{-0.5f, 0.5f, 0.f}, {0.f,0.f,0.f}, {0.f,0.f,0.f}, {1.0f, 1.0f, 1.0f, 1.f}, {}},
    };
    //const std::vector<Vertex> vertices = {
    //    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    //    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    //    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    //    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    //};
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    void run()
    {
        gfx::GFXGlobalConfig config;
        config.WindowHeight = 720;
        config.WindowWidth = 1280;
        strcpy(config.Title, "Puslar");
        strcpy(config.ProgramName, "Puslar");

        gfxapp = static_cast<gfx::GFXVulkanApplication*>(gfx::CreateGFXApplication(gfx::GFXApi::Vulkan, config));
        gfxapp->Initialize();

        createDescriptorSetLayout();
        createGraphicsPipeline();
        createFramebuffers();

        vertexBuffer = gfxapp->CreateBuffer(gfx::GFXBufferUsage::Vertex, sizeof(vertices[0]) * vertices.size());
        vertexBuffer->Fill(vertices.data());

        indexBuffer = (gfx::GFXVulkanBuffer*)gfxapp->CreateBuffer(gfx::GFXBufferUsage::Index, sizeof(indices[0]) * indices.size());
        indexBuffer->Fill(indices.data());

        uniformBuffers.push_back((gfx::GFXVulkanBuffer*)gfxapp->CreateBuffer(gfx::GFXBufferUsage::ConstantBuffer, sizeof(UniformBufferObject)));
        uniformBuffers.push_back((gfx::GFXVulkanBuffer*)gfxapp->CreateBuffer(gfx::GFXBufferUsage::ConstantBuffer, sizeof(UniformBufferObject)));
        createDescriptorPool();
        createDescriptorSets();

        createSyncObjects();

        gfxapp->OnLoop = [this](float dt)
        {
            drawFrame(dt);
        };
        gfxapp->ExecLoop();

        cleanup();
        gfxapp->Terminate();
    }
    void createDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(gfxapp->GetVkDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
    void createDescriptorSets()
    {

        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        //两个相同布局
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        //创建两个descriptor set
        if (vkAllocateDescriptorSets(gfxapp->GetVkDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            //引用对应的两个ubo
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i]->GetVkBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            //重新配置关联descriptor set和buffer引用
            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(gfxapp->GetVkDevice(), 1, &descriptorWrite, 0, nullptr);

        }

    }
    //绑定位置为0的顶点着色器布局的ubo
    void createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(gfxapp->GetVkDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
private:

    gfx::GFXVulkanApplication* gfxapp;
    gfx::GFXBuffer* vertexBuffer;
    gfx::GFXVulkanBuffer* indexBuffer;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    //VkPipelineLayout pipelineLayout;KC

    //VkSurfaceKHR surface;

    //VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    //VkDevice device;

    //VkQueue graphicsQueue;
    //VkQueue presentQueue;

    //VkSwapchainKHR swapChain;
    //std::vector<VkImage> swapChainImages;
    //VkFormat swapChainImageFormat;
    //VkExtent2D swapChainExtent;
    //std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    //VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<gfx::GFXVulkanBuffer*> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    //VkCommandPool commandPool;
    /*std::vector<VkCommandBuffer> commandBuffers;*/

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;
    //VkBuffer vertexBuffer;
    //VkDeviceMemory vertexBufferMemory;



    void cleanupSwapChain() {
        for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
            vkDestroyFramebuffer(gfxapp->GetVkDevice(), swapChainFramebuffers[i], nullptr);
        }

        for (size_t i = 0; i < gfxapp->GetVkSwapchainImageViews().size(); i++) {
            vkDestroyImageView(gfxapp->GetVkDevice(), gfxapp->GetVkSwapchainImageViews()[i], nullptr);
        }

        vkDestroySwapchainKHR(gfxapp->GetVkDevice(), gfxapp->GetVkSwapchain(), nullptr);
    }

    void recreateSwapChain() {

        int width = 0, height = 0;
        glfwGetFramebufferSize(reinterpret_cast<GLFWwindow*>(gfxapp->GetWindowHandle()), &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(reinterpret_cast<GLFWwindow*>(gfxapp->GetWindowHandle()), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(gfxapp->GetVkDevice());

        cleanupSwapChain();

        gfxapp->CreateSwapChain();

        //createSwapChain();
        //createImageViews();
        createFramebuffers();
    }

    void mainLoop()
    {

        while (!glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(gfxapp->GetWindowHandle()))) {
            glfwPollEvents();
            //drawFrame();
        }

        vkDeviceWaitIdle(gfxapp->GetVkDevice());
    }

    void cleanup() {

        cleanupSwapChain();

        vertexBuffer->Release();
        indexBuffer->Release();
        for (auto& uniformBuffer : uniformBuffers)
        {
            uniformBuffer->Release();
        }

        vkDestroyDescriptorPool(gfxapp->GetVkDevice(), descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(gfxapp->GetVkDevice(), descriptorSetLayout, nullptr);


        vkDestroyPipeline(gfxapp->GetVkDevice(), graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(gfxapp->GetVkDevice(), pipelineLayout, nullptr);
        vkDestroyRenderPass(gfxapp->GetVkDevice(), gfxapp->GetVkRenderPass(), nullptr);

        /*vkDestroyCommandPool(gfxapp->GetVkDevice(), gfxapp->GetVkCommandPool(), nullptr);*/
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(gfxapp->GetVkDevice(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(gfxapp->GetVkDevice(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(gfxapp->GetVkDevice(), inFlightFences[i], nullptr);
        }

    }


    void createGraphicsPipeline() {

        auto vertShaderCode = readFile("shader/lit.vert.spv");
        auto fragShaderCode = readFile("shader/lit.pixel.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = gfx::GetBindingDescription();
        auto attributeDescriptions = gfx::GetAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(gfxapp->GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = gfxapp->GetVkRenderPass();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(gfxapp->GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(gfxapp->GetVkDevice(), fragShaderModule, nullptr);
        vkDestroyShaderModule(gfxapp->GetVkDevice(), vertShaderModule, nullptr);
    }

    void createFramebuffers()
    {
        swapChainFramebuffers.resize(gfxapp->GetVkSwapchainImageViews().size());

        for (size_t i = 0; i < gfxapp->GetVkSwapchainImageViews().size(); i++) {
            VkImageView attachments[] = {
                gfxapp->GetVkSwapchainImageViews()[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = gfxapp->GetVkRenderPass();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = gfxapp->GetVkSwapChainExtent().width;
            framebufferInfo.height = gfxapp->GetVkSwapChainExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(gfxapp->GetVkDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }


    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = gfxapp->GetVkRenderPass();
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = gfxapp->GetVkSwapChainExtent();

        VkClearValue clearColor = { {{0.0f, 0.0f, 0.2f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f + gfxapp->GetVkSwapChainExtent().height;
            viewport.width = (float)gfxapp->GetVkSwapChainExtent().width;
            viewport.height = -(float)gfxapp->GetVkSwapChainExtent().height;
            //viewport.x = 0.0f;
            //viewport.y = 0.0f;
            //viewport.width = (float)swapChainExtent.width;
            //viewport.height = (float)swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = gfxapp->GetVkSwapChainExtent();
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            //vkCmdDraw(commandBuffer, 3, 1, 0, 0);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkBuffer vertexBuffers[] = { static_cast<gfx::GFXVulkanBuffer*>(vertexBuffer)->GetVkBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);
            //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        }
        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void createSyncObjects() {

        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(gfxapp->GetVkDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(gfxapp->GetVkDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(gfxapp->GetVkDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }



    }

    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), gfxapp->GetVkSwapChainExtent().width / (float)gfxapp->GetVkSwapChainExtent().height, 0.1f, 10.0f);

        uniformBuffers[currentImage]->Fill(&ubo);
    }

    void drawFrame(float)
    {

        updateUniformBuffer(currentFrame);

        //std::cout << "tick" << std::endl;
        vkWaitForFences(gfxapp->GetVkDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(gfxapp->GetVkDevice(), gfxapp->GetVkSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(gfxapp->GetVkDevice(), 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(gfxapp->GetVkCommandBuffer(currentFrame), /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(gfxapp->GetVkCommandBuffer(currentFrame), imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &gfxapp->GetVkCommandBuffer(currentFrame);

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(gfxapp->GetVkGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { gfxapp->GetVkSwapchain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(gfxapp->GetVkPresentQueue(), &presentInfo);
        vkQueueWaitIdle(gfxapp->GetVkPresentQueue());

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(gfxapp->GetVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }



    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}