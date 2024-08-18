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
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float time : TEXCOORD1;
    bool HasAlbedo : TEXCOORD2;
    bool HasNormalMap : TEXCOORD3;
    float3 CameraPosition : TEXCOORD4;
    int Mode : TEXCOORD5;
    row_major float3x3 tbn : TEXCOORD6;
};

VertexOut Main(VertexIn Input)
{
    VertexOut Output;
    float4 pos = mul(float4(Input.position, 1.0), World);
    Output.Position = mul(pos, ViewProj);
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