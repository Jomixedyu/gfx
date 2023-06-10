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

    struct GFXCommandBufferScope
    {
    private:
        GFXApplication* m_app;
        std::shared_ptr<GFXCommandBuffer> m_cmdbuffer;
    public:
        GFXCommandBufferScope(GFXApplication* app);
        GFXCommandBuffer* operator->() const
        {
            return m_cmdbuffer.get();
        }
        ~GFXCommandBufferScope();

        GFXCommandBufferScope(const GFXCommandBufferScope&) = delete;
        GFXCommandBufferScope(GFXCommandBufferScope&&) = delete;
    };

}