#include "GFXVulkanViewport.h"
#include "GFXVulkanApplication.h"
#include "PhysicalDeviceHelper.h"
#include "BufferHelper.h"
#include <array>

#ifdef max
#undef max
#endif

namespace gfx
{
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
    GFXVulkanViewport::GFXVulkanViewport(GFXVulkanApplication* app, GLFWwindow* window)
        : m_app(app), m_window(window)
    {
        this->InitSwapChain();
        this->InitRenderPass();
        this->InitCommandBuffers();
        this->InitDepthTestBuffer();
        this->InitFrameBuffers();
    }
    GFXVulkanViewport::~GFXVulkanViewport()
    {
        vkDestroyRenderPass(m_app->GetVkDevice(), m_renderPass, nullptr);
    }
    void GFXVulkanViewport::InitRenderPass()
    {

    }

    void GFXVulkanViewport::InitCommandBuffers()
    {
        std::vector<VkCommandBuffer> buffers;
        buffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_app->GetVkCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = buffers.size();

        if (vkAllocateCommandBuffers(m_app->GetVkDevice(), &allocInfo, buffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (size_t i = 0; i < buffers.size(); i++)
        {
            m_commandBuffers.push_back(std::unique_ptr<GFXVulkanCommandBuffer>(new GFXVulkanCommandBuffer(m_app)));
        }
    }
    void GFXVulkanViewport::InitDepthTestBuffer()
    {
        this->TermDepthTestBuffer();
        VkFormat depthFormat = BufferHelper::FindDepthFormat(m_app);
        auto extent = m_swapChainExtent;

        VkImage depthImage;
        VkDeviceMemory depthMemory;

        BufferHelper::CreateImage(
            m_app, extent.width, extent.height, depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            depthImage, depthMemory);
        auto depthImageView = BufferHelper::CreateImageView(m_app, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        BufferHelper::TransitionImageLayout(m_app, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        m_depthTex = new GFXVulkanTexture2D(m_app, extent.width, extent.height, 1, depthFormat, depthImage, depthMemory, depthImageView, false, GFXSamplerConfig{});
    }

    void GFXVulkanViewport::TermDepthTestBuffer()
    {
        if (m_depthTex != nullptr)
        {
            delete m_depthTex;
            m_depthTex = nullptr;
        }
    }

    void GFXVulkanViewport::InitFrameBuffers()
    {
        this->TermFrameBuffers();
        m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

        for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {

            std::array<VkImageView, 2> attachments =
            {
                m_swapChainImageViews[i],
                m_depthTex->GetVkImageView()
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_swapChainExtent.width;
            framebufferInfo.height = m_swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_app->GetVkDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void GFXVulkanViewport::TermFrameBuffers()
    {
        for (size_t i = 0; i < m_swapChainFramebuffers.size(); i++)
        {
            vkDestroyFramebuffer(m_app->GetVkDevice(), m_swapChainFramebuffers[i], nullptr);
        }
        m_swapChainFramebuffers.clear();
    }


    void GFXVulkanViewport::InitSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = _QuerySwapChainSupport(m_app);
        VkSurfaceFormatKHR surfaceFormat = _ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = _ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = _ChooseSwapExtent(m_app, swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_app->GetVkSurface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


        vk::QueueFamilyIndices indices = vk::PhysicalDeviceHelper::FindQueueFamilies(m_app->GetVkSurface(), m_app->GetVkPhysicalDevice());
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

        if (vkCreateSwapchainKHR(m_app->GetVkDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        //create swapchain images
        vkGetSwapchainImagesKHR(m_app->GetVkDevice(), m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_app->GetVkDevice(), m_swapChain, &imageCount, m_swapChainImages.data());

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

            if (vkCreateImageView(m_app->GetVkDevice(), &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    void GFXVulkanViewport::TermSwapChain()
    {
        for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
        {
            vkDestroyImageView(m_app->GetVkDevice(), m_swapChainImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(m_app->GetVkDevice(), m_swapChain, nullptr);
    }

    void GFXVulkanViewport::ReInitSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_app->GetVkDevice());

        TermSwapChain();

        InitSwapChain();
        InitDepthTestBuffer();
        InitFrameBuffers();
    }
    GFXRenderTarget* gfx::GFXVulkanViewport::GetRenderTarget()
    {
        return m_renderTarget;
    }
}