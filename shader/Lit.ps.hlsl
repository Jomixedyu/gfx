#include "Common.inc.hlsl"



VK_SAMPLER(1) Texture2D texture1 : register(t1);
VK_SAMPLER(1) SamplerState state : register(s1);

OutPixelAssembly main(InPixelAssembly v2f)
{
    OutPixelAssembly p2o;
    p2o.Color = texture1.Sample(state, v2f.TexCoord0);
    //p2o.Color = tex2D(inTex, v2f.TexCoord0);
    //f2p.Color = float4(v2f.TexCoord0, 0, 1);
    return p2o;
}