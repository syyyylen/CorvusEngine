﻿#include "CorvusEditor.h"
#include "Logger.h"
#include <ImGui/imgui.h>

#include "Image.h"
#include "InputSystem.h"
#include "ShaderCompiler.h"
#include "RHI/Buffer.h"
#include "RHI/Uploader.h"

#define MAX_LIGHTS 1

struct PointLight
{
    DirectX::XMFLOAT3 Position;
    float Padding1 = 0.0f;
    // 16 bytes boundary 
    DirectX::XMFLOAT4 Color = { 1.0, 1.0, 1.0, 1.0 };
    // 16 bytes boundary 
    float ConstantAttenuation = 1.0f;
    float LinearAttenuation = 0.2f;
    float QuadraticAttenuation = 0.1f;
    float Padding3;
    // 16 bytes boundary
    int Enabled = true;
    float Padding4[3];
};

struct PointLightsConstantBuffer
{
    PointLight PointLights[MAX_LIGHTS];
};

struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 ViewProj;
    float Time;
    DirectX::XMFLOAT3 CameraPosition;
    int Mode;
};

struct ObjectConstantBuffer
{
    DirectX::XMFLOAT4X4 World;
    int HasAlbedo = false;
    int HasNormalMap = false;
    int Padding1;
    int Padding2;
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

    m_lightsConstantBuffer = m_renderer->CreateBuffer(256 * 2, 0, BufferType::Constant, false);
    m_renderer->CreateConstantBuffer(m_lightsConstantBuffer);

    auto addModel = [this](const std::string& modelPath, const std::string& albedoPath, const std::string& normalPath,
        float offsetX = 0.0f, float offsetY = 0.0f, float rotX = 0.0f, float scale = 1.0f)
    {
        auto model = std::make_shared<RenderItem>();
        model->ImportMesh(m_renderer, modelPath);

        Uploader uploader = m_renderer->CreateUploader();
        Image albedoImg; // We need those img inside the uploader scope
        Image normalImg;

        if(!albedoPath.empty())
        {
            albedoImg.LoadImageFromFile(albedoPath);
            auto albedoTexture = m_renderer->CreateTexture(albedoImg.Width, albedoImg.Height, TextureFormat::RGBA8, TextureType::ShaderResource);
            m_renderer->CreateShaderResourceView(albedoTexture);
            model->GetMaterial().HasAlbedo = true;
            model->GetMaterial().Albedo = albedoTexture;
            uploader.CopyHostToDeviceTexture(albedoImg, albedoTexture);
        }

        if(!normalPath.empty())
        {
            normalImg.LoadImageFromFile(normalPath);
            auto normalTexture = m_renderer->CreateTexture(normalImg.Width, normalImg.Height, TextureFormat::RGBA8, TextureType::ShaderResource);
            m_renderer->CreateShaderResourceView(normalTexture);
            model->GetMaterial().HasNormal = true;
            model->GetMaterial().Normal = normalTexture;
            uploader.CopyHostToDeviceTexture(normalImg, normalTexture);
        }

        if(uploader.HasCommands())
            m_renderer->FlushUploader(uploader);

        DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&model->GetPrimitives()[0].Transform);
        mat *= DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotX));
        mat *= DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(180.0f));
        mat *= DirectX::XMMatrixTranslation(offsetX, offsetY, 0.0f);
        mat *= DirectX::XMMatrixScaling(scale, scale, scale);
        DirectX::XMStoreFloat4x4(&model->GetPrimitives()[0].Transform, mat);

        ObjectConstantBuffer objCbuf;
        objCbuf.HasAlbedo = model->GetMaterial().HasAlbedo;
        objCbuf.HasNormalMap = model->GetMaterial().HasNormal;
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&model->GetPrimitives()[0].Transform);
        DirectX::XMStoreFloat4x4(&objCbuf.World, DirectX::XMMatrixTranspose(world));

        model->GetPrimitives()[0].m_objectConstantBuffer = m_renderer->CreateBuffer(256, 0, BufferType::Constant, false);
        m_renderer->CreateConstantBuffer(model->GetPrimitives()[0].m_objectConstantBuffer);

        void* objCbufData;
        model->GetPrimitives()[0].m_objectConstantBuffer->Map(0, 0, &objCbufData);
        memcpy(objCbufData, &objCbuf, sizeof(ObjectConstantBuffer));
        model->GetPrimitives()[0].m_objectConstantBuffer->Unmap(0, 0);

        m_renderItems.push_back(model);
    };

    addModel("Assets/DamagedHelmet.gltf", "Assets/DamagedHelmet_albedo.jpg", "Assets/DamagedHelmet_normal.jpg", 0.0f, 0.0f, 90.0f);
    addModel("Assets/SciFiHelmet.gltf", "Assets/SciFiHelmet_BaseColor.png", "Assets/SciFiHelmet_Normal.png", 3.0);
    addModel("Assets/sphere.gltf", "", "", 6.0f);
    addModel("Assets/dragon.obj", "", "", 38.0f, -4.0f, 0.0f, 0.25f);
    
    m_startTime = clock();

    m_window->Maximize();

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

        DirectX::XMMATRIX viewProj = view * proj;

        SceneConstantBuffer cbuf;
        cbuf.Time = m_elapsedTime;
        cbuf.CameraPosition = m_camera.GetPosition();
        cbuf.Mode = m_viewMode;
        DirectX::XMStoreFloat4x4(&cbuf.ViewProj, DirectX::XMMatrixTranspose(viewProj));
        
        void* data;
        m_constantBuffer->Map(0, 0, &data);
        memcpy(data, &cbuf, sizeof(SceneConstantBuffer));
        m_constantBuffer->Unmap(0, 0);

        // TODO point lights & proper game object inspector
        
        PointLight testLight = {};
        testLight.Position = m_lightPosition;
        testLight.ConstantAttenuation = m_lightConstantAttenuation;
        testLight.LinearAttenuation = m_lightLinearAttenuation;
        testLight.QuadraticAttenuation = m_lightQuadraticAttenuation;
        testLight.Color = m_lightColor;
        
        PointLightsConstantBuffer lightsCbuf;
        lightsCbuf.PointLights[0] = testLight;

        void* data2;
        m_lightsConstantBuffer->Map(0, 0, &data2);
        memcpy(data2, &lightsCbuf, sizeof(PointLightsConstantBuffer));
        m_lightsConstantBuffer->Unmap(0, 0);

        // TODO point lights 

        commandList->Begin();
        commandList->ImageBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);

        commandList->SetViewport(0, 0, width, height);
        commandList->SetTopology(Topology::TriangleList);

        commandList->BindRenderTargets({ texture }, m_depthBuffer);
        commandList->ClearRenderTarget(texture, 0.0f, 0.0f, 0.0f, 1.0f);
        commandList->ClearDepthTarget(m_depthBuffer);
        
        commandList->BindGraphicsPipeline(m_trianglePipeline);
        
        commandList->BindConstantBuffer(m_constantBuffer, 0);
        commandList->BindConstantBuffer(m_lightsConstantBuffer, 5);
        
        commandList->BindGraphicsSampler(m_textureSampler, 2);

        for(const auto renderItem : m_renderItems)
        {
            auto& material = renderItem->GetMaterial();

            if(material.HasAlbedo)
                commandList->BindGraphicsShaderResource(material.Albedo, 3);

            if(material.HasNormal)
                commandList->BindGraphicsShaderResource(material.Normal, 4);
            
            const auto primitives = renderItem->GetPrimitives();
            for(const auto& primitive : primitives)
            {
                commandList->BindConstantBuffer(primitive.m_objectConstantBuffer, 1);
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
        static const char* modes[] = { "Default", "Diffuse", "Specular", "Albedo", "Normal", "Debug" };
        ImGui::Combo("View Mode", (int*)&m_viewMode, modes, 6);
        ImGui::End();

        ImGui::Begin("Debug Point Light");
        float pos[3] = { m_lightPosition.x, m_lightPosition.y, m_lightPosition.z };
        ImGui::InputFloat3("Position", pos);
        m_lightPosition.x = pos[0];
        m_lightPosition.y = pos[1];
        m_lightPosition.z = pos[2];
        ImGui::SliderFloat("C Attenuation", &m_lightConstantAttenuation, 0.0f, 1.0f);
        ImGui::SliderFloat("L Attenuation", &m_lightLinearAttenuation, 0.0f, 1.0f);
        ImGui::SliderFloat("Q Attenuation", &m_lightQuadraticAttenuation, 0.0f, 1.0f);
        float color[4] = { m_lightColor.x, m_lightColor.y, m_lightColor.z, m_lightColor.w };
        ImGui::ColorEdit4("Color", color);
        m_lightColor.x = color[0];
        m_lightColor.y = color[1];
        m_lightColor.z = color[2];
        m_lightColor.w = color[3];
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
