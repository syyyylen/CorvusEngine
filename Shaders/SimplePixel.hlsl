struct PixelIn
{
    float4 Position : SV_POSITION;
};

float4 Main(PixelIn Input) : SV_TARGET
{
    return float4(0.2, 0.0, 0.6, 1.0);
}