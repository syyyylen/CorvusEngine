#include "CorvusEditor.h"
#include "Logger.h"
#include <ImGui/imgui.h>

#include "Image.h"
#include "InputSystem.h"
#include "Rendering/DeferredRenderPass.h"
#include "Rendering/ForwardRenderPass.h"
#include "Rendering/ShaderCompiler.h"
#include "RHI/Buffer.h"
#include "RHI/Uploader.h"
#include "Rendering/RenderingLayouts.h"
#include "Rendering/TransparencyRenderPass.h"
#include "RHI/D3D12Renderer.h"

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

    int defaultWidth = 1380;
    int defaultHeight = 960;

    m_window = std::make_shared<Window>(defaultWidth, defaultHeight, L"Corvus Editor");
    m_window->DefineOnResize([this, updateProjMatrix](int width, int height)
    {
        LOG(Debug, "Window resize !");
        m_renderer->Resize(width, height);
        m_deferredPass->OnResize(m_renderer, width, height);
        m_transparencyPass->OnResize(m_renderer, width, height);
        updateProjMatrix((float)width, (float)height);
    });

    m_renderer = std::make_shared<D3D12Renderer>(m_window->GetHandle());

    // m_forwardPass = std::make_shared<ForwardRenderPass>();
    // m_forwardPass->Initialize(m_renderer, defaultWidth, defaultHeight);

    m_deferredPass = std::make_shared<DeferredRenderPass>();
    m_deferredPass->Initialize(m_renderer, defaultWidth, defaultHeight);

    m_transparencyPass = std::make_shared<TransparencyRenderPass>();
    m_transparencyPass->Initialize(m_renderer, defaultWidth, defaultHeight);

    auto addModel = [this](const std::string& modelPath, const std::string& albedoPath, const std::string& normalPath,
        float offsetX = 0.0f, float offsetY = 0.0f, float rotX = 0.0f, float scale = 1.0f, bool transparent = false)
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

        transparent ? m_transparentRenderItems.push_back(model) : m_opaqueRenderItems.push_back(model);
    };

    addModel("Assets/DamagedHelmet.gltf", "Assets/DamagedHelmet_albedo.jpg", "Assets/DamagedHelmet_normal.jpg", 0.0f, 0.0f, 90.0f);
    addModel("Assets/SciFiHelmet.gltf", "Assets/SciFiHelmet_BaseColor.png", "Assets/SciFiHelmet_Normal.png", 3.0);
    addModel("Assets/sphere.gltf", "", "", 6.0f, 0.0f, 0.0f, 1.0f, true);
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

        if(m_fov != m_previousFov)
        {
            m_camera.UpdatePerspectiveFOV(m_fov * 3.14159f, (float)width / (float)height);
            m_previousFov = m_fov;
        }

        m_camera.Walk(m_cameraForward * (m_moveSpeed * dt));
        m_camera.Strafe(m_cameraRight * (m_moveSpeed * dt));

        m_camera.UpdateViewMatrix();
        m_camera.UpdateInvViewProjMatrix((float)width, (float)height);
        
        // TODO point lights & proper game object inspector
        PointLight testLight = {};
        testLight.Position = m_lightPosition;
        testLight.ConstantAttenuation = m_lightConstantAttenuation;
        testLight.LinearAttenuation = m_lightLinearAttenuation;
        testLight.QuadraticAttenuation = m_lightQuadraticAttenuation;
        testLight.Color = m_lightColor;
        // TODO point lights & proper game object inspector

        std::vector<PointLight> pointLights;
        pointLights.emplace_back(testLight);

        GlobalPassData passData = {};
        passData.DeltaTime = dt;
        passData.ElapsedTime = m_elapsedTime;
        passData.ViewMode = m_viewMode;
        passData.PointLights = pointLights;

        auto commandList = m_renderer->GetCurrentCommandList();
        auto backbuffer = m_renderer->GetBackBuffer();
        
        commandList->Begin();
        commandList->ImageBarrier(backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandList->SetViewport(0, 0, width, height);
        commandList->BindRenderTargets({ backbuffer }, nullptr);
        commandList->ClearRenderTarget(backbuffer, 0.0f, 0.0f, 0.0f, 1.0f);

        m_deferredPass->Pass(m_renderer, passData, m_camera, m_opaqueRenderItems);

        // m_forwardPass->Pass(m_renderer, passData, m_camera, m_opaqueRenderItems);
        m_transparencyPass->Pass(m_renderer, passData, m_camera, m_transparentRenderItems);
        
        commandList->BindRenderTargets({ backbuffer }, nullptr);

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
        
        auto GBuffer = std::static_pointer_cast<DeferredRenderPass>(m_deferredPass)->GetGBuffer();
        
        ImGui::Begin("Debug GBuffer");
        ImGui::Image((ImTextureID)GBuffer.AlbedoRenderTarget->m_srvUav.GPU.ptr, ImVec2(480, 260));
        ImGui::Image((ImTextureID)GBuffer.NormalRenderTarget->m_srvUav.GPU.ptr, ImVec2(480, 260));
        ImGui::Image((ImTextureID)GBuffer.DepthBuffer->m_srvUav.GPU.ptr, ImVec2(480, 260));
        ImGui::End();
        
        m_renderer->EndImGuiFrame();

        commandList->ImageBarrier(backbuffer, D3D12_RESOURCE_STATE_PRESENT);
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
