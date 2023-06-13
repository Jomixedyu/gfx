#pragma once
#include <memory>

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

    public:
        virtual GFXApplication* GetApplication() const = 0;

    };



}