#pragma once
#include "Camera.h"
#include "RenderingLayouts.h"
#include "RenderItem.h"

struct GlobalPassData
{
    float DeltaTime;
    float ElapsedTime;
    int ViewMode;
    std::vector<PointLight> PointLights;
};

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) = 0;
    virtual void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<std::shared_ptr<RenderItem>>& renderItems) = 0;
    virtual void OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) = 0;
};
