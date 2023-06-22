#pragma once
#include "VulkanInclude.h"
#include <vector>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXRenderTarget.h>
#include "GFXVulkanRenderTarget.h"

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
        void RecordCommandBuffer(GFXCommandBuffer* commandBuffer, const std::vector<GFXRenderTarget*>& renderTarget);
    protected:
        GFXVulkanApplication* m_app;
        GFXVulkanRenderTarget* m_renderTarget;

        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
        bool m_framebufferResized = false;
        uint32_t m_currentFrame = 0;
        const int MAX_FRAMES_IN_FLIGHT = 2;
    };
}