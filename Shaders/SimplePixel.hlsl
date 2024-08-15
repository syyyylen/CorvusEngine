struct PixelIn
{
    float4 Position : SV_POSITION;
};

float4 Main(PixelIn Input) : SV_TARGET
{
    // safety Yellow
    return float4(255.0f, 240.0f, 0.0f, 1.0f);
}