cbuffer CBuf : register(b0)
{
    float4x4 WorldViewProj;
    float time;
    float3 padding;
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
    row_major float3x3 tbn : TEXCOORD2;
};

VertexOut Main(VertexIn Input)
{
    VertexOut Output;
    Output.Position = mul(float4(Input.position, 1.0), WorldViewProj);
    Output.normal = normalize(Input.normal); // TODO * World matrix
    
    // Output.tbn[0] = normalize(mul(Input.tangent, World));
    // Output.tbn[1] = normalize(mul(Input.binormal, World));
    // Output.tbn[2] = normalize(mul(Input.normal, World));
    
    Output.tbn[0] = 1.0;
    Output.tbn[1] = 1.0;
    Output.tbn[2] = 1.0;
    
    return Output;
}