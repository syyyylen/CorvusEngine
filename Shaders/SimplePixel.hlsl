cbuffer CBuf : register(b0)
{
    float4 color;
    float time;
    float3 padding;
};

struct PixelIn
{
    float4 Position : SV_POSITION;
};

float4 Main(PixelIn Input) : SV_TARGET
{
    float f = (cos(time) + 1) / 2 * 0.5f;
    return lerp(float4(0.2, 0.0, 0.6, 1.0), color, f);
    
    // safety Yellow
    // return float4(255.0f, 240.0f, 0.0f, 1.0f);
}