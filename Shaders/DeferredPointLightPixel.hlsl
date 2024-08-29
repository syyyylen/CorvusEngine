Texture2D Albedo : register(t2);
Texture2D Normal : register(t3);
Texture2D WorldPosition : register(t4);
Texture2D Depth : register(t5);

cbuffer PointLightCbuf : register(b6)
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

float3 DoDiffuse(float4 lightColor, float3 lightVector, float3 normal)
{
    return max(0.0f, dot(lightVector, normal)) * lightColor.xyz;
}

float3 DoSpecular(float4 lightColor, float3 lightVector, float3 normal, float3 view)
{
    float3 reflectedLight = normalize(reflect(-lightVector, normal));
    float amountSpecularLight = 0;
    if(dot(lightVector, normal) > 0)
        amountSpecularLight = pow(max(0.0f, dot(reflectedLight, view)), 10.0f /* Shininess */);

    return amountSpecularLight * lightColor.xyz;
}

float DoAttenuation(float distance)
{
    return 1.0f / (ConstantAttenuation + LinearAttenuation * distance + QuadraticAttenuation * distance * distance);
}

float4 Main(PixelIn Input) : SV_TARGET
{
    float4 albedo = float4(Albedo.Load(int3(Input.Position.xy, 0)).xyz, 1.0);
    float3 normal = normalize(Normal.Load(int3(Input.Position.xy, 0)).xyz);
    float3 positionWS = WorldPosition.Load(int3(Input.Position.xy, 0)).xyz;
    float depth = Depth.Load(int3(Input.Position.xy, 0)).x;

    float3 view = normalize(positionWS - Input.CameraPosition);

    float l = length(positionWS - Input.ObjectPosition);
    if(l > Radius)
        return float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 lightVector = normalize(l);
    float distance = length(l);
        
    float attenuation = DoAttenuation(distance);
    float3 diffuse = DoDiffuse(Color, lightVector, normal) * attenuation;
    float3 specular = DoSpecular(Color, lightVector, normal, view) * attenuation;

    float3 finalLight = diffuse + specular;

    switch (Input.Mode)
    {
    case 0:
        return albedo * float4(finalLight, 1.0);
    case 1:
        return albedo;
    case 2:
        return float4(normal, 1.0);
    case 3:
        return float4(depth, 0.0, 0.0, 1.0);
    case 4 :
        return float4(positionWS, 1.0f);
    case 5:
        return float4(255.0f, 240.0f, 0.0f, 1.0f);
    default:
        return float4(0.5, 0.5, 0.5, 1.0f);
    }
}