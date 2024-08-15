struct PixelIn
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    row_major float3x3 tbn : TEXCOORD2;
};

float4 Main(PixelIn Input) : SV_TARGET
{
    // safety Yellow
    return float4(Input.normal, 1.0f);
    return float4(255.0f, 240.0f, 0.0f, 1.0f);
}