#pragma once

namespace gfx
{
    enum class GFXBufferUsage
    {
        Vertex,
        Index,
    };

    class GFXBuffer
    {
    public:
        GFXBuffer();
        GFXBuffer(const GFXBuffer&) = delete;
        GFXBuffer(GFXBuffer&&) = delete;
        virtual ~GFXBuffer();
    public:
        virtual void Fill(GFXBufferUsage usage, const void* data, size_t bufferSize) = 0;
        virtual void Release() = 0;
    public:
        virtual size_t GetSize() const = 0;
        virtual bool IsValid() const = 0;
    };

}