#include "CorvusEditor.h"
#include "Logger.h"
#include <ImGui/imgui.h>

#include "InputSystem.h"
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

    InputSystem::Create();
    InputSystem::Get()->AddListener(this);
    InputSystem::Get()->ShowCursor(false);

    auto updateProjMatrix = [this](float width, float height)
    {
        float aspectRatio = width / height;
        m_camera.UpdatePerspectiveFOV(m_fov * 3.14159f, aspectRatio);
    };

    m_window = std::make_shared<Window>(1380, 960, L"Corvus Editor");
    m_window->DefineOnResize([this, updateProjMatrix](int width, int height)
    {
        LOG(Debug, "Window resize !");
        m_renderer->Resize(width, height);
        m_depthBuffer.reset();
        m_depthBuffer = m_renderer->CreateTexture(width, height, TextureFormat::R32Depth, TextureType::DepthTarget);
        m_renderer->CreateDepthView(m_depthBuffer);
        
        updateProjMatrix((float)width, (float)height);
    });

    m_renderer = std::make_shared<D3D12Renderer>(m_window->GetHandle());

    m_textureSampler = m_renderer->CreateSampler(D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    m_depthBuffer = m_renderer->CreateTexture(1380, 960, TextureFormat::R32Depth, TextureType::DepthTarget);
    m_renderer->CreateDepthView(m_depthBuffer);

    GraphicsPipelineSpecs specs;
    specs.FormatCount = 1;
    specs.Formats[0] = TextureFormat::RGBA8;
    specs.DepthEnabled = true;
    specs.Depth = DepthOperation::Less;
    specs.DepthFormat = TextureFormat::R32Depth;
    specs.Cull = CullMode::None;
    specs.Fill = FillMode::Solid;
    ShaderCompiler::CompileShader("Shaders/SimpleVertex.hlsl", ShaderType::Vertex, specs.ShadersBytecodes[ShaderType::Vertex]);
    ShaderCompiler::CompileShader("Shaders/SimplePixel.hlsl", ShaderType::Pixel, specs.ShadersBytecodes[ShaderType::Pixel]);

    m_trianglePipeline = m_renderer->CreateGraphicsPipeline(specs);
    m_constantBuffer = m_renderer->CreateBuffer(256, 0, BufferType::Constant, false);
    m_renderer->CreateConstantBuffer(m_constantBuffer);

    auto model = std::make_shared<RenderItem>();
    model->ImportMesh(m_renderer, "Assets/DamagedHelmet.gltf");
    m_renderItems.push_back(model);
    
    m_startTime = clock();

    // m_window->Maximize();

    uint32_t width, height;
    m_window->GetSize(width, height);
    updateProjMatrix((float)width, (float)height);
    
    InputSystem::Get()->SetCursorPosition(Vec2((float)width/2.0f, (float)height/2.0f));

    m_lastMousePos[0] = width/2.0f;
    m_lastMousePos[1] = height/2.0f;
}

CorvusEditor::~CorvusEditor()
{
    LOG(Debug, "Destroying Corvus Editor");

    InputSystem::Get()->RemoveListener(this);
    InputSystem::Release();
    
    Logger::WriteLogsToFile();
}

void CorvusEditor::Run()
{
    while(m_window->IsRunning())
    {
        InputSystem::Get()->Update();
        
        float time = clock() - m_startTime;
        float dt = (time - m_lastTime) / 1000.0f;
        m_lastTime = time;
        m_elapsedTime += dt;
        
        uint32_t width, height;
        m_window->GetSize(width, height);

        auto commandList = m_renderer->GetCurrentCommandList();
        auto texture = m_renderer->GetBackBuffer();

        if(m_fov != m_previousFov)
        {
            m_camera.UpdatePerspectiveFOV(m_fov * 3.14159f, (float)width / (float)height);
            m_previousFov = m_fov;
        }

        m_camera.Walk(m_cameraForward * (m_moveSpeed * dt));
        m_camera.Strafe(m_cameraRight * (m_moveSpeed * dt));

        m_camera.UpdateViewMatrix();

        auto view = m_camera.GetViewMatrix();
        auto proj = m_camera.GetProjMatrix();

        DirectX::XMMATRIX worldViewProj = view * proj; // TODO add world mat

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

        commandList->BindRenderTargets({ texture }, m_depthBuffer);
        commandList->ClearRenderTarget(texture, 0.0f, 0.0f, 0.0f, 1.0f);
        commandList->ClearDepthTarget(m_depthBuffer);
        
        commandList->BindGraphicsPipeline(m_trianglePipeline);
        
        commandList->BindConstantBuffer(m_constantBuffer, 0);

        // commandList->BindGraphicsSampler(m_textureSampler, 1);

        for(const auto renderItem : m_renderItems)
        {
            for(const auto& primitive : renderItem->m_primitives)
            {
                commandList->BindVertexBuffer(primitive.m_vertexBuffer);
                commandList->BindIndexBuffer(primitive.m_indicesBuffer);
                commandList->DrawIndexed(primitive.m_indexCount);
            }
        }

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

        ImGui::Begin("Debug");
        ImGui::SliderFloat("FOV", &m_fov, 0.1f, 1.0f);
        ImGui::SliderFloat("Move Speed", &m_moveSpeed, 1.0f, 20.0f);
        ImGui::End();
        
        m_renderer->EndImGuiFrame();

        commandList->ImageBarrier(texture, D3D12_RESOURCE_STATE_PRESENT);
        commandList->End();
        m_renderer->ExecuteCommandBuffers({ commandList }, D3D12_COMMAND_LIST_TYPE_DIRECT);

        m_renderer->Present(false);
        m_renderer->EndFrame();

        m_window->BroadCast();
    }
}

void CorvusEditor::OnKeyDown(int key)
{
    if(key == 'Z')
        m_cameraForward = 1.0f;
    else if(key == 'S')
        m_cameraForward = -1.0f;
    else if(key == 'Q')
        m_cameraRight = -1.0f;
    else if(key == 'D')
        m_cameraRight = 1.0f;
}

void CorvusEditor::OnKeyUp(int key)
{
    m_cameraForward = 0.0f;
    m_cameraRight = 0.0f;
    
    if(key == 'E')
    {
        m_mouseLocked = m_mouseLocked ? false : true;
        InputSystem::Get()->ShowCursor(!m_mouseLocked);
        uint32_t width, height;
        m_window->GetSize(width, height);
        InputSystem::Get()->SetCursorPosition(Vec2(width/2.0f, height/2.0f));
        m_lastMousePos[0] = width/2.0f;
        m_lastMousePos[1] = height/2.0f;
    }
}

void CorvusEditor::OnMouseMove(const InputListener::Vec2& mousePosition)
{
    if(!m_mouseLocked)
        return;

    float dx = DirectX::XMConvertToRadians(0.25f * (mousePosition.X - m_lastMousePos[0]));
    float dy = DirectX::XMConvertToRadians(0.25f * (mousePosition.Y - m_lastMousePos[1]));

    m_camera.Pitch(dy);
    m_camera.RotateY(dx);
    
    m_lastMousePos[0] = mousePosition.X;
    m_lastMousePos[1] = mousePosition.Y;
}

void CorvusEditor::OnLeftMouseDown(const InputListener::Vec2& mousePos)
{
}

void CorvusEditor::OnRightMouseDown(const InputListener::Vec2& mousePos)
{
}

void CorvusEditor::OnLeftMouseUp(const InputListener::Vec2& mousePos)
{
}

void CorvusEditor::OnRightMouseUp(const InputListener::Vec2& mousePos)
{
}
