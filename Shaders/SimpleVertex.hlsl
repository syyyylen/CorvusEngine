cbuffer CBuf : register(b0)
{
    float4x4 ViewProj;
    float Time;
    float3 CameraPosition;
    int Mode;
};

cbuffer ObjectCbuf : register(b1)
{
    column_major float4x4 World;
    bool HasAlbedo;
    bool HasNormalMap;
    bool IsInstanced;
    bool Padding;
};

// struct InstanceData
// {
//     column_major float4x4 WorldMat;
// };
//
// StructuredBuffer<InstanceData> InstancesData : register(t5);

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
    float3 PositionWS : TEXCOORD0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD1;
    float time : TEXCOORD2;
    bool HasAlbedo : TEXCOORD3;
    bool HasNormalMap : TEXCOORD4;
    float3 CameraPosition : TEXCOORD5;
    int Mode : TEXCOORD6;
    row_major float3x3 tbn : TEXCOORD7;
};

VertexOut Main(VertexIn Input, uint InstanceID : SV_InstanceID)
{
    VertexOut Output;
    // float4x4 WorldMat = IsInstanced ? InstancesData[InstanceID].WorldMat : World;
    Output.PositionWS = mul(float4(Input.position, 1.0), World).xyz;
    Output.Position = mul(float4(Output.PositionWS, 1.0), ViewProj);
    Output.normal = normalize(mul(Input.normal, (float3x3)World));
    Output.uv = Input.texcoord;
    
    Output.tbn[0] = normalize(mul(Input.tangent, (float3x3)World));
    Output.tbn[1] = normalize(mul(Input.binormal, (float3x3)World));
    Output.tbn[2] = normalize(mul(Input.normal, (float3x3)World));

    Output.time = Time;
    Output.HasAlbedo = HasAlbedo;
    Output.HasNormalMap = HasNormalMap;
    Output.CameraPosition = CameraPosition;
    Output.Mode = Mode;

    return Output;
}