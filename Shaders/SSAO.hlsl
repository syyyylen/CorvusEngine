RWTexture2D<float> OutSSAOTexture : register(u0);

cbuffer CBuf : register(b1)
{
    float Value;
}

[numthreads(32, 32, 1)]
void Main(uint3 ThreadID : SV_DispatchThreadID)
{
    OutSSAOTexture[ThreadID.xy] = Value;
}