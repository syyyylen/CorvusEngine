#pragma once
#include "Camera.h"
#include "Core.h"
#include "InputListener.h"
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
    std::unique_ptr<D3D12Renderer> m_renderer;
    std::shared_ptr<GraphicsPipeline> m_trianglePipeline;
    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indicesBuffer;
    std::shared_ptr<Buffer> m_constantBuffer;

    float m_startTime;
    float m_lastTime;
    float m_elapsedTime;

    Camera m_camera;
    float m_cameraForward;
    float m_cameraRight;
    float m_lastMousePos[2];
    
    float m_moveSpeed = 2.5f;
    bool m_mouseLocked = true;
};
