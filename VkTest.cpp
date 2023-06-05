
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
#include <glm/glm/glm.hpp>

#include <gfx/GFXApplication.h>
#include <GFXVulkanApplication.h>
#include <GFXVulkanBuffer.h>

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

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        return attributeDescriptions;
    }
};

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
    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
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

        createGraphicsPipeline();
        createFramebuffers();

        vertexBuffer = gfxapp->CreateBuffer();
        vertexBuffer->Fill(gfx::GFXBufferUsage::Vertex, vertices.data(), sizeof(vertices[0]) * vertices.size());
        indexBuffer = (gfx::GFXVulkanBuffer*)gfxapp->CreateBuffer();
        indexBuffer->Fill(gfx::GFXBufferUsage::Index, indices.data(), sizeof(indices[0]) * indices.size());

        createSyncObjects();

        gfxapp->OnLoop = [this](float dt)
        {
            drawFrame(dt);
        };
        gfxapp->ExecLoop();

        cleanup();
        gfxapp->Terminate();
    }

private:
    gfx::GFXVulkanApplication* gfxapp;
    gfx::GFXBuffer* vertexBuffer;
    gfx::GFXVulkanBuffer* indexBuffer;

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

    //VkCommandPool commandPool;
    /*std::vector<VkCommandBuffer> commandBuffers;*/

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;
    //VkBuffer vertexBuffer;
    //VkDeviceMemory vertexBufferMemory;

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void initVulkan() 
    {


    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(gfxapp->GetVkPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void createVertexBuffer() {
        vertexBuffer = gfxapp->CreateBuffer();
        vertexBuffer->Fill(gfx::GFXBufferUsage::Vertex, vertices.data(), sizeof(vertices[0]) * vertices.size());
        
    }

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
        /*vkDestroyBuffer(gfxapp->GetVkDevice(), vertexBuffer, nullptr);
        vkFreeMemory(gfxapp->GetVkDevice(), vertexBufferMemory, nullptr);*/

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

        //vkDestroyDevice(device, nullptr);

        //if (enableValidationLayers) {
        //    //DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        //}

        //vkDestroySurfaceKHR(gfxapp->GetVkInstance(), surface, nullptr);
        //vkDestroyInstance(gfxapp->GetVkInstance(), nullptr);

        
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    //void createSwapChain() 
    //{
    //    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(gfxapp->GetVkPhysicalDevice());

    //    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    //    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    //    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    //    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    //    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    //        imageCount = swapChainSupport.capabilities.maxImageCount;
    //    }

    //    VkSwapchainCreateInfoKHR createInfo{};
    //    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    //    createInfo.surface = gfxapp->GetVkSurface();

    //    createInfo.minImageCount = imageCount;
    //    createInfo.imageFormat = surfaceFormat.format;
    //    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    //    createInfo.imageExtent = extent;
    //    createInfo.imageArrayLayers = 1;
    //    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //    QueueFamilyIndices indices = findQueueFamilies(gfxapp->GetVkPhysicalDevice());
    //    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    //    if (indices.graphicsFamily != indices.presentFamily) {
    //        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    //        createInfo.queueFamilyIndexCount = 2;
    //        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    //    }
    //    else {
    //        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //    }

    //    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    //    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    //    createInfo.presentMode = presentMode;
    //    createInfo.clipped = VK_TRUE;

    //    createInfo.oldSwapchain = VK_NULL_HANDLE;

    //    if (vkCreateSwapchainKHR(gfxapp->GetVkDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create swap chain!");
    //    }

    //    //create spawnchain image
    //    vkGetSwapchainImagesKHR(gfxapp->GetVkDevice(), swapChain, &imageCount, nullptr);
    //    swapChainImages.resize(imageCount);
    //    vkGetSwapchainImagesKHR(gfxapp->GetVkDevice(), swapChain, &imageCount, swapChainImages.data());

    //    swapChainImageFormat = surfaceFormat.format;
    //    swapChainExtent = extent;
    //}

    //void createImageViews() {
    //    swapChainImageViews.resize(swapChainImages.size());

    //    for (size_t i = 0; i < swapChainImages.size(); i++) {
    //        VkImageViewCreateInfo createInfo{};
    //        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    //        createInfo.image = swapChainImages[i];
    //        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    //        createInfo.format = swapChainImageFormat;
    //        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    //        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    //        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    //        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    //        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //        createInfo.subresourceRange.baseMipLevel = 0;
    //        createInfo.subresourceRange.levelCount = 1;
    //        createInfo.subresourceRange.baseArrayLayer = 0;
    //        createInfo.subresourceRange.layerCount = 1;

    //        if (vkCreateImageView(gfxapp->GetVkDevice(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
    //            throw std::runtime_error("failed to create image views!");
    //        }
    //    }
    //}

    //void createRenderPass() {
    //    VkAttachmentDescription colorAttachment{};
    //    colorAttachment.format = gfxapp->GetSwapChainImageFormat();
    //    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    //    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //    VkAttachmentReference colorAttachmentRef{};
    //    colorAttachmentRef.attachment = 0;
    //    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //    VkSubpassDescription subpass{};
    //    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //    subpass.colorAttachmentCount = 1;
    //    subpass.pColorAttachments = &colorAttachmentRef;

    //    VkSubpassDependency dependency{};
    //    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    //    dependency.dstSubpass = 0;
    //    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //    dependency.srcAccessMask = 0;
    //    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    //    VkRenderPassCreateInfo renderPassInfo{};
    //    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    //    renderPassInfo.attachmentCount = 1;
    //    renderPassInfo.pAttachments = &colorAttachment;
    //    renderPassInfo.subpassCount = 1;
    //    renderPassInfo.pSubpasses = &subpass;
    //    renderPassInfo.dependencyCount = 1;
    //    renderPassInfo.pDependencies = &dependency;

    //    if (vkCreateRenderPass(gfxapp->GetVkDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create render pass!");
    //    }
    //}

    void createGraphicsPipeline() {

        auto vertShaderCode = readFile("shader/vert.spv");
        auto fragShaderCode = readFile("shader/frag.spv");

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

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

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
        pipelineLayoutInfo.setLayoutCount = 0;
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

    //void createCommandPool() {
    //    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(gfxapp->GetVkPhysicalDevice());

    //    VkCommandPoolCreateInfo poolInfo{};
    //    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    //    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    //    if (vkCreateCommandPool(gfxapp->GetVkDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create command pool!");
    //    }
    //}

    void createCommandBuffer() {
        //commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        //VkCommandBufferAllocateInfo allocInfo{};
        //allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        //allocInfo.commandPool = gfxapp->GetVkCommandPool();
        //allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        //allocInfo.commandBufferCount = commandBuffers.size();

        //if (vkAllocateCommandBuffers(gfxapp->GetVkDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        //    throw std::runtime_error("failed to allocate command buffers!");
        //}
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

    void drawFrame(float) {
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

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(reinterpret_cast<GLFWwindow*>(gfxapp->GetWindowHandle()), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, gfxapp->GetVkSurface(), &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, gfxapp->GetVkSurface(), &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, gfxapp->GetVkSurface(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, gfxapp->GetVkSurface(), &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, gfxapp->GetVkSurface(), &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, gfxapp->GetVkSurface(), &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
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
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}