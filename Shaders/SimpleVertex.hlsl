cbuffer CBuf : register(b0)
{
    float4x4 WorldViewProj;
    float time;
    float3 padding;
};

struct VertexIn
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut Main(VertexIn Input)
{
    VertexOut Output;
    Output.Position = mul(float4(Input.Position, 1.0), WorldViewProj);
    Output.Color = Input.Color;
    return Output;
}