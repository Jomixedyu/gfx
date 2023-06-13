#pragma once
#include "GFXApplication.h"

namespace gfx
{
    struct GFXCommandBufferScope
    {
    private:
        GFXApplication* m_app;
        std::shared_ptr<GFXCommandBuffer> m_cmdbuffer;
    public:
        GFXCommandBufferScope(GFXApplication* app)
            : m_app(app), m_cmdbuffer(nullptr)
        {
            m_cmdbuffer = m_app->CreateCommandBuffer();
            m_cmdbuffer->Begin();
        }
        GFXCommandBuffer* operator->() const
        {
            return m_cmdbuffer.get();
        }
        ~GFXCommandBufferScope()
        {
            m_cmdbuffer->End();
        }

        GFXCommandBufferScope(const GFXCommandBufferScope&) = delete;
        GFXCommandBufferScope(GFXCommandBufferScope&&) = delete;
    };
}