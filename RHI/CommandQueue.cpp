#include "CommandQueue.h"

CommandQueue::CommandQueue(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type) : m_device(device), m_type(type), m_fenceValue(0)
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;

    HRESULT hr = m_device->GetDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue));
    if(FAILED(hr))
    {
        LOG(Error, "CommandQueue : failed to create command queue !");
        std::string errorMsg = std::system_category().message(hr);
        LOG(Error, errorMsg);
    }

    hr = device->GetDevice()->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    if(FAILED(hr))
    {
        LOG(Error, "Fence : failed to create fence !");
        std::string errorMsg = std::system_category().message(hr);
        LOG(Error, errorMsg);
    }
}

CommandQueue::~CommandQueue()
{
    m_commandQueue->Release();
    m_fence->Release();
}

void CommandQueue::Signal(ID3D12Fence* fence, uint64_t value)
{
    m_commandQueue->Signal(fence, value);
}

void CommandQueue::WaitForFenceValue(uint64_t target, uint64_t timeout)
{
    if (m_fence->GetCompletedValue() < target)
    {
        HANDLE event = CreateEvent(nullptr, false, false, nullptr);
        m_fence->SetEventOnCompletion(target, event);

        if(WaitForSingleObject(event, timeout) == WAIT_TIMEOUT)
            LOG(Error, "Fence : GPU Timeout !");
    }
}

void CommandQueue::Submit(const std::vector<std::shared_ptr<CommandList>>& buffers)
{
    std::vector<ID3D12CommandList*> lists;
    lists.reserve(buffers.size());
    for (auto& buffer : buffers) {
        lists.push_back(buffer->GetCommandList());
    }

    m_commandQueue->ExecuteCommandLists(lists.size(), lists.data());
}