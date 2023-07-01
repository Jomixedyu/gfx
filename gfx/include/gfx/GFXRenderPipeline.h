#pragma once
#include "GFXRenderContext.h"
#include "GFXRenderTarget.h"
#include <vector>

namespace gfx
{
    class GFXRenderPipeline
    {
    public:
        virtual void OnRender(GFXRenderContext* context, const std::vector<GFXRenderTarget*>& renderTargets) = 0;
    };
}