RWTexture2D<float3> OutTex : register(u0);

[numthreads(8, 8, 1)]
void Main(uint2 ThreadID : SV_DispatchThreadID)
{
    float outputWidth, outputHeight;
    OutTex.GetDimensions(outputWidth, outputHeight);

    float2 uv = float2(ThreadID.x / outputWidth, ThreadID.y / outputHeight);

    OutTex[ThreadID] = float3(uv.x, uv.y, 0);
}