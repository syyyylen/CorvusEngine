#include "CorvusEditor.h"
#include "Logger.h"
#include <ImGui/imgui.h>
#include "ShaderCompiler.h"
#include "RHI/Buffer.h"
#include "RHI/Uploader.h"

struct TempCbuf
{
    float color[4];
    float time;
    float padding[3];
};

CorvusEditor::CorvusEditor()
{
    LOG(Debug, "Starting Corvus Editor");

    m_window = std::make_shared<Window>(1280, 720, L"Corvus Editor");
    m_window->DefineOnResize([this](int width, int height)
    {
        LOG(Debug, "Window resize !");
        m_renderer->Resize(width, height);
    });

    // m_window->Maximize();

    m_renderer = std::make_unique<D3D12Renderer>(m_window->GetHandle());

    GraphicsPipelineSpecs specs;
    specs.FormatCount = 1;
    specs.Formats[0] = TextureFormat::RGBA8;
    specs.DepthEnabled = false;
    specs.Cull = CullMode::None;
    specs.Fill = FillMode::Solid;
    ShaderCompiler::CompileShader("Shaders/SimpleVertex.hlsl", ShaderType::Vertex, specs.ShadersBytecodes[ShaderType::Vertex]);
    ShaderCompiler::CompileShader("Shaders/SimplePixel.hlsl", ShaderType::Pixel, specs.ShadersBytecodes[ShaderType::Pixel]);

    m_trianglePipeline = m_renderer->CreateGraphicsPipeline(specs);

    float vertices[] = {
        0.5f,  0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
       -0.5f,  0.5f, 0.0f
   };

    uint32_t indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    m_vertexBuffer = m_renderer->CreateBuffer(sizeof(vertices), sizeof(float) * 3, BufferType::Vertex, false);
    m_indicesBuffer = m_renderer->CreateBuffer(sizeof(indices), sizeof(uint32_t), BufferType::Index, false);
    m_constantBuffer = m_renderer->CreateBuffer(256, 0, BufferType::Constant, false);
    m_renderer->CreateConstantBuffer(m_constantBuffer);
    
    Uploader uploader = m_renderer->CreateUploader();
    uploader.CopyHostToDeviceLocal(vertices, sizeof(vertices), m_vertexBuffer);
    uploader.CopyHostToDeviceLocal(indices, sizeof(indices), m_indicesBuffer);
    m_renderer->FlushUploader(uploader);

    m_renderer->WaitForGPU();

    m_startTime = clock();
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

        TempCbuf cbuf;
        cbuf.color[0] = 1.0f;
        cbuf.color[1] = 0.0f;
        cbuf.color[2] = 0.0f;
        cbuf.color[3] = 1.0f;
        cbuf.time = m_elapsedTime;
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

        commandList->DrawIndexed(6);

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
