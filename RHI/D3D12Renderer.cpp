#include "D3D12Renderer.h"
#include <ImGui/imgui_impl_dx12.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui.h>

D3D12Renderer::D3D12Renderer(HWND hwnd) : m_frameIndex(0)
{
    m_device = std::make_shared<Device>();
    m_directCommandQueue = std::make_shared<CommandQueue>(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_computeCommandQueue = std::make_shared<CommandQueue>(m_device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
    m_copyCommandQueue = std::make_shared<CommandQueue>(m_device, D3D12_COMMAND_LIST_TYPE_COPY);
    m_heaps.RtvHeap = std::make_shared<DescriptorHeap>(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 512);
    m_heaps.ShaderHeap = std::make_shared<DescriptorHeap>(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2048);
    m_allocator = std::make_shared<Allocator>(m_device);
    m_swapChain = std::make_shared<SwapChain>(m_device, m_directCommandQueue, m_heaps.RtvHeap, hwnd);

    LOG(Debug, "Renderer Initialization Completed");

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        m_commandBuffers[i] = std::make_shared<CommandList>(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        m_frameValues[i] = 0;
    }

    m_fontDescriptor = m_heaps.ShaderHeap->Allocate();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_EnableDpiAwareness();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(m_device->GetDevice(), FRAMES_IN_FLIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, m_heaps.ShaderHeap->GetHeap(), m_fontDescriptor.CPU, m_fontDescriptor.GPU);
}

D3D12Renderer::~D3D12Renderer()
{
    WaitForPreviousFrame();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    m_heaps.ShaderHeap->Free(m_fontDescriptor);
}

void D3D12Renderer::Resize(uint32_t width, uint32_t height)
{
    WaitForPreviousFrame();

    if(m_swapChain)
        m_swapChain->Resize(width, height);
}

void D3D12Renderer::BeginFrame()
{
    m_frameIndex = m_swapChain->AcquireImage();
    m_directCommandQueue->WaitForFenceValue(m_frameValues[m_frameIndex], 10'000'000);

    m_allocator->GetAllocator()->SetCurrentFrameIndex(m_frameIndex);
}

void D3D12Renderer::EndFrame()
{
    m_frameValues[m_frameIndex] = m_directCommandQueue->GetFenceValue();
}

void D3D12Renderer::Present(bool vsync)
{
    m_swapChain->Present(vsync);
}

void D3D12Renderer::BeginImGuiFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void D3D12Renderer::EndImGuiFrame()
{
    auto cmdList = m_commandBuffers[m_frameIndex]->GetCommandList();

    ID3D12DescriptorHeap* pHeaps[] = { m_heaps.ShaderHeap->GetHeap() };
    cmdList->SetDescriptorHeaps(1, pHeaps);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
}

void D3D12Renderer::ExecuteCommandBuffers(const std::vector<std::shared_ptr<CommandList>>& buffers, D3D12_COMMAND_LIST_TYPE type)
{
    if(type == D3D12_COMMAND_LIST_TYPE_DIRECT)
    {
        m_directCommandQueue->Submit(buffers);
        m_directCommandQueue->Signal();
        return;
    }

    if(type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
    {
        m_computeCommandQueue->Submit(buffers);
        m_computeCommandQueue->Signal();
        return;
    }

    if(type == D3D12_COMMAND_LIST_TYPE_COPY)
    {
        m_copyCommandQueue->Submit(buffers);
        m_copyCommandQueue->Signal();
        return;
    }
}

std::shared_ptr<GraphicsPipeline> D3D12Renderer::CreateGraphicsPipeline(GraphicsPipelineSpecs& specs)
{
    return std::make_shared<GraphicsPipeline>(m_device, specs);
}

std::shared_ptr<Buffer> D3D12Renderer::CreateBuffer(uint64_t size, uint64_t stride, BufferType type, bool readback)
{
    return std::make_shared<Buffer>(m_allocator, size, stride, type, readback);
}

Uploader D3D12Renderer::CreateUploader()
{
    return Uploader(m_device, m_allocator);
}

void D3D12Renderer::FlushUploader(Uploader& uploader)
{
    uploader.m_commandList->Begin();

    for(auto& command : uploader.m_commands)
    {
        auto cmdList = uploader.m_commandList;

        switch (command.type) {
            case Uploader::UploadCommandType::HostToDeviceShared: {
                void *pData;
                command.destBuffer->Map(0, 0, &pData);
                memcpy(pData, command.data, command.size);
                command.destBuffer->Unmap(0, 0);
                break;
            }
            case Uploader::UploadCommandType::BufferToBuffer:
            case Uploader::UploadCommandType::HostToDeviceLocal: {
                cmdList->CopyBufferToBuffer(command.destBuffer, command.sourceBuffer);
                break;
            }
            case Uploader::UploadCommandType::TextureToTexture: {
                cmdList->CopyTextureToTexture(command.destTexture, command.sourceTexture);
                break;
            }
        }
    }

    uploader.m_commandList->End();
    ExecuteCommandBuffers({ uploader.m_commandList }, D3D12_COMMAND_LIST_TYPE_COPY);
    WaitForPreviousDeviceSubmit(D3D12_COMMAND_LIST_TYPE_COPY);
    uploader.m_commands.clear();
}

void D3D12Renderer::WaitForPreviousFrame()
{
    uint64_t wait = m_directCommandQueue->Signal();
    m_directCommandQueue->WaitForFenceValue(wait, 10'000'000);
}

void D3D12Renderer::WaitForPreviousDeviceSubmit(D3D12_COMMAND_LIST_TYPE type)
{
    switch (type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT: {
            m_directCommandQueue->WaitGPUSide();
            break;
        }
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:  {
            m_computeCommandQueue->WaitGPUSide();
            break;
        }
        case D3D12_COMMAND_LIST_TYPE_COPY: {
            m_copyCommandQueue->WaitGPUSide();
            break;
        }
    }
}