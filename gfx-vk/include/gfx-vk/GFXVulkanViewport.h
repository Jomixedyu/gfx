#pragma once
#include "VulkanInclude.h"
#include <vector>
#include <memory>
#include "GFXVulkanCommandBuffer.h"

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/include/GLFW/glfw3.h>
#include <glfw/include/GLFW/glfw3native.h>

#include <gfx/GFXViewport.h>
#include "GFXVulkanRenderTarget.h"
#include "GFXVulkanTexture2D.h"
#include "GFXVulkanFrameBuffer.h"
#include "GFXVulkanQueue.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanViewport : public GFXViewport
    {

    public:

        GFXVulkanViewport(GFXVulkanApplication* app, GLFWwindow* window);
        virtual ~GFXVulkanViewport() override;

        void InitSwapChain();
        void TermSwapChain();
        void InitFrameBuffers();
        void TermFrameBuffers();
        void InitDepthTestBuffer();
        void TermDepthTestBuffer();
        void InitCommandBuffers();
        void InitRenderPass();
        void InitQueue();
        void ReInitSwapChain();
    public:
        VkSwapchainKHR GetVkSwapChain() const { return m_swapChain; }
        VkImage GetVkSwapChainImage() const { return m_swapChainImages[m_imageIndex]; }
        VkImageView GetVkSwapChainImageView() const { return m_swapChainImageViews[m_imageIndex]; }
        VkExtent2D GetVkSwapChainExtent() const { return m_swapChainExtent; }
        VkFormat GetVkSwapChainImageFormat() const { return m_swapChainImageFormat; }
        GFXVulkanApplication* GetApplication() const { return m_app; }
        GFXVulkanQueue* GetQueue() const { return m_queues[m_currentFrame]; }
        VkResult AcquireNextImage(uint32_t* outIndex);
    public:
        virtual GFXRenderTarget* GetRenderTarget() override;
    protected:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
        bool m_framebufferResized = false;

        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        std::vector<GFXVulkanRenderTarget*> m_renderTargets;
        std::vector<GFXVulkanQueue*> m_queues;

        GFXVulkanTexture2D* m_depthTex = nullptr;

        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        uint32_t m_imageIndex = 0;
    public:
        int32_t m_currentFrame = 0;

        GFXVulkanApplication* m_app;
        GLFWwindow* m_window = nullptr;
    };
}