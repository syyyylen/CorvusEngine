cbuffer CBuf : register(b0)
{
    float4x4 ViewProj;
    float Time;
    float3 CameraPosition;
    int Mode;
};

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
    float3 CameraPosition : TEXCOORD0;
    float3 ObjectPosition : TEXCOORD1;
    int Mode : TEXCOORD2;
    int InstanceIdx : TEXCOORD3;
};

VertexOut Main(VertexIn Input, uint InstanceID : SV_InstanceID)
{
    VertexOut Output;
    float3 positionWS = mul(float4(Input.position, 1.0), InstancesData[InstanceID].WorldMat).xyz;
    Output.Position = mul(float4(positionWS, 1.0), ViewProj);
    Output.CameraPosition = CameraPosition;
    Output.ObjectPosition = float3(InstancesData[InstanceID].WorldMat[3].xyz);
    Output.Mode = Mode;
    Output.InstanceIdx = InstanceID;

    return Output;
}