#include "Shaders/PBR.hlsl"

struct VertexOut
{
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD;
};

cbuffer CBuf : register(b0)
{
    row_major float4x4 ViewProj;
    float Time;
    float3 CameraPosition;
    int Mode;
    float3 DirLightDirection;
    float DirLightIntensity;
    float3 Padding;
    row_major float4x4 InvViewProj;
};

SamplerState Sampler : register(s1);
Texture2D Albedo : register(t2);
Texture2D Normal : register(t3);
Texture2D MetallicRoughness : register(t4);
Texture2D Depth : register(t5);

float4 Main(VertexOut Input) : SV_TARGET
{
    float4 albedo = float4(Albedo.Sample(Sampler, Input.Texcoord.xy).xyz, 1.0);
    float3 normal = normalize(Normal.Sample(Sampler, Input.Texcoord.xy).xyz);
    float3 metallicRoughness = MetallicRoughness.Sample(Sampler, Input.Texcoord.xy).xyz;
    float depth = Depth.Sample(Sampler, Input.Texcoord.xy).x;

    float4 clipSpacePosition = float4(Input.Texcoord * 2.0 - 1.0, depth, 1.0);
    clipSpacePosition.y *= -1.0;

    float4 worldSpacePosition = mul(clipSpacePosition, InvViewProj);
    worldSpacePosition /= worldSpacePosition.w;

    float4 lightColor = float4(1.0, 1.0, 1.0, 1.0) * DirLightIntensity;
    float3 dirLightVec = normalize(DirLightDirection) * -1.0f; // l (norm vec pointing toward light direction)

    // basic Phong lighting for directional
    float3 ambiant = 0.1f * albedo.xyz;
    float3 view = normalize(CameraPosition - worldSpacePosition.xyz);
    float3 F0 = lerp(Fdielectric, albedo.xyz, metallicRoughness.b);
    
    float3 finalLight = PBR(F0, normal, view, dirLightVec, normalize(dirLightVec + view), lightColor.xyz, albedo.xyz, metallicRoughness.g, metallicRoughness.b) + ambiant;

    switch (Mode)
    {
    case 0:
        return float4(finalLight, 1.0);
    case 1:
        return albedo;
    case 2:
        return float4(normal, 1.0);
    case 3:
        return float4(depth, 0.0, 0.0, 1.0);
    case 4:
        return float4(worldSpacePosition.xyz, 1.0f);
    case 5:
        return float4(metallicRoughness, 1.0);
    default:
        return float4(finalLight, 1.0);
    }
    
    // safety Yellow
    // return float4(255.0f, 240.0f, 0.0f, 1.0f);
}