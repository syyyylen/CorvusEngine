#pragma once
#include "Core.h"

#define MAX_LIGHTS 100

struct PointLight
{
    DirectX::XMFLOAT3 Position;
    float Radius = 20.0f;
    // 16 bytes boundary 
    DirectX::XMFLOAT4 Color = { 1.0, 1.0, 1.0, 1.0 };
    // 16 bytes boundary 
    float ConstantAttenuation = 1.0f;
    float LinearAttenuation = 0.2f;
    float QuadraticAttenuation = 0.1f;
    float Padding3;
};

struct PointLightsConstantBuffer
{
    PointLight PointLights[MAX_LIGHTS];
};

struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 ViewProj; // Stores InvViewProj in Deferred PS case (retrieve pos from depth)
    // 16 bytes boundary
    float Time;
    DirectX::XMFLOAT3 CameraPosition;
    // 16 bytes boundary
    int Mode;
    DirectX::XMFLOAT3 DirLightDirection;
    // 16 bytes boundary
    float DirLightIntensity;
};

struct ObjectConstantBuffer
{
    DirectX::XMFLOAT4X4 World;
    int HasAlbedo = false;
    int HasNormalMap = false;
    int IsInstanced = false;
    int Padding2;
};

struct ScreenQuadVertex
{
    DirectX::XMFLOAT4 Position;
    DirectX::XMFLOAT2 TexCoord;
};

struct PBRDebugSettings // TODO remove this when PBR done
{
    float Roughness = 0.2f;
    float Metallic = 0.0f;
    float Padding[2];
}; // 16 bytes

struct InstanceData
{
    DirectX::XMFLOAT4X4 WorldMat;
};