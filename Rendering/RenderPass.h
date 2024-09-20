#pragma once
#include "Camera.h"
#include "RenderingLayouts.h"
#include "RenderItem.h"

struct DirectionalLightInfo
{
    DirectX::XMFLOAT3 Direction = { 1.0, 1.0, 0.0 };
    float Intensity = 1.0f;
};

struct GlobalPassData
{
    float DeltaTime;
    float ElapsedTime;
    int ViewMode;
    std::vector<PointLight> PointLights;
    DirectionalLightInfo DirectionalInfo;
    float viewportSizeX;
    float viewportSizeY;
};

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) = 0;
    virtual void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<std::shared_ptr<RenderItem>>& renderItems) = 0;
    virtual void OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) = 0;
    virtual std::shared_ptr<Texture> GetRenderTexture() = 0;
};
