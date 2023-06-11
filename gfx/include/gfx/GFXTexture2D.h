#pragma once
#include <cstdint>

namespace gfx
{

    enum class GFXTextureFormat
    {
        R8,
        R8G8B8,
        R8G8B8A8,
        R8G8B8A8_SRGB,
    };

    class GFXTexture2D
    {
    public:
        virtual ~GFXTexture2D() {}
    public:
        int32_t GetWidth() const { return m_width; }
        int32_t GetHeight() const { return m_height; }
        int32_t GetChannelCount() const { return m_channel; }
        virtual const uint8_t* GetData() const = 0;
    protected:
        bool m_allowCpuRead;
        int32_t m_width;
        int32_t m_height;
        int32_t m_channel;
    };
}