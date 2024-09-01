#include "Shaders/PBR.hlsl"

Texture2D Albedo : register(t2);
Texture2D Normal : register(t3);
Texture2D WorldPosition : register(t4);

struct PointLight
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

StructuredBuffer<PointLight> InstancesData : register(t5, space2);

struct PixelIn
{
    float4 Position : SV_POSITION;
    float3 CameraPosition : TEXCOORD0;
    float3 ObjectPosition : TEXCOORD1;
    int Mode : TEXCOORD2;
    int InstanceIdx : TEXCOORD3;
};

float DoAttenuation(PointLight light, float distance)
{
    return 1.0f / (light.ConstantAttenuation + light.LinearAttenuation * distance + light.QuadraticAttenuation * distance * distance);
}

float4 Main(PixelIn Input) : SV_TARGET
{
    PointLight lightInfo = InstancesData[Input.InstanceIdx];
    float4 albedo = float4(Albedo.Load(int3(Input.Position.xy, 0)).xyz, 1.0);
    float3 normal = normalize(Normal.Load(int3(Input.Position.xy, 0)).xyz);
    float3 positionWS = WorldPosition.Load(int3(Input.Position.xy, 0)).xyz;

    float3 view = normalize(Input.CameraPosition - positionWS);

    float3 l = Input.ObjectPosition - positionWS;
    float d = length(l);
    if(d > lightInfo.Radius)
        return float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 lightVector = normalize(l);
    float attenuation = DoAttenuation(lightInfo, d);
    float3 radiance = lightInfo.Color.xyz * attenuation;

    float3 finalLight = PBR(Fdielectric, normal, view, lightVector, normalize(lightVector + view), radiance, albedo.xyz, Roughness, Metallic); // TODO compute F0

    return float4(finalLight, 1.0);
}