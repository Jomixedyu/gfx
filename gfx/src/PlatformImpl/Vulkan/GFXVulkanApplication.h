#pragma once
#include <gfx/GFXApplication.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <gfx/GFXThirdParty/glfw/include/GLFW/glfw3.h>
#include <gfx/GFXThirdParty/glfw/include/GLFW/glfw3native.h>


namespace gfx
{
    class GFXVulkanApplication : public GFXApplication
    {
    public:
        GFXVulkanApplication(GFXGlobalConfig config)
        {
            m_config = config;
        }


    public:
        virtual void Initialize() override;
        virtual void ExecLoop() override;
        virtual void RequestStop() override;
        virtual void Terminate() override;

        virtual GFXBuffer* CreateBuffer(GFXBufferUsage usage, size_t bufferSize) override;
        virtual std::shared_ptr<GFXCommandBuffer> CreateCommandBuffer() override;
        virtual GFXExtensions GetExtensionNames() override;
        virtual intptr_t GetWindowHandle() override;
    public:
        const VkDevice& GetVkDevice() const { return m_device; }
        const VkPhysicalDevice& GetVkPhysicalDevice() const { return m_physicalDevice; }
        const VkInstance& GetVkInstance() const { return m_instance; }
        const VkSurfaceKHR& GetVkSurface() const { return m_surface; }
        const VkQueue& GetVkGraphicsQueue() const { return m_graphicsQueue; }
        const VkQueue& GetVkPresentQueue() const { return m_presentQueue; }
        const VkCommandPool& GetVkCommandPool() const { return m_commandPool; }
        const VkSwapchainKHR& GetVkSwapchain() const { return m_swapChain; }
        const VkFormat& GetSwapChainImageFormat() const { return m_swapChainImageFormat; }
        const std::vector<VkImageView>& GetVkSwapchainImageViews() const { return m_swapChainImageViews; }
        const VkExtent2D& GetVkSwapChainExtent() const { return m_swapChainExtent; }
        const VkRenderPass& GetVkRenderPass() const { return m_renderPass; }
        const std::vector<VkCommandBuffer> GetVkCommandBuffers() const { return m_commandBuffers; }
        const VkCommandBuffer& GetVkCommandBuffer(size_t index) const { return m_commandBuffers[index]; }
    protected:
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    private:
        void InitVkInstance();

        void InitPickPhysicalDevice();
        void InitLogicalDevice();
        void InitCommandPool();
    public:
        void CreateSwapChain();
        void CreateRenderPass();
        void CreateCommandBuffers();
    protected:
        
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        GLFWwindow* m_window;
        bool m_framebufferResized = false;

        VkInstance m_instance;
        VkSurfaceKHR m_surface;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        VkCommandPool m_commandPool;

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device;

        VkSwapchainKHR m_swapChain;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<VkFramebuffer> m_swapChainFramebuffers;

        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        std::vector<VkCommandBuffer> m_commandBuffers;

        VkRenderPass m_renderPass;

        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;

        array_list<char*> m_extensions;
        size_t m_count = 0;

        bool m_isAppEnding = false;
    };
}