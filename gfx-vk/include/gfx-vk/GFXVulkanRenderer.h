#pragma once
#include "VulkanInclude.h"
#include <vector>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanRenderer
    {
    public:
        GFXVulkanRenderer(GFXVulkanApplication* app);
        ~GFXVulkanRenderer();
    public:
        void Render();

    protected:
        void RecordCommandBuffer(VkCommandBuffer cmdBuffer, uint32_t imageIndex);
    protected:
        GFXVulkanApplication* m_app;

        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
        bool m_framebufferResized = false;
        uint32_t m_currentFrame = 0;
        const int MAX_FRAMES_IN_FLIGHT = 2;
    };
}