#pragma once
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include <Core.h>
#include <dxgi1_4.h>

#include "Texture.h"

#define FRAMES_IN_FLIGHT 3

class SwapChain 
{
public:
    SwapChain(std::shared_ptr<Device> device, std::shared_ptr<CommandQueue> directQueue, std::shared_ptr<DescriptorHeap> rtvHeap, HWND window);
    ~SwapChain();

    void Present(bool vsync);
    void Resize(uint32_t width, uint32_t height);

    int AcquireImage() { return m_swapChain->GetCurrentBackBufferIndex(); }
    std::shared_ptr<Texture> GetTexture(uint32_t idx) { return m_textures[idx]; }

private:
    IDXGISwapChain3* m_swapChain;
    std::shared_ptr<Device> m_device;
    std::shared_ptr<DescriptorHeap> m_rtvHeap;
    HWND m_window;

    ID3D12Resource* m_buffers[FRAMES_IN_FLIGHT];
    DescriptorHandle m_descriptors[FRAMES_IN_FLIGHT];
    std::shared_ptr<Texture> m_textures[FRAMES_IN_FLIGHT];

    int m_width;
    int m_height;
};
