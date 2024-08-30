static const float PI = 3.1415926;
static const float Epsilon = 0.00001;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;

cbuffer PBRDebugSettings : register(b6)
{
    float Roughness;
    float Metallic;
};

float DistributionGGX(float3 N, float3 H, float a)
{
    a = a * a;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    return nom / max(denom, 0.00001f);
}

float GeometryShlickGGX(float a, float3 N, float3 X)
{
    float nom = max(dot(N, X), 0.0f);
    
    float r = (a + 1.0);
    float k = (r*r) / 8.0;
    
    float denom = max(dot(N, X), 0.0f) * (1.0f - k) + k;
    denom = max(denom, 0.00001f);

    return nom / denom;
}

float GeometrySmith(float a, float3 N, float3 V, float3 L)
{
    return GeometryShlickGGX(a, N, V) * GeometryShlickGGX(a, N, L);
}

float3 FresnelShlick(float3 F0, float3 V, float3 H)
{
    return F0 + (1.0f - F0) * pow(1.0f - max(dot(V, H), 0.0f), 5.0f);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float r)   // cosTheta is n.v
{
    return F0 + (max(float3(1.0f - r, 1.0f - r, 1.0f - r), F0) - F0) * pow(1.0 - cosTheta, 5.0f);
}

float3 PBR(float3 F0, float3 N, float3 V, float3 L, float3 H, float3 radiance, float3 albedo, float roughness, float metallic)
{
    float3 Ks = FresnelShlick(F0, V, H);
    float3 Kd = float3(1.0f, 1.0f, 1.0f) - Ks;
    Kd *= 1.0 - metallic;
    
    float3 lambert = albedo / PI;
    
    float3 cookTorranceNumerator = DistributionGGX(N, H, roughness) * GeometrySmith(roughness, N, V, L) * FresnelShlick(F0, V, H);
    float cookTorranceDenominator = 4.0f * max(dot(V, N), 0.0f) * max(dot(L, N), 0.0f);
    cookTorranceDenominator = max(cookTorranceDenominator, 0.00001f);
    float3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;
    
    float3 BRDF = Kd * lambert + cookTorrance;
    float3 outgoingLight = BRDF * radiance * max(dot(L, N), 0.0f);
    
    return outgoingLight;
}

float3 DoDiffuse(float4 lightColor, float3 lightVector, float3 normal)
{
    return max(0.0f, dot(lightVector, normal)) * lightColor.xyz;
}

float3 DoSpecular(float4 lightColor, float3 lightVector, float3 normal, float3 view)
{
    float3 reflectedLight = normalize(reflect(-lightVector, normal));
    float amountSpecularLight = 0;
    if(dot(lightVector, normal) > 0)
        amountSpecularLight = pow(max(0.0f, dot(reflectedLight, view)), 10.0f /* Shininess */);

    return amountSpecularLight * lightColor.xyz;
}