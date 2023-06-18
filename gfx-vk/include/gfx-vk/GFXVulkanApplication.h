#pragma once
#include <gfx/GFXApplication.h>

#include "VulkanInclude.h"

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/include/GLFW/glfw3.h>
#include <glfw/include/GLFW/glfw3native.h>

#include <chrono>

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

        void TickRender(float deltaTime);
        virtual GFXBuffer* CreateBuffer(GFXBufferUsage usage, size_t bufferSize) override;
        virtual std::shared_ptr<GFXCommandBuffer> CreateCommandBuffer() override;
        virtual std::shared_ptr<GFXVertexLayoutDescription> CreateVertexLayoutDescription() override;
        virtual std::shared_ptr<GFXImage> CreateImage() override;
        virtual std::shared_ptr<GFXShaderModule> CreateShaderModule(const std::vector<uint8_t>& vert, const std::vector<uint8_t>& frag) override;
        virtual std::shared_ptr<GFXGraphicsPipeline> CreateGraphicsPipeline(
            const GFXGraphicsPipelineConfig& config,
            std::shared_ptr<GFXVertexLayoutDescription> VertexLayout,
            std::shared_ptr<GFXShaderModule> ShaderModule,
            const std::shared_ptr<GFXDescriptorSetLayout>& descSetLayout) override;

        virtual std::shared_ptr<GFXTexture2D> CreateTexture2DFromMemory(
            const uint8_t* data, int32_t length,
            bool enableReadWrite = false, GFXTextureFormat format = GFXTextureFormat::R8G8B8A8_SRGB) override;
        virtual GFXDescriptorManager* GetDescriptorManager() override;

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
        const std::vector<VkCommandBuffer> GetVkCommandBuffers() const;
        const VkCommandBuffer& GetVkCommandBuffer(size_t index) const;
        class GFXVulkanCommandBuffer* GetCommandBuffer(size_t index) const { return m_commandBuffers[index].get(); }
        VkImageView GetVkDepthImageView() const { return m_depthImageView; }
        const std::vector<VkFramebuffer>& GetVkFrameBuffers() const { return m_swapChainFramebuffers; }

        class GFXVulkanRenderPass* GetSwapChainRenderPass() const { return m_renderPass; }
    protected:
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    private:
        void InitVkInstance();

        void InitPickPhysicalDevice();
        void InitLogicalDevice();
        void InitCommandPool();
        void InitDescriptorPool();
    public:
        void InitSwapChain();
        void TermSwapChain();
        void InitRenderPass();
        void InitCommandBuffers();
        void InitDepthTestBuffer();
        void TermDepthTestBuffer();
        void InitFrameBuffers();
        void TermFrameBuffers();
        void ReInitSwapChain();
    protected:

        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        GLFWwindow* m_window = nullptr;
        bool m_framebufferResized = false;

        VkInstance m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
        VkCommandPool m_commandPool = VK_NULL_HANDLE;

        VkImage m_depthImage = VK_NULL_HANDLE;
        VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
        VkImageView m_depthImageView = VK_NULL_HANDLE;

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;

        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<VkFramebuffer> m_swapChainFramebuffers;

        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        std::vector<std::unique_ptr<class GFXVulkanCommandBuffer>> m_commandBuffers;

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;

        class GFXVulkanRenderPass* m_renderPass = nullptr;
        class GFXVulkanDescriptorManager* m_descriptorManager = nullptr;
        class GFXVulkanRenderer* m_renderer = nullptr;

        array_list<char*> m_extensions;
        size_t m_count = 0;

        bool m_isAppEnding = false;

        std::chrono::steady_clock::time_point m_startTime;
        std::chrono::steady_clock::time_point m_lastTime;
    };
}