Texture2D Albedo : register(t2);
Texture2D Normal : register(t3);
Texture2D WorldPosition : register(t4);
Texture2D Depth : register(t5);

struct PixelIn
{
    float4 Position : SV_POSITION;
    float3 CameraPosition : TEXCOORD0;
    float3 ObjectPosition : TEXCOORD1;
    int Mode : TEXCOORD2;
};

float4 Main(PixelIn Input) : SV_TARGET
{
    float4 albedo = float4(Albedo.Load(int3(Input.Position.xy, 0)).xyz, 1.0);
    float3 normal = normalize(Normal.Load(int3(Input.Position.xy, 0)).xyz);
    float3 positionWS = WorldPosition.Load(int3(Input.Position.xy, 0)).xyz;
    float depth = Depth.Load(int3(Input.Position.xy, 0)).x;

    float f = 1.0 - clamp(length(positionWS - Input.ObjectPosition) / 4.0, 0, 1);

    switch (Input.Mode)
    {
    case 0:
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    case 1:
        return albedo;
    case 2:
        return float4(normal, 1.0);
    case 3:
        return float4(depth, 0.0, 0.0, 1.0);
    case 4 :
        return float4(positionWS, 1.0f);
    case 5:
        return albedo * f;
    default:
        return float4(0.5, 0.5, 0.5, 1.0f);
    }
}