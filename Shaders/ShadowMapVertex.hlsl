cbuffer Cbuf : register(b0)
{
    row_major float4x4 ViewProj;
}

struct InstanceData
{
    row_major float4x4 WorldMat;
    bool HasAlbedo;
    bool HasNormalMap;
    bool HasMetallicRoughness;
};

StructuredBuffer<InstanceData> InstancesData : register(t1, space1);

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
};

VertexOut Main(VertexIn Input, uint InstanceID : SV_InstanceID)
{
    VertexOut Output;
    float3 positionWS = mul(float4(Input.position, 1.0), InstancesData[InstanceID].WorldMat).xyz;
    Output.Position = mul(float4(positionWS, 1.0), ViewProj);
    return Output;
}