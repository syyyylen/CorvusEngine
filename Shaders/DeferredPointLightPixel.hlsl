SamplerState Sampler : register(s5);
Texture2D Albedo : register(t2);
Texture2D Normal : register(t3);
Texture2D Depth : register(t4);

struct PixelIn
{
    float4 Position : SV_POSITION;
    float3 PositionWS : TEXCOORD0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD1;
    float time : TEXCOORD2;
    bool HasAlbedo : TEXCOORD3;
    bool HasNormalMap : TEXCOORD4;
    float3 CameraPosition : TEXCOORD5;
    int Mode : TEXCOORD6;
    row_major float3x3 tbn : TEXCOORD7;
};

float4 Main(PixelIn Input) : SV_TARGET
{
    float4 albedo = float4(Albedo.Sample(Sampler, Input.Position.xy).xyz, 1.0);
    float3 normal = normalize(Normal.Sample(Sampler, Input.Position.xy).xyz);
    float depth = Depth.Sample(Sampler, Input.Position.xy).x;

    switch (Input.Mode)
    {
    case 0:
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
        // return float4(255.0f, 240.0f, 0.0f, 1.0f);
    case 1:
        return albedo;
    case 2:
        return float4(normal, 1.0);
    case 3:
        return float4(depth, 0.0, 0.0, 1.0);
    default:
        return float4(0.5, 0.5, 0.5, 1.0f);
    }
}