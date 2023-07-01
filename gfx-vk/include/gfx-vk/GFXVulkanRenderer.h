#pragma once
#include "VulkanInclude.h"
#include <vector>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXRenderTarget.h>
#include "GFXVulkanRenderTarget.h"
#include "GFXVulkanRenderContext.h"

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

        bool m_framebufferResized = false;
        uint32_t m_currentFrame = 0;
        const int MAX_FRAMES_IN_FLIGHT = 2;

    };
}