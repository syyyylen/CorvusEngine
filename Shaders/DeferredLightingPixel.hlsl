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

// #define MAX_LIGHTS 1

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

/*
cbuffer LightsCbuf : register(b5)
{
    PointLight PointLights[MAX_LIGHTS];
};
*/

float3 DoDiffuse(float4 lightColor, float3 lightVector, float3 normal)
{
    return max(0.0f, dot(lightVector, normal)) * lightColor.xyz;
}

float3 DoSpecular(float4 lightColor, float3 lightVector, float3 normal, float3 view, float3 inputNormal)
{
    float3 reflectedLight = normalize(reflect(-lightVector, normal));
    float amountSpecularLight = 0;
    if(dot(lightVector, inputNormal) > 0) // we use vertex normal instead of normal one
        amountSpecularLight = pow(max(0.0f, dot(reflectedLight, view)), 10.0f /* Shininess */);

    return amountSpecularLight * lightColor.xyz;
}

float DoAttenuation(PointLight light, float distance)
{
    return 1.0f / (light.ConstantAttenuation + light.LinearAttenuation * distance + light.QuadraticAttenuation * distance * distance);
}

float4 Main(VertexOut Input) : SV_TARGET
{
    float4 albedo = float4(Albedo.Sample(Sampler, Input.Texcoord.xy).xyz, 1.0);
    float3 normal = normalize(Normal.Sample(Sampler, Input.Texcoord.xy).xyz);
    float depth = Depth.Sample(Sampler, Input.Texcoord.xy).x;

    float4 lightColor = float4(1.0, 1.0, 1.0, 1.0);
    float3 dirLightVec = normalize(float3(1.0, 1.0, 0.0)); // l (norm vec pointing toward light direction)

    // basic Phong lighting for directional
    float3 ambiant = 0.1f * albedo.xyz;
    float3 dirDiffuse = DoDiffuse(lightColor, dirLightVec, normal);
    
    float3 view = normalize(Input.Position.xyz - CameraPosition);
    float3 dirSpecular = DoSpecular(lightColor, dirLightVec, normal, view, normal);

    float3 dirLight = dirDiffuse + dirSpecular + ambiant;

    switch (Mode)
    {
    case 0:
        return albedo * float4(dirLight, 1.0);
    case 1:
        return albedo;
    case 2:
        return float4(normal, 1.0);
    case 3:
        return float4(depth, 0.0, 0.0, 1.0);
    default:
        return albedo;
    }
    
    // safety Yellow
    // return float4(255.0f, 240.0f, 0.0f, 1.0f);
}