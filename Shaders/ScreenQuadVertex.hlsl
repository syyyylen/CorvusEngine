struct VertexOut
{
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD;
};

VertexOut Main(uint vid : SV_VertexID)
{
    VertexOut vertexOut;
    vertexOut.Texcoord = float2((vid << 1) & 2, vid & 2);
    vertexOut.Position = float4(vertexOut.Texcoord * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    return vertexOut;
}