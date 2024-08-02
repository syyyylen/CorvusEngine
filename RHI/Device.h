#pragma once
#include <Core.h>

class Device
{
public:
    Device();
    ~Device();

    ID3D12Device* GetDevice() { return m_device; }
    IDXGIAdapter1* GetAdapter() { return m_adapter; }
    IDXGIFactory3* GetFactory() { return m_factory; }

private:
    ID3D12Device* m_device;
    ID3D12Debug* m_debug;
    ID3D12DebugDevice* m_debugDevice;
    IDXGIFactory3* m_factory;
    IDXGIAdapter1* m_adapter;
    
};
