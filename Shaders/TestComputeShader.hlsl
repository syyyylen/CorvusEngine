RWTexture2DArray<float4> OutTex : register(u0);

[numthreads(32, 32, 1)]
void Main(uint3 ThreadID : SV_DispatchThreadID)
{
    OutTex[ThreadID] = float4(0, 0, 0.2, 1);
}