#include "Uploader.h"
#include "Image.h"
#include "CommandList.h"

Uploader::Uploader(std::shared_ptr<Device> device, const Heaps& heaps, std::shared_ptr<Allocator> allocator) : m_device(device), m_allocator(allocator)
{
    m_commandList = std::make_shared<CommandList>(device, heaps, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

Uploader::~Uploader()
{
}

void Uploader::CopyHostToDeviceShared(void *pData, uint64_t uiSize, std::shared_ptr<Buffer> destBuffer)
{
    UploadCommand command;
    command.type = UploadCommandType::HostToDeviceShared;
    command.data = pData;
    command.size = uiSize;
    command.destBuffer = destBuffer;

    m_commands.push_back(command);
}

void Uploader::CopyHostToDeviceLocal(void *pData, uint64_t uiSize, std::shared_ptr<Buffer> destBuffer)
{
    auto buffer = std::make_shared<Buffer>(m_allocator, uiSize, 0, BufferType::Copy, false);

    {
        UploadCommand command;
        command.type = UploadCommandType::HostToDeviceShared;
        command.data = pData;
        command.size = uiSize;
        command.destBuffer = buffer;

        m_commands.push_back(command);
    }

    {
        UploadCommand command;
        command.type = UploadCommandType::HostToDeviceLocal;
        command.sourceBuffer = buffer;
        command.destBuffer = destBuffer;

        m_commands.push_back(command);
    }
}

void Uploader::CopyHostToDeviceTexture(Image& image, std::shared_ptr<Texture> destTexture)
{
    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(m_allocator, image.Width * image.Height * 4, 0, BufferType::Copy, false);

    {
        UploadCommand command;
        command.type = UploadCommandType::HostToDeviceShared;
        command.data = image.Bytes;
        command.size = image.Width * image.Height * 4;
        command.destBuffer = buffer;

        m_commands.push_back(command);
    }
    
    {
        UploadCommand command;
        command.type = UploadCommandType::BufferToTexture;
        command.sourceBuffer = buffer;
        command.destTexture = destTexture;

        m_commands.push_back(command);
    }
}

void Uploader::CopyBufferToBuffer(std::shared_ptr<Buffer> sourceBuffer, std::shared_ptr<Buffer> destBuffer)
{
    UploadCommand command;
    command.type = UploadCommandType::BufferToBuffer;
    command.sourceBuffer = sourceBuffer;
    command.destBuffer = destBuffer;

    m_commands.push_back(command);
}

void Uploader::CopyTextureToTexture(std::shared_ptr<Texture> sourceTexture, std::shared_ptr<Texture> destTexture)
{
    UploadCommand command;
    command.type = UploadCommandType::TextureToTexture;
    command.sourceTexture = sourceTexture;
    command.destTexture = destTexture;

    m_commands.push_back(command);
}