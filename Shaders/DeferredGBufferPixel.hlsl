SamplerState Sampler : register(s2);
Texture2D Albedo : register(t3);
Texture2D Normal : register(t4);

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

struct PixelOut
{
    float3 Albedo : SV_TARGET0;
    float4 Normal : SV_TARGET1;
};

PixelOut Main(PixelIn Input)
{
    PixelOut pixelOut;
    
    float4 albedo = float4(1.0, 1.0, 1.0, 1.0);
    float3 normal = Input.normal;

    if(Input.HasAlbedo)
        albedo = Albedo.Sample(Sampler, Input.uv);

    if(Input.HasNormalMap)
    {
        float4 normalSampled = Normal.Sample(Sampler, Input.uv);
        normalSampled.xyz = (normalSampled.xyz * 2.0) - 1.0;
        normalSampled.xyz = mul(normalSampled.xyz, Input.tbn);
        normal = normalSampled.xyz;
    }
    
    normal = normalize(normal);

    pixelOut.Albedo = albedo.xyz;
    pixelOut.Normal = float4(normal, 1.0);

    return pixelOut;
}