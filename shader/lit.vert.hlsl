#include "Common.hlsl"



cbuffer MatrixBufferObject : register(b0, space0) { MatrixBuffer MatrixBufferObject; }

PixelInAssembly main(VertexInAssembly a2v)
{
    PixelInAssembly v2f = (PixelInAssembly)0;
    v2f.Color = a2v.Color;
    v2f.Position = mul(MatrixBufferObject.ProjectionMatrix, mul(MatrixBufferObject.ViewMatrix, mul(MatrixBufferObject.ModelMatrix, float4(a2v.Position.xyz, 1.0))));
    return v2f;
}
