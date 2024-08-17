SamplerState Sampler : register(s2);
Texture2D Albedo : register(t3);
Texture2D Normal : register(t4);

struct PixelIn
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float time : TEXCOORD1;
    row_major float3x3 tbn : TEXCOORD2;
};

float4 Main(PixelIn Input) : SV_TARGET
{
    float4 albedo = Albedo.Sample(Sampler, Input.uv);
    float3 normal = Input.normal;

    float4 normalSampled = Normal.Sample(Sampler, float2(Input.uv.x, 1.0 - Input.uv.y));
    normalSampled.xyz = (normalSampled.xyz * 2.0) - 1.0;
    normalSampled.xyz = mul(normalSampled.xyz, Input.tbn);
    normal = normalSampled.xyz;

    normal = normalize(normal);
    
    float f = (cos(Input.time) + 1) / 2 * 0.5f;
    f = smoothstep(0.0f, 1.0f, f);
    float3 mix = lerp(albedo.xyz, normal, f);
    
    return float4(mix, 1.0f);
    
    // safety Yellow
    // return float4(255.0f, 240.0f, 0.0f, 1.0f);
}