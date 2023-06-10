#include "Common.inc.hlsl"



cbuffer MatrixBufferObject : register(b0)
{ 
    CommonBuffer MatrixBufferObject;
}

float4 TransformMVP(in float3 position)
{
    return mul(MatrixBufferObject.ProjectionMatrix, mul(MatrixBufferObject.ViewMatrix, mul(MatrixBufferObject.ModelMatrix, float4(position, 1.0))));
}

InPixelAssembly main(InVertexAssembly a2v)
{
    InPixelAssembly v2f = (InPixelAssembly) 0;
    v2f.WorldNormal = a2v.Normal;
    v2f.TexCoord0 = a2v.TexCoord0;
    v2f.TexCoord1 = a2v.TexCoord1;
    v2f.TexCoord2 = a2v.TexCoord2;
    v2f.TexCoord3 = a2v.TexCoord3;
    v2f.Color = a2v.Color;

    v2f.Position = TransformMVP(a2v.Position);
    return v2f;
}
