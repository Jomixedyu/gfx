#pragma once
#include <gfx/GFXCommandBuffer.h>
#include "GFXVulkanApplication.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanCommandBuffer : public GFXCommandBuffer
    {

    public:
        GFXVulkanCommandBuffer(GFXVulkanApplication* app)
            : m_app(app)
        {

        }

        virtual void Begin() override;
        virtual void End() override;

    public:
        virtual GFXApplication* GetApplication() const override
        {
            return m_app;
        }
        VkCommandBuffer GetVkCommandBuffer() const { return m_cmdBuffer; }
    protected:
        VkCommandBuffer m_cmdBuffer;
        GFXVulkanApplication* m_app;
    };
}