#pragma once
#include "Core.h"

#define MAX_LIGHTS 1

struct PointLight
{
    DirectX::XMFLOAT3 Position;
    float Padding1 = 0.0f;
    // 16 bytes boundary 
    DirectX::XMFLOAT4 Color = { 1.0, 1.0, 1.0, 1.0 };
    // 16 bytes boundary 
    float ConstantAttenuation = 1.0f;
    float LinearAttenuation = 0.2f;
    float QuadraticAttenuation = 0.1f;
    float Padding3;
    // 16 bytes boundary
    int Enabled = true;
    float Padding4[3];
};

struct PointLightsConstantBuffer
{
    PointLight PointLights[MAX_LIGHTS];
};

struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 ViewProj; // Stores InvViewProj in Deferred PS case (retrieve pos from depth)
    float Time;
    DirectX::XMFLOAT3 CameraPosition;
    int Mode;
};

struct ObjectConstantBuffer
{
    DirectX::XMFLOAT4X4 World;
    int HasAlbedo = false;
    int HasNormalMap = false;
    int Padding1;
    int Padding2;
};

struct ScreenQuadVertex
{
    DirectX::XMFLOAT4 Position;
    DirectX::XMFLOAT2 TexCoord;
};