struct VertexOut {
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD;
};

cbuffer CBuf : register(b0)
{
    float4x4 InvViewProj;
    float Time;
    float3 CameraPosition;
    int Mode;
    float3 DirLightDirection;
    float DirLightIntensity;
};

SamplerState Sampler : register(s1);
Texture2D Albedo : register(t2);
Texture2D Normal : register(t3);
Texture2D WorldPosition : register(t4);
Texture2D Depth : register(t5);

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

float4 Main(VertexOut Input) : SV_TARGET
{
    float4 albedo = float4(Albedo.Sample(Sampler, Input.Texcoord.xy).xyz, 1.0);
    float3 normal = normalize(Normal.Sample(Sampler, Input.Texcoord.xy).xyz);
    float3 positionWS = WorldPosition.Sample(Sampler, Input.Texcoord.xy).xyz;
    float depth = Depth.Sample(Sampler, Input.Texcoord.xy).x;

    float4 lightColor = float4(1.0, 1.0, 1.0, 1.0);
    float3 dirLightVec = normalize(DirLightDirection); // l (norm vec pointing toward light direction)

    // basic Phong lighting for directional
    float3 ambiant = 0.1f * albedo.xyz;
    float3 dirDiffuse = DoDiffuse(lightColor, dirLightVec, normal);
    
    float3 view = normalize(positionWS - CameraPosition);
    float3 dirSpecular = DoSpecular(lightColor, dirLightVec, normal, view, normal);

    float3 dirLight = (dirDiffuse + dirSpecular + ambiant) * DirLightIntensity;

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
    case 4:
        return float4(positionWS, 1.0);
    case 5: // Debug
        return albedo * float4(dirLight, 1.0);
    default:
        return albedo;
    }
    
    // safety Yellow
    // return float4(255.0f, 240.0f, 0.0f, 1.0f);
}