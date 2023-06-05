
float2 positions[3] = float2[](
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
    );

float3 colors[3] = float3[](
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
    );

struct VertexIn
{
    uint vertId : SV_VertexID;
};

struct PixelIn
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PixelIn MainVS(VertexIn v)
{
    return positions[v.vertId];
}


float4 MainPS(in PixelIn p) : SV_TARGET
{
    return p.color;
}

technique TShader
{
    pass P0
    {
        VertexShader = compile vs_1_1 MainVS();
        PixelShader = compile ps_1_1 MainPS();
    }
}