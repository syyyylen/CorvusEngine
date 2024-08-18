SamplerState Sampler : register(s2);
Texture2D Albedo : register(t3);
Texture2D Normal : register(t4);

struct PixelIn
{
    float4 Position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float time : TEXCOORD1;
    bool HasAlbedo : TEXCOORD2;
    bool HasNormalMap : TEXCOORD3;
    row_major float3x3 tbn : TEXCOORD4;
};

float4 Main(PixelIn Input) : SV_TARGET
{
    float4 albedo = float4(1.0, 1.0, 1.0, 1.0);
    float3 normal = Input.normal;

    if(Input.HasAlbedo)
        albedo = Albedo.Sample(Sampler, Input.uv);

    if(Input.HasNormalMap)
    {
        float4 normalSampled = Normal.Sample(Sampler, float2(Input.uv.x, 1.0 - Input.uv.y));
        normalSampled.xyz = (normalSampled.xyz * 2.0) - 1.0;
        normalSampled.xyz = mul(normalSampled.xyz, Input.tbn);
        normal = normalSampled.xyz;
    }
    
    normal = normalize(normal);
    
    float f = (cos(Input.time) + 1) / 2 * 0.5f;
    f = smoothstep(0.0f, 1.0f, f);
    float3 mix = lerp(albedo.xyz, normal, f);

    if(Input.HasNormalMap)
        return float4(mix, 1.0f);

    return albedo;
    
    // safety Yellow
    // return float4(255.0f, 240.0f, 0.0f, 1.0f);
}