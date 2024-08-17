cbuffer CBuf : register(b0)
{
    float4x4 ViewProj;
    float time;
    float3 padding;
};

cbuffer ObjectCbuf : register(b1)
{
    float4x4 World;
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
    float2 uv : TEXCOORD;
    row_major float3x3 tbn : TEXCOORD2;
};

VertexOut Main(VertexIn Input)
{
    VertexOut Output;
    float4 pos = mul(float4(Input.position, 1.0), World);
    Output.Position = mul(pos, ViewProj);
    Output.normal = normalize(mul(Input.normal, (float3x3)World));
    Output.uv = Input.texcoord;
    
    // Output.tbn[0] = normalize(mul(Input.tangent, World));
    // Output.tbn[1] = normalize(mul(Input.binormal, World));
    // Output.tbn[2] = normalize(mul(Input.normal, World));
    
    return Output;
}