#pragma once
#include <Core.h>

#include "Allocator.h"
#include "Device.h"

class Renderer;
class Buffer;
class Texture;
class CommandList;

class Uploader 
{
public:
    Uploader(std::shared_ptr<Device> device, std::shared_ptr<Allocator> allocator);
    ~Uploader();

    void CopyHostToDeviceShared(void* pData, uint64_t uiSize, std::shared_ptr<Buffer> destBuffer);
    void CopyHostToDeviceLocal(void* pData, uint64_t uiSize, std::shared_ptr<Buffer> destBuffer);
    void CopyBufferToBuffer(std::shared_ptr<Buffer> sourceBuffer, std::shared_ptr<Buffer> destBuffer);
    void CopyTextureToTexture(std::shared_ptr<Texture> sourceTexture, std::shared_ptr<Texture> destTexture);

private:
    friend class D3D12Renderer;

    std::shared_ptr<Device> m_device;
    std::shared_ptr<Allocator> m_allocator;
    std::shared_ptr<CommandList> m_commandList;

    enum class UploadCommandType
    {
        HostToDeviceShared,
        HostToDeviceLocal,
        BufferToBuffer,
        TextureToTexture
    };

    struct UploadCommand
    {
        UploadCommandType type;
        void* data;
        uint64_t size;

        std::shared_ptr<Texture> sourceTexture;
        std::shared_ptr<Texture> destTexture;

        std::shared_ptr<Buffer> sourceBuffer;
        std::shared_ptr<Buffer> destBuffer;
    };

    std::vector<UploadCommand> m_commands;
};