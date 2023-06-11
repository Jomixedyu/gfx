#pragma once
#include <gfx/GFXBuffer.h>

namespace gfx
{

    GFXBuffer::GFXBuffer(GFXBufferUsage usage, size_t bufferSize) :
        m_usage(usage),  m_bufferSize(bufferSize)
    {

    }

    GFXBuffer::~GFXBuffer()
    {

    }
}