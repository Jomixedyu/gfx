struct VertexInAssembly
{
    [[vk::location(0)]] float3 Position : POSITION0;
    [[vk::location(1)]] float3 Normal : NORMAL;
    [[vk::location(2)]] float3 Tangent : TANGENT;
    [[vk::location(3)]] float4 Color : COLOR0;
    [[vk::location(4)]] float2 TexCoord0 : TEXCOORD0;
    [[vk::location(5)]] float2 TexCoord1 : TEXCOORD1;
    [[vk::location(6)]] float2 TexCoord2 : TEXCOORD2;
    [[vk::location(7)]] float2 TexCoord3 : TEXCOORD3;
};

struct PixelInAssembly
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float4 Color : COLOR0;
};

struct PixelOutAssembly
{
    float4 Color : SV_TARGET;
};

struct MatrixBuffer
{
	float4x4 ModelMatrix;
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
};