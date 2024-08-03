#include "CorvusEditor.h"
#include "Logger.h"
#include <ImGui/imgui.h>
#include "ShaderCompiler.h"
#include "RHI/Buffer.h"
#include "RHI/Uploader.h"

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

    Uploader uploader = m_renderer->CreateUploader();
    uploader.CopyHostToDeviceLocal(vertices, sizeof(vertices), m_vertexBuffer);
    uploader.CopyHostToDeviceLocal(indices, sizeof(indices), m_indicesBuffer);
    m_renderer->FlushUploader(uploader);

    m_renderer->WaitForGPU();
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
        uint32_t width, height;
        m_window->GetSize(width, height);

        auto commandList = m_renderer->GetCurrentCommandList();
        auto texture = m_renderer->GetBackBuffer();

        commandList->Begin();
        commandList->ImageBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);

        commandList->SetViewport(0, 0, width, height);
        commandList->SetTopology(Topology::TriangleList);

        commandList->BindRenderTargets({ texture }, nullptr);
        commandList->BindGraphicsPipeline(m_trianglePipeline);
        commandList->BindVertexBuffer(m_vertexBuffer);
        commandList->BindIndexBuffer(m_indicesBuffer);

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
