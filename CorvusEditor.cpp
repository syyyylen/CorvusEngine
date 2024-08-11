#include "CorvusEditor.h"
#include "Logger.h"
#include <ImGui/imgui.h>
#include "ShaderCompiler.h"
#include "RHI/Buffer.h"
#include "RHI/Uploader.h"

struct TempCbuf
{
    DirectX::XMFLOAT4X4 worldViewProj;
    float time;
    float padding[3];
};

CorvusEditor::CorvusEditor()
{
    LOG(Debug, "Starting Corvus Editor");

    auto updateProjMatrix = [this](float width, float height)
    {
        float aspectRatio = width / height;
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * 3.14159f, aspectRatio, 1.0f, 1000.0f);
        DirectX::XMStoreFloat4x4(&m_proj, P);
    };

    m_window = std::make_shared<Window>(1920, 1080, L"Corvus Editor");
    m_window->DefineOnResize([this, updateProjMatrix](int width, int height)
    {
        LOG(Debug, "Window resize !");
        m_renderer->Resize(width, height);
        
        updateProjMatrix((float)width, (float)height);
    });

    m_renderer = std::make_unique<D3D12Renderer>(m_window->GetHandle());

    GraphicsPipelineSpecs specs;
    specs.FormatCount = 1;
    specs.Formats[0] = TextureFormat::RGBA8;
    specs.DepthEnabled = false;
    specs.Cull = CullMode::Back;
    specs.Fill = FillMode::Solid;
    ShaderCompiler::CompileShader("Shaders/SimpleVertex.hlsl", ShaderType::Vertex, specs.ShadersBytecodes[ShaderType::Vertex]);
    ShaderCompiler::CompileShader("Shaders/SimplePixel.hlsl", ShaderType::Pixel, specs.ShadersBytecodes[ShaderType::Pixel]);

    m_trianglePipeline = m_renderer->CreateGraphicsPipeline(specs);

    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 col;
    };

    std::array<Vertex, 8> vertices = {
        Vertex( { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }),
        Vertex( { DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) }),
        Vertex( { DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }),
        Vertex( { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }),
        Vertex( { DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }),
        Vertex( { DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) }),
        Vertex( { DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }),
        Vertex( { DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }),
    };

    std::array<uint32_t, 36> indices = {
        0, 1, 2,
        0, 2, 3,

        4, 6, 5,
        4, 7, 6,

        4, 5, 1,
        4, 1, 0,

        3, 2, 6,
        3, 6, 7,

        1, 5, 6,
        1, 6, 2,

        4, 0, 3,
        4, 3, 7
    };

    m_vertexBuffer = m_renderer->CreateBuffer(sizeof(Vertex) * (UINT)vertices.size(), sizeof(Vertex), BufferType::Vertex, false);
    m_indicesBuffer = m_renderer->CreateBuffer(sizeof(uint32_t) * (UINT)indices.size(), sizeof(uint32_t), BufferType::Index, false);
    m_constantBuffer = m_renderer->CreateBuffer(256, 0, BufferType::Constant, false);
    m_renderer->CreateConstantBuffer(m_constantBuffer);
    
    Uploader uploader = m_renderer->CreateUploader();
    uploader.CopyHostToDeviceLocal(vertices.data(), sizeof(vertices), m_vertexBuffer);
    uploader.CopyHostToDeviceLocal(indices.data(), sizeof(indices), m_indicesBuffer);
    m_renderer->FlushUploader(uploader);

    m_renderer->WaitForGPU();

    m_startTime = clock();

    // m_window->Maximize();

    uint32_t width, height;
    m_window->GetSize(width, height);
    updateProjMatrix((float)width, (float)height);
}

CorvusEditor::~CorvusEditor()
{
    LOG(Debug, "Destroying Corvus Editor");
    Logger::WriteLogsToFile();
}

void CorvusEditor::Run()
{
    while(m_window->IsRunning())
    {
        float time = clock() - m_startTime;
        float dt = (time - m_lastTime) / 1000.0f;
        m_lastTime = time;
        m_elapsedTime += dt;
        
        uint32_t width, height;
        m_window->GetSize(width, height);

        auto commandList = m_renderer->GetCurrentCommandList();
        auto texture = m_renderer->GetBackBuffer();

        DirectX::XMFLOAT4X4 world;
        world.m[0][0] = 1.0f;
        world.m[1][1] = 1.0f;
        world.m[2][2] = 1.0f;
        world.m[3][3] = 1.0f;

        DirectX::XMVECTOR pos = DirectX::XMVectorSet(-10.0f, 5.0f, 0.0f, 1.0f);
        DirectX::XMVECTOR target = DirectX::XMVectorZero();
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
        DirectX::XMStoreFloat4x4(&m_view, view);

        DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&m_proj);
        DirectX::XMMATRIX wrld = DirectX::XMLoadFloat4x4(&world);

        DirectX::XMMATRIX worldViewProj = wrld * view * proj;

        TempCbuf cbuf;
        cbuf.time = m_elapsedTime;
        DirectX::XMStoreFloat4x4(&cbuf.worldViewProj, DirectX::XMMatrixTranspose(worldViewProj));
        
        void* data;
        m_constantBuffer->Map(0, 0, &data);
        memcpy(data, &cbuf, sizeof(TempCbuf));
        m_constantBuffer->Unmap(0, 0);

        commandList->Begin();
        commandList->ImageBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);

        commandList->SetViewport(0, 0, width, height);
        commandList->SetTopology(Topology::TriangleList);

        commandList->BindRenderTargets({ texture }, nullptr);
        commandList->BindGraphicsPipeline(m_trianglePipeline);
        commandList->BindVertexBuffer(m_vertexBuffer);
        commandList->BindIndexBuffer(m_indicesBuffer);
        commandList->BindConstantBuffer(m_constantBuffer, 0);

        commandList->ClearRenderTarget(texture, 1.0f, 8.0f, 0.0f, 1.0f);

        commandList->DrawIndexed(36);

        m_renderer->BeginImGuiFrame();

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    m_window->Close();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGui::Begin("FrameRate");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        m_renderer->EndImGuiFrame();

        commandList->ImageBarrier(texture, D3D12_RESOURCE_STATE_PRESENT);
        commandList->End();
        m_renderer->ExecuteCommandBuffers({ commandList }, D3D12_COMMAND_LIST_TYPE_DIRECT);

        m_renderer->Present(true);
        m_renderer->EndFrame();

        m_window->BroadCast();
    }
}
