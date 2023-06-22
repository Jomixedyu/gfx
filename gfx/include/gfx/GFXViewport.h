#pragma once
#include "GFXRenderTarget.h"

namespace gfx
{
    class GFXViewport
    {
    public:
        virtual GFXRenderTarget* GetRenderTarget() = 0;
    public:
        virtual ~GFXViewport() {}
    };
}