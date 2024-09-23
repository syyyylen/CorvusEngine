struct VertexIn
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float3 LookUpVector : POSITION;
};

cbuffer CBuf : register(b0)
{
    row_major float4x4 ViewProj;
    float3 CameraPosition;
};

VertexOut Main(VertexIn vertexIn)
{
    VertexOut vertexOut;

    // Use local vertex position as cubemap lookup vector.
    vertexOut.LookUpVector = vertexIn.position;

    // Always center sky about camera.
    float4 posW = float4(vertexIn.position, 1.0f);
    posW.xyz += CameraPosition;

    // Set z = w so that z/w = 1 (i.e., skydome always on far plane).
    vertexOut.Position = mul(posW, ViewProj).xyww;
	
    return vertexOut;
}