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
    float ViewportSizeX;
    float ViewportSizeY;
    std::shared_ptr<TextureCube> IrradianceMap;
};

struct RenderMeshData
{
    std::string MeshIdentifier;
    std::vector<Primitive> Primitives;
    std::vector<DirectX::XMFLOAT4X4> InstancesTransforms;
    Material Material;
    std::shared_ptr<Buffer> InstancesDataBuffer;
};

struct RenderTargetInfo
{
    std::shared_ptr<Texture> RenderTexture;
    std::shared_ptr<Texture> DepthBuffer;
};

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) = 0;
    virtual void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<RenderMeshData>& renderMeshesData, RenderTargetInfo renderTarget) = 0;
    virtual void OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) = 0;
};
