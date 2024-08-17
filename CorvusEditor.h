﻿#pragma once
#include "Camera.h"
#include "Core.h"
#include "InputListener.h"
#include "RenderItem.h"
#include "Window.h"
#include "RHI/D3D12Renderer.h"

class CorvusEditor : public InputListener
{
public:
    CorvusEditor();
    ~CorvusEditor();

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
    std::shared_ptr<Window> m_window;
    std::shared_ptr<D3D12Renderer> m_renderer;
    std::shared_ptr<GraphicsPipeline> m_trianglePipeline;
    std::shared_ptr<Buffer> m_constantBuffer;
    std::shared_ptr<Buffer> m_objectConstantBuffer;
    std::shared_ptr<Texture> m_depthBuffer;
    std::shared_ptr<Sampler> m_textureSampler;

    // TODO material system
    std::shared_ptr<Texture> m_albedoTexture;
    std::shared_ptr<Texture> m_normalTexture;

    float m_startTime;
    float m_lastTime;
    float m_elapsedTime;

    Camera m_camera;
    float m_cameraForward;
    float m_cameraRight;
    float m_lastMousePos[2];

    float m_fov = 0.35f;
    float m_previousFov = m_fov;
    float m_moveSpeed = 6.5f;
    bool m_mouseLocked = true;

    std::vector<std::shared_ptr<RenderItem>> m_renderItems;
};
