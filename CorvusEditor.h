﻿#pragma once
#include "Camera.h"
#include "Core.h"
#include "InputListener.h"
#include "ResourcesManager.h"
#include "Rendering/RenderItem.h"
#include "Window.h"
#include "ECS/Scene.h"
#include "ImGui/ImGuizmo.h"
#include "RHI/D3D12Renderer.h"
#include "Rendering/RenderPass.h"

class CorvusEditor : public InputListener
{
public:
    CorvusEditor();
    ~CorvusEditor();

    std::shared_ptr<RenderItem> AddModelToScene(std::string name, const std::string& modelPath, const std::string& albedoPath, const std::string& normalPath, const std::string& mrPath,
        DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f }, bool transparent = false);
    void AddLightToScene(DirectX::XMFLOAT3 position, DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f }, bool randomColor = false);
    void RenderUI(float width, float height);
    void Run();

    // InputListener interface
    void OnKeyDown(int key) override;
    void OnKeyUp(int key) override;
    void OnMouseMove(const InputListener::Vec2& mousePosition) override;
    virtual void OnLeftMouseDown(const InputListener::Vec2& mousePos) override;
    virtual void OnRightMouseDown(const InputListener::Vec2& mousePos) override;
    virtual void OnLeftMouseUp(const InputListener::Vec2& mousePos) override;
    virtual void OnRightMouseUp(const InputListener::Vec2& mousePos) override;
    // end InputListener interface

private:
    void UpdateProjMatrix(float width, float height);
    
    std::shared_ptr<Window> m_window;
    std::shared_ptr<D3D12Renderer> m_renderer;
    std::shared_ptr<GraphicsPipeline> m_trianglePipeline;
    std::shared_ptr<Buffer> m_constantBuffer;
    std::shared_ptr<Buffer> m_lightsConstantBuffer;
    std::shared_ptr<Sampler> m_textureSampler;
    
    std::shared_ptr<RenderPass> m_deferredPass;
    std::shared_ptr<RenderPass> m_forwardPass;
    std::shared_ptr<RenderPass> m_transparencyPass;
    
    // std::vector<std::shared_ptr<RenderItem>> m_opaqueRenderItems;
    // std::vector<std::shared_ptr<RenderItem>> m_transparentRenderItems;

    std::shared_ptr<ResourcesManager> m_resourceManager;

    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<GameObject> m_selectedGo;

    std::unordered_map<std::string, std::shared_ptr<Buffer>> m_meshesInstancesBuffers;
    std::unordered_map<std::string, int> m_meshesInstancesCount;

    float m_startTime;
    float m_lastTime;
    float m_elapsedTime;

    Camera m_camera;
    float m_cameraForward;
    float m_cameraRight;
    float m_lastMousePos[2];

    float m_fov = 0.35f;
    float m_previousFov = m_fov;
    float m_moveSpeed = 18.0f;
    bool m_mouseLocked = true;

    float m_dirLightDirection[3] = { 1.0f, -1.0f, -1.0f };
    float m_dirLightIntensity = 2.0;

    // TODO remove this
    float m_testLightConstAttenuation = 0.65f;
    float m_testLightLinearAttenuation = 0.1f;
    float m_testLightQuadraticAttenuation = 0.02f;
    // DirectX::XMFLOAT4 m_testLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    bool m_enablePointLights = true;
    bool m_movePointLights = false;
    float m_movePointLightsSpeed = 0.8f;

    ImGuizmo::OPERATION m_gizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;
    int m_viewMode;
    bool m_displayUI = true;

    ImVec2 m_viewportCachedSize;
};
