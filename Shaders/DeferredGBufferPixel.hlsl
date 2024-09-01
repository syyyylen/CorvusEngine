cbuffer ObjectCbuf : register(b1)
{
    column_major float4x4 World;
    bool HasAlbedo;
    bool HasNormalMap;
    bool IsInstanced;
    bool Padding;
};

SamplerState Sampler : register(s2);
Texture2D Albedo : register(t3);
Texture2D Normal : register(t4);

struct PixelIn
{
    float4 Position : SV_POSITION;
    float3 PositionWS : TEXCOORD0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD1;
    row_major float3x3 tbn : TEXCOORD2;
};

struct PixelOut
{
    float3 Albedo : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float3 PositionWS : SV_TARGET2;
};

PixelOut Main(PixelIn Input)
{
    PixelOut pixelOut;
    
    float4 albedo = float4(1.0, 1.0, 1.0, 1.0);
    float3 normal = Input.normal;

    if(HasAlbedo)
        albedo = Albedo.Sample(Sampler, Input.uv);

    if(HasNormalMap)
    {
        float4 normalSampled = Normal.Sample(Sampler, Input.uv);
        normalSampled.xyz = (normalSampled.xyz * 2.0) - 1.0;
        normalSampled.xyz = mul(normalSampled.xyz, Input.tbn);
        normal = normalSampled.xyz;
    }
    
    normal = normalize(normal);

    pixelOut.Albedo = albedo.xyz;
    pixelOut.Normal = float4(normal, 1.0);
    pixelOut.PositionWS = Input.PositionWS;

    return pixelOut;
}