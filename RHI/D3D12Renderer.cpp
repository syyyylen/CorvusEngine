#include "D3D12Renderer.h"

D3D12Renderer::D3D12Renderer(HWND hwnd)
{
    m_device = std::make_shared<Device>();

    LOG(Debug, "Renderer Initialization Completed");
}

D3D12Renderer::~D3D12Renderer()
{
}
