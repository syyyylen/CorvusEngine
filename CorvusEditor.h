#pragma once
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
    std::shared_ptr<D3D12Renderer> m_renderer;
};
