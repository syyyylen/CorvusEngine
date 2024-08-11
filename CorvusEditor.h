﻿#pragma once
#include "Core.h"
#include "Window.h"
#include "RHI/D3D12Renderer.h"

class CorvusEditor
{
public:
    CorvusEditor();
    ~CorvusEditor();

    void Run();

private:
    std::shared_ptr<Window> m_window;
    std::unique_ptr<D3D12Renderer> m_renderer;
    std::shared_ptr<GraphicsPipeline> m_trianglePipeline;
    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indicesBuffer;
    std::shared_ptr<Buffer> m_constantBuffer;

    float m_startTime;
    float m_lastTime;
    float m_elapsedTime;

    DirectX::XMFLOAT4X4 m_world;
    DirectX::XMFLOAT4X4 m_view;
    DirectX::XMFLOAT4X4 m_proj;
    float m_cam[3] = {-10.0f, 5.0f, 0.0f} ;
};
