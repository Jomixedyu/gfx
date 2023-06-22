#pragma once
#include <memory>
#include "GFXShaderPass.h"

namespace gfx
{
    class GFXApplication;

    class GFXCommandBuffer
    {
    public:
        virtual void Begin() = 0;
        virtual void End() = 0;

        GFXCommandBuffer() {}
        GFXCommandBuffer(const GFXCommandBuffer&) = delete;
        GFXCommandBuffer(GFXCommandBuffer&&) = delete;
        virtual ~GFXCommandBuffer() {}

        virtual void CmdClear(float r, float g, float b, float a, bool depth, bool stencil) = 0;
        virtual void CmdBindPipeline(GFXShaderPass* pipeline) = 0;
        virtual void CmdBindVertexBuffers() = 0;
        virtual void CmdBindIndexBuffer() = 0;
        virtual void CmdBindDescriptorSets() = 0;
        virtual void CmdDrawIndexed() = 0;

    public:
        virtual GFXApplication* GetApplication() const = 0;

    };



}