#pragma once
#include <memory>
#include "GFXShaderModule.h"
#include "GFXVertexLayoutDescription.h"
#include "GFXRenderPass.h"

namespace gfx
{
    enum class GFXCullMode
    {
        None = 0,
        Front = 1,
        Back = 1 << 1,
        FrontAndBack = Front | Back,
    };
    enum class GFXCompareMode
    {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };
    struct GFXGraphicsPipelineConfig
    {
    public:
        GFXCullMode CullMode;
        bool DepthTestEnable;
        bool DepthWriteEnable;
        GFXCompareMode DepthCompareOp;
    };

    class GFXGraphicsPipeline
    {
    public:
        virtual ~GFXGraphicsPipeline() {}
    };
}