struct VertexOut
{
    float4 Position : SV_POSITION;
    float3 LookUpVector : POSITION;
};

TextureCube CubeMap : register(t1);
SamplerState Sampler : register(s2);

float4 Main(VertexOut pixelIn) : SV_Target
{
    return float4(0.0f, 0.0f, 0.1f, 1.0f);
    // return CubeMap.Sample(Sampler, pixelIn.LookUpVector);
}