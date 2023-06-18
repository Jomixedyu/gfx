#include "GFXVulkanApplication.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanApplication.h"
#include <glfw/include/GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include "PhysicalDeviceHelper.h"
#include "BufferHelper.h"
#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanBuffer.h"
#include "GFXVulkanVertexLayoutDescription.h"
#include "GFXVulkanTexture2D.h"
#include "GFXVulkanDescriptorManager.h"
#include "GFXVulkanShaderModule.h"
#include "GFXVulkanGraphicsPipeline.h"
#include "GFXVulkanRenderer.h"
#include "GFXVulkanRenderPass.h"
#include <set>
#include <cmath>
#include <array>
#include <algorithm>
#include <stdlib.h>

#undef max

namespace gfx
{
    GFXExtensions GFXVulkanApplication::GetExtensionNames()
    {
        if (m_extensions.empty())
        {
            uint32_t     glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            m_count = glfwExtensionCount;

            for (uint32_t i = 0; i < glfwExtensionCount; i++)
            {
                const size_t strlen = ::strlen(glfwExtensions[i]);
                const size_t bufferSize = strlen + 1;

                auto str = new char[bufferSize];
                ::strcpy(str, glfwExtensions[i]);

                m_extensions.push_back(str);
            }

            if (GetConfig().EnableValid)
            {
                const size_t bufferSize = ::strlen(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) + 1;

                auto str = new char[bufferSize];
                ::strcpy(str, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

                m_extensions.push_back(str);
                ++m_count;
            }
        }

        return GFXExtensions(const_cast<const char* const*>(m_extensions.data()), m_count);
    }

    intptr_t GFXVulkanApplication::GetWindowHandle()
    {
        return reinterpret_cast<intptr_t>(m_window);
    }

    const std::vector<VkCommandBuffer> GFXVulkanApplication::GetVkCommandBuffers() const
    {
        std::vector<VkCommandBuffer> ret;
        for (auto& buffer : m_commandBuffers)
        {
            ret.push_back(buffer->GetVkCommandBuffer());
        }
        return ret;
    }

    const VkCommandBuffer& GFXVulkanApplication::GetVkCommandBuffer(size_t index) const
    {
        return m_commandBuffers[index]->GetVkCommandBuffer();
    }

    void GFXVulkanApplication::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<GFXVulkanApplication*>(glfwGetWindowUserPointer(window));
        app->m_framebufferResized = true;
    }

    static const std::vector<const char*> validationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };

    static bool _CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
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

    enum class ConsoleColor
    {
        black = 0,
        dark_blue = 1,
        dark_green = 2,
        dark_aqua, dark_cyan = 3,
        dark_red = 4,
        dark_purple = 5, dark_pink = 5, dark_magenta = 5,
        dark_yellow = 6,
        dark_white = 7,
        gray = 8,
        blue = 9,
        green = 10,
        aqua = 11, cyan = 11,
        red = 12,
        purple = 13, pink = 13, magenta = 13,
        yellow = 14,
        white = 15
    };
    static void _SetColor(ConsoleColor foreground, ConsoleColor background)
    {
        WORD consoleColor;
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;

        if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
        {
            consoleColor = ((int)foreground + ((int)background * 16));
            SetConsoleTextAttribute(hStdOut, consoleColor);
        }
    }
    static VKAPI_ATTR VkBool32 VKAPI_CALL _VkDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            _SetColor(ConsoleColor::red, ConsoleColor::black);
        }
        else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            _SetColor(ConsoleColor::yellow, ConsoleColor::black);
        }
        else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            _SetColor(ConsoleColor::blue, ConsoleColor::black);
        }
        else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            _SetColor(ConsoleColor::white, ConsoleColor::black);
        }
        else
        {
            _SetColor(ConsoleColor::gray, ConsoleColor::black);
        }


        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
    static void _PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        //| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        createInfo.pfnUserCallback = _VkDebugCallback;
    }
    static VkResult _CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void GFXVulkanApplication::InitVkInstance()
    {
        if (m_config.EnableValid && !_CheckValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = m_config.ProgramName;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = m_config.ProgramName;
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = GetExtensionNames();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.GetCount());
        createInfo.ppEnabledExtensionNames = extensions.GetExtensionNames();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (m_config.EnableValid)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            _PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }

    }


    void GFXVulkanApplication::InitPickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            if (vk::PhysicalDeviceHelper::IsDeviceSuitable(m_surface, device))
            {
                m_physicalDevice = device;
                break;
            }
        }

        if (m_physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void GFXVulkanApplication::InitLogicalDevice()
    {
        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        vk::QueueFamilyIndices indices = vk::PhysicalDeviceHelper::FindQueueFamilies(m_surface, m_physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (m_config.EnableValid)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            _PopulateDebugMessengerCreateInfo(debugCreateInfo);
            //createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
    }

    void GFXVulkanApplication::InitCommandPool()
    {
        vk::QueueFamilyIndices queueFamilyIndices = vk::PhysicalDeviceHelper::FindQueueFamilies(m_surface, m_physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    static SwapChainSupportDetails _QuerySwapChainSupport(GFXVulkanApplication* app)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(app->GetVkPhysicalDevice(), app->GetVkSurface(), &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(app->GetVkPhysicalDevice(), app->GetVkSurface(), &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(app->GetVkPhysicalDevice(), app->GetVkSurface(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(app->GetVkPhysicalDevice(), app->GetVkSurface(), &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(app->GetVkPhysicalDevice(), app->GetVkSurface(), &presentModeCount, details.presentModes.data());
        }

        return details;
    }
    static VkSurfaceFormatKHR _ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }
    VkPresentModeKHR _ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }
    static VkExtent2D _ChooseSwapExtent(GFXVulkanApplication* app, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(reinterpret_cast<GLFWwindow*>(app->GetWindowHandle()), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    void GFXVulkanApplication::InitSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = _QuerySwapChainSupport(this);
        VkSurfaceFormatKHR surfaceFormat = _ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = _ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = _ChooseSwapExtent(this, swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


        vk::QueueFamilyIndices indices = vk::PhysicalDeviceHelper::FindQueueFamilies(m_surface, m_physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        //create swapchain images
        vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

        //create swapchain image view
        m_swapChainImageViews.resize(m_swapChainImages.size());

        for (size_t i = 0; i < m_swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }
    void GFXVulkanApplication::TermSwapChain()
    {
        for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
        {
            vkDestroyImageView(m_device, m_swapChainImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

    }

    static bool _HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void GFXVulkanApplication::InitRenderPass()
    {
        m_renderPass = new GFXVulkanRenderPass(this);
    }

    void GFXVulkanApplication::InitCommandBuffers()
    {
        std::vector<VkCommandBuffer> buffers;
        buffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = buffers.size();

        if (vkAllocateCommandBuffers(m_device, &allocInfo, buffers.data()) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (size_t i = 0; i < buffers.size(); i++)
        {
            m_commandBuffers.push_back(std::unique_ptr<GFXVulkanCommandBuffer>(new GFXVulkanCommandBuffer(this)));
        }
    }


    void GFXVulkanApplication::InitDepthTestBuffer()
    {
        this->TermDepthTestBuffer();
        VkFormat depthFormat = BufferHelper::FindDepthFormat(this);
        auto extent = this->GetVkSwapChainExtent();
        BufferHelper::CreateImage(
            this, extent.width, extent.height, depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_depthImage, m_depthImageMemory);
        m_depthImageView = BufferHelper::CreateImageView(this, m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        BufferHelper::TransitionImageLayout(this, m_depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    void GFXVulkanApplication::TermDepthTestBuffer()
    {
        if (m_depthImage != VK_NULL_HANDLE)
        {
            vkDestroyImageView(m_device, m_depthImageView, nullptr);
            vkDestroyImage(m_device, m_depthImage, nullptr);
            vkFreeMemory(m_device, m_depthImageMemory, nullptr);
            m_depthImage = VK_NULL_HANDLE;
        }
    }

    void GFXVulkanApplication::InitFrameBuffers()
    {
        this->TermFrameBuffers();
        m_swapChainFramebuffers.resize(GetVkSwapchainImageViews().size());

        for (size_t i = 0; i < GetVkSwapchainImageViews().size(); i++) {

            std::array<VkImageView, 2> attachments =
            {
                GetVkSwapchainImageViews()[i],
                GetVkDepthImageView()
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass->GetVkRenderPass();
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = GetVkSwapChainExtent().width;
            framebufferInfo.height = GetVkSwapChainExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(GetVkDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }


    void GFXVulkanApplication::TermFrameBuffers()
    {
        for (size_t i = 0; i < m_swapChainFramebuffers.size(); i++)
        {
            vkDestroyFramebuffer(GetVkDevice(), m_swapChainFramebuffers[i], nullptr);
        }
        m_swapChainFramebuffers.clear();
    }

    void GFXVulkanApplication::InitDescriptorPool()
    {
        m_descriptorManager = new GFXVulkanDescriptorManager(this);
    }
    void GFXVulkanApplication::ReInitSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_device);

        TermSwapChain();

        InitSwapChain();
        InitDepthTestBuffer();
        InitFrameBuffers();
    }
    void GFXVulkanApplication::Initialize()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_window = glfwCreateWindow(m_config.WindowWidth, m_config.WindowHeight, m_config.Title, nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);

        this->InitVkInstance();

        // setup debuger
        if (m_config.EnableValid)
        {
            VkDebugUtilsMessengerCreateInfoEXT createInfo{};
            _PopulateDebugMessengerCreateInfo(createInfo);

            //if (_CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) 
            //{
            //    throw std::runtime_error("failed to set up debug messenger!");
            //}
        }
        // create surface
        if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }

        this->InitPickPhysicalDevice();
        this->InitLogicalDevice();
        this->InitCommandPool();
        this->InitSwapChain();
        this->InitRenderPass();
        this->InitCommandBuffers();
        this->InitDepthTestBuffer();
        this->InitFrameBuffers();
        this->InitDescriptorPool();

        m_renderer = new GFXVulkanRenderer(this);
    }

    void GFXVulkanApplication::ExecLoop()
    {
        m_startTime = std::chrono::high_resolution_clock::now();

        while (!m_isAppEnding)
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_lastTime).count();

            if (glfwWindowShouldClose(m_window))
            {
                if (OnExitWindow)
                {
                    if (OnExitWindow())
                    {
                        m_isAppEnding = true;
                    }
                }
                else
                {
                    m_isAppEnding = true;
                }
            }

            glfwPollEvents();


            TickRender(deltaTime);
            if (OnLoop)
            {
                OnLoop(deltaTime);
            }
            m_lastTime = currentTime;
        }
        vkDeviceWaitIdle(m_device);
    }
    void GFXVulkanApplication::TickRender(float deltaTime)
    {
        m_renderer->Render();
    }

    void GFXVulkanApplication::RequestStop()
    {
        m_isAppEnding = true;
    }

    void GFXVulkanApplication::Terminate()
    {
        delete m_renderPass;

        this->TermSwapChain();
        this->TermDepthTestBuffer();
        this->TermFrameBuffers();

        m_commandBuffers.clear();

        delete m_descriptorManager;
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);


        vkDestroyDevice(m_device, nullptr);

        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

        if (m_config.EnableValid)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr && m_debugMessenger != VK_NULL_HANDLE)
            {
                func(m_instance, m_debugMessenger, nullptr);
            }
        }

        vkDestroyInstance(m_instance, nullptr);

        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    GFXBuffer* GFXVulkanApplication::CreateBuffer(GFXBufferUsage usage, size_t bufferSize)
    {
        return new GFXVulkanBuffer(this, usage, bufferSize);
    }
    std::shared_ptr<GFXCommandBuffer> gfx::GFXVulkanApplication::CreateCommandBuffer()
    {
        return std::shared_ptr<GFXCommandBuffer>(new GFXVulkanCommandBuffer(this));
    }
    std::shared_ptr<GFXVertexLayoutDescription> gfx::GFXVulkanApplication::CreateVertexLayoutDescription()
    {
        return std::shared_ptr<GFXVertexLayoutDescription>(new GFXVulkanVertexLayoutDescription());
    }
    std::shared_ptr<GFXImage> GFXVulkanApplication::CreateImage()
    {
        return std::shared_ptr<GFXImage>();
    }

    std::shared_ptr<GFXTexture2D> gfx::GFXVulkanApplication::CreateTexture2DFromMemory(
        const uint8_t* data, int32_t length, bool enableReadWrite, GFXTextureFormat format)
    {
        return GFXVulkanTexture2D::CreateFromMemory(this, data, length, enableReadWrite, format);
    }

    std::shared_ptr<GFXShaderModule> GFXVulkanApplication::CreateShaderModule(const std::vector<uint8_t>& vert, const std::vector<uint8_t>& frag)
    {
        return std::shared_ptr<GFXShaderModule>(new GFXVulkanShaderModule(this, vert, frag));
    }

    std::shared_ptr<GFXGraphicsPipeline> GFXVulkanApplication::CreateGraphicsPipeline(
        const GFXGraphicsPipelineConfig& config, 
        std::shared_ptr<GFXVertexLayoutDescription> vertexLayout, 
        std::shared_ptr<GFXShaderModule> shaderModule,
        const std::shared_ptr<GFXDescriptorSetLayout>& descSetLayout)
    {
        auto vkPipeline = new GFXVulkanGraphicsPipeline(this, config, vertexLayout, shaderModule, descSetLayout);
        return std::shared_ptr<GFXGraphicsPipeline>(vkPipeline);
    }

    GFXDescriptorManager* GFXVulkanApplication::GetDescriptorManager()
    {
        return m_descriptorManager;
    }
}