#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>

namespace gfx
{
    GFXCommandBufferScope::GFXCommandBufferScope(GFXApplication* app)
        : m_app(app), m_cmdbuffer(nullptr)
    {
        m_cmdbuffer = m_app->CreateCommandBuffer();
        m_cmdbuffer->Begin();
    }
    GFXCommandBufferScope::~GFXCommandBufferScope()
    {
        m_cmdbuffer->End();
    }
}