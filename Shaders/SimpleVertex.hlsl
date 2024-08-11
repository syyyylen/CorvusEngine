cbuffer CBuf : register(b0)
{
    float4x4 WorldViewProj;
    float time;
};

struct VertexIn
{
    float3 Position : POSITION;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
};

VertexOut Main(VertexIn Input)
{
    VertexOut Output = (VertexOut)0;
    // Output.Position = float4(Input.Position, 1.0);
    
    Output.Position = mul(float4(Input.Position, 1.0), WorldViewProj);
    return Output;
}