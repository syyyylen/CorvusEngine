struct VertexOut {
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD;
};

cbuffer CBuf : register(b0)
{
    float4x4 ViewProj;
    float Time;
    float3 CameraPosition;
    int Mode;
};

SamplerState Sampler : register(s1);
Texture2D Albedo : register(t2);
Texture2D Normal : register(t3);
Texture2D Depth : register(t4);

/*
#define MAX_LIGHTS 1

struct PointLight
{
    float3 Position;
    float Padding1;
    // 16 bytes boundary 
    float4 Color;
    // 16 bytes boundary 
    float ConstantAttenuation;
    float LinearAttenuation;
    float QuadraticAttenuation;
    float Padding3;
    // 16 bytes boundary
    bool Enabled;
    float3 Padding4;
    // 16 bytes boundary
}; // size 48 bytes (16 * 3)

cbuffer LightsCbuf : register(b5)
{
    PointLight PointLights[MAX_LIGHTS];
};
*/

float4 Main(VertexOut Input) : SV_TARGET
{
    return float4(255.0f, 240.0f, 0.0f, 1.0f);
    
    float4 albedo = float4(Albedo.Sample(Sampler, Input.Position.xy).xyz, 1.0);
    float3 normal = normalize(Normal.Sample(Sampler, Input.Position.xy).xyz);
    float depth = Depth.Sample(Sampler, Input.Position.xy).x;
    
    switch (Mode)
    {
    case 0:
        return albedo;
    case 1:
        return float4(normal, 1.0);
    case 2:
        return float4(depth, 0.0, 0.0, 1.0);
    default:
        return albedo;
    }
    
    // safety Yellow
    // return float4(255.0f, 240.0f, 0.0f, 1.0f);
}