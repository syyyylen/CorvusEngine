#pragma once
#include "Core.h"

#define MAX_LIGHTS 150

#define MAX_INSTANCES 300

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
    float Padding3 = 0.0f;
};

struct PointLightsConstantBuffer
{
    PointLight PointLights[MAX_LIGHTS];
};

struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 ViewProj;
    // 16 bytes boundary
    float Time;
    DirectX::XMFLOAT3 CameraPosition;
    // 16 bytes boundary
    int Mode;
    DirectX::XMFLOAT3 DirLightDirection;
    // 16 bytes boundary
    float DirLightIntensity;
    float ScreenDimensions[2];
    float Padding;
    // 16 bytes boundary
    DirectX::XMFLOAT4X4 InvViewProj;
};

struct ObjectConstantBuffer
{
    DirectX::XMFLOAT4X4 World;
    int HasAlbedo = false;
    int HasNormalMap = false;
    int IsInstanced = false;
    int HasMetallicRoughness = false;
};

struct ScreenQuadVertex
{
    DirectX::XMFLOAT4 Position;
    DirectX::XMFLOAT2 TexCoord;
};

struct InstanceData
{
    DirectX::XMFLOAT4X4 WorldMat;
    int HasAlbedo;
    int HasNormalMap;
    int HasMetallicRoughness;
};

struct SkyBoxConstantBuffer
{
    DirectX::XMFLOAT4X4 ViewProj;
    DirectX::XMFLOAT3 CameraPosition;
};

struct PrefilterMapFilterSettings
{
    float Roughness;
    float Padding[3];
};