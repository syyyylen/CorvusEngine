#include "Shaders/PBR.hlsl"

Texture2D Albedo : register(t2);
Texture2D Normal : register(t3);
Texture2D WorldPosition : register(t4);
Texture2D Depth : register(t5);

cbuffer PointLightCbuf : register(b7)
{
    float3 Position;
    float Radius;
    // 16 bytes boundary 
    float4 Color;
    // 16 bytes boundary 
    float ConstantAttenuation;
    float LinearAttenuation;
    float QuadraticAttenuation;
    float Padding3;
};

struct PixelIn
{
    float4 Position : SV_POSITION;
    float3 CameraPosition : TEXCOORD0;
    float3 ObjectPosition : TEXCOORD1;
    int Mode : TEXCOORD2;
};

float DoAttenuation(float distance)
{
    return 1.0f / (ConstantAttenuation + LinearAttenuation * distance + QuadraticAttenuation * distance * distance);
}

float4 Main(PixelIn Input) : SV_TARGET
{
    float4 albedo = float4(Albedo.Load(int3(Input.Position.xy, 0)).xyz, 1.0);
    float3 normal = normalize(Normal.Load(int3(Input.Position.xy, 0)).xyz);
    float3 positionWS = WorldPosition.Load(int3(Input.Position.xy, 0)).xyz;
    float depth = Depth.Load(int3(Input.Position.xy, 0)).x; // TODO reconstruct pos from depth

    float3 view = normalize(Input.CameraPosition - positionWS);

    float3 l = Input.ObjectPosition - positionWS;
    float d = length(l);
    if(d > Radius)
        return float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 lightVector = normalize(l);
    float attenuation = DoAttenuation(d);
    float3 radiance = Color.xyz * attenuation;

    float3 finalLight = PBR(Fdielectric, normal, view, lightVector, normalize(lightVector + view), radiance, albedo.xyz, Roughness, Metallic); // TODO compute F0

    if(Input.Mode == 10) // if depth unused, remove at compil, fck up bind slots
        return depth;

    return float4(finalLight, 1.0);
}