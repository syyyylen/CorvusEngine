struct VertexIn {
    float4 Position : POSITION;
    float2 Texcoord : TEXCOORD;
};

struct VertexOut {
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD;
};

VertexOut Main(VertexIn Input)
{
    VertexOut vertexOut;
    vertexOut.Position = Input.Position;
    vertexOut.Texcoord = Input.Texcoord;
    return vertexOut;
}