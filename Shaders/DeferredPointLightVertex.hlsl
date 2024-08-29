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
};

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
};

VertexOut Main(VertexIn Input)
{
    VertexOut Output;
    float3 positionWS = mul(float4(Input.position, 1.0), World).xyz;
    Output.Position = mul(float4(positionWS, 1.0), ViewProj);
    Output.CameraPosition = CameraPosition;
    Output.ObjectPosition = float3(World[3].xyz);
    Output.Mode = Mode;

    return Output;
}