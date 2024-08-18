SamplerState Sampler : register(s2);
Texture2D Albedo : register(t3);
Texture2D Normal : register(t4);

struct PixelIn
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

float4 Main(PixelIn Input) : SV_TARGET
{
    float3 lightColor = float3(1.0, 1.0, 1.0)  * 1.0f /* Directional Intensity */;
    float3 dirLight = normalize(float3(1.0, 1.0, 0.0)); // l (norm vec pointing toward light direction)
    float4 albedo = float4(1.0, 1.0, 1.0, 1.0);
    float3 normal = Input.normal;

    if(Input.HasAlbedo)
        albedo = Albedo.Sample(Sampler, Input.uv);

    if(Input.HasNormalMap)
    {
        float4 normalSampled = Normal.Sample(Sampler, Input.uv);
        normalSampled.xyz = (normalSampled.xyz * 2.0) - 1.0;
        normalSampled.xyz = mul(normalSampled.xyz, Input.tbn);
        normal = normalSampled.xyz;
    }
    
    normal = normalize(normal);

    // basic Phong lighting
    float3 ambiant = 0.1f * albedo.xyz;
    float3 diffuse = max(0.0f, dot(dirLight, normal)) * lightColor;
    
    float3 reflectedLight = normalize(reflect(dirLight, normal));
    float3 view = normalize(Input.Position.xyz - Input.CameraPosition);
    float amountSpecularLight = 0;
    if(dot(dirLight, Input.normal) > 0) // we use vertex normal instead of normal map one
        amountSpecularLight = pow(max(0.0f, dot(reflectedLight, view)), 10.0f /* Shininess */);
    float3 specular = amountSpecularLight * lightColor * 1.0f;

    float3 finalLight = diffuse + specular + ambiant;

    switch (Input.Mode)
    {
    case 0:
        return albedo * float4(finalLight, 1.0);
    case 1:
        return albedo * float4(diffuse, 1.0);
    case 2:
        return albedo * float4(specular, 1.0);
    case 3:
        return albedo;
    case 4:
        return float4(normal, 1.0f);
    default:
        return albedo * float4(finalLight, 1.0);
    }
    
    // safety Yellow
    // return float4(255.0f, 240.0f, 0.0f, 1.0f);
}