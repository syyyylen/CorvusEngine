#pragma once
#include <Core.h>

#include "Allocator.h"
#include "Device.h"

struct Image;
class Renderer;
class Buffer;
class Texture;
class CommandList;
struct Heaps;

class Uploader 
{
public:
    Uploader(std::shared_ptr<Device> device, const Heaps& heaps, std::shared_ptr<Allocator> allocator);
    ~Uploader();

    void CopyHostToDeviceShared(void* pData, uint64_t uiSize, std::shared_ptr<Buffer> destBuffer);
    void CopyHostToDeviceLocal(void* pData, uint64_t uiSize, std::shared_ptr<Buffer> destBuffer);
    void CopyHostToDeviceTexture(Image& image, std::shared_ptr<Texture> destTexture);
    void CopyBufferToBuffer(std::shared_ptr<Buffer> sourceBuffer, std::shared_ptr<Buffer> destBuffer);
    void CopyTextureToTexture(std::shared_ptr<Texture> sourceTexture, std::shared_ptr<Texture> destTexture);
    bool HasCommands() { return !m_commands.empty(); }

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
        TextureToTexture,
        BufferToTexture
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