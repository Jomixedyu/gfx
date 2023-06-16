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


        virtual void CmdClear(float r, float g, float b, float a, bool depth, bool stencil) override;
        virtual void CmdBindPipeline(GFXGraphicsPipeline* pipeline) override;
        virtual void CmdBindVertexBuffers() override;
        virtual void CmdBindIndexBuffer() override;
        virtual void CmdBindDescriptorSets() override;
        virtual void CmdDrawIndexed() override;

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