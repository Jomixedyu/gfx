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
        void ReInitSwapChain();
    public:
        VkSwapchainKHR GetVkSwapChain() const { return m_swapChain; }
        VkImage GetVkSwapChainImage() const { return m_swapChainImages[m_currentFrame]; }
        VkImageView GetVkSwapChainImageView() const { return m_swapChainImageViews[m_currentFrame]; }
        VkFramebuffer GetVkSwapChainFrameBuffer() const { return m_swapChainFramebuffers[m_currentFrame]; }
        VkExtent2D GetVkSwapChainExtent() const { return m_swapChainExtent; }
        VkFormat GetCkSwapChainImageFormat() const { return m_swapChainImageFormat; }
        GFXVulkanCommandBuffer* GetVkCommandBuffer() const { return m_commandBuffers[m_currentFrame].get(); }
        GFXVulkanApplication* GetApplication() const { return m_app; }
    public:
        virtual GFXRenderTarget* GetRenderTarget() override;
    protected:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<VkFramebuffer> m_swapChainFramebuffers;
        std::vector<std::unique_ptr<GFXVulkanCommandBuffer>> m_commandBuffers;

        VkRenderPass m_renderPass = VK_NULL_HANDLE;

        GFXVulkanTexture2D* m_depthTex;

        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        int32_t m_currentFrame = 0;

        GFXVulkanApplication* m_app;
        GLFWwindow* m_window = nullptr;

        GFXVulkanRenderTarget* m_renderTarget;
    };
}