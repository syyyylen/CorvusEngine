#pragma once
#include <Core.h>

#include "CommandList.h"
#include "Device.h"

class CommandQueue
{
public:
    CommandQueue(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type);
    ~CommandQueue();

    uint64_t Signal();
    void WaitForFenceValue(uint64_t target, uint64_t timeout);
    void WaitGPUSide();
    void Submit(const std::vector<std::shared_ptr<CommandList>>& buffers);

    ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue; }
    D3D12_COMMAND_LIST_TYPE GetType() { return m_type; }

    ID3D12Fence* GetFence() { return m_fence; }
    uint64_t GetFenceValue() { return m_fenceValue; }

private:
    std::shared_ptr<Device> m_device;
    D3D12_COMMAND_LIST_TYPE m_type;
    ID3D12CommandQueue* m_commandQueue;
    ID3D12Fence* m_fence;
    uint64_t m_fenceValue;
};