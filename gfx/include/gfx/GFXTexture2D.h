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



    enum class GFXSamplerFilter
    {
        Nearest,
        Linear,
        Cubic
    };
    enum class GFXSamplerAddressMode
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
    };

    struct GFXSamplerConfig
    {
        GFXSamplerFilter Filter;
        GFXSamplerAddressMode AddressMode;
    };

    class GFXTexture2D
    {
    protected:
        GFXTexture2D(int32_t width, int32_t height, int32_t channel, GFXSamplerConfig cfg, bool enableReadwrite)
            : m_width(width), m_height(height), m_channel(channel), m_samplerConfig(cfg), m_enableReadWrite(enableReadwrite)
        {

        }
    public:
        virtual ~GFXTexture2D() {}
    public:
        int32_t GetWidth() const { return m_width; }
        int32_t GetHeight() const { return m_height; }
        int32_t GetChannelCount() const { return m_channel; }
        virtual const uint8_t* GetData() const = 0;
    protected:
        bool m_enableReadWrite;
        GFXSamplerConfig m_samplerConfig;
        int32_t m_width;
        int32_t m_height;
        int32_t m_channel;
    };
}