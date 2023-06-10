#include "Common.inc.hlsl"

[[vk::combinedImageSampler]][[vk::binding(1)]]
Texture2D texture1;
[[vk::combinedImageSampler]][[vk::binding(1)]]
SamplerState state;

OutPixelAssembly main(InPixelAssembly v2f)
{
    OutPixelAssembly p2o;
    p2o.Color = texture1.Sample(state, v2f.TexCoord0);
    //p2o.Color = tex2D(inTex, v2f.TexCoord0);
    //f2p.Color = float4(v2f.TexCoord0, 0, 1);
    return p2o;
}