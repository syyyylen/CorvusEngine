#pragma once
#include "Core.h"
#include "Device.h"

class D3D12Renderer
{
public:
    D3D12Renderer(HWND hwnd);
    ~D3D12Renderer();

private:
    std::shared_ptr<Device> m_device;
};
