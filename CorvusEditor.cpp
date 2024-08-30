#include "CorvusEditor.h"

#include <random>

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
        // m_transparencyPass->OnResize(m_renderer, width, height);
        updateProjMatrix((float)width, (float)height);
    });

    m_renderer = std::make_shared<D3D12Renderer>(m_window->GetHandle());

    m_resourceManager = std::make_shared<ResourcesManager>(m_renderer);

    m_deferredPass = std::make_shared<DeferredRenderPass>();
    m_deferredPass->Initialize(m_renderer, defaultWidth, defaultHeight);

    // m_transparencyPass = std::make_shared<TransparencyRenderPass>();
    // m_transparencyPass->Initialize(m_renderer, defaultWidth, defaultHeight);

    // ----------------------------------------------- POINT LIGHTS DEMO ------------------------------------------------
    if(true)
    {
        m_dirLightIntensity = 0.0f;
        
        constexpr float space = 3.0f;
        constexpr int row = 10;
        constexpr int column = 10;
    
        // AddModelToScene("Assets/cube.obj", "", "", { space * row/2, -0.5f, space * column/2 }, {}, { 25.0f, 0.2f, 25.0f });
    
        for(int i = 0; i < row; i++)
        {
            for(int j = 0; j < column; j++)
            {
                float posX = space * (float)i;
                float posZ = space * (float)j;

                if(i % 2 == 0)
                    AddModelToScene("Assets/dragon.obj", "", "", { posX, 0.0f, posZ }, { 0.0f, 0.0f, 0.0f }, { 0.25f, 0.25f, 0.25f });
                else
                    AddLightToScene({ posX, 1.0f, posZ }, {}, true);
            }
        }
    }
    // ----------------------------------------------- POINT LIGHTS DEMO ------------------------------------------------

    if(false)
    {
        AddModelToScene("Assets/DamagedHelmet.gltf", "Assets/DamagedHelmet_albedo.jpg", "Assets/DamagedHelmet_normal.jpg", {}, { 180.0f, 0.0f, -90.0f });
        AddModelToScene("Assets/DamagedHelmet.gltf", "Assets/DamagedHelmet_albedo.jpg", "Assets/DamagedHelmet_normal.jpg", { 3.0f, 0.0f, 0.0f }, { 180.0f, 0.0f, -90.0f });
        AddModelToScene("Assets/DamagedHelmet.gltf", "Assets/DamagedHelmet_albedo.jpg", "Assets/DamagedHelmet_normal.jpg", { 6.0f, 0.0f, 0.0f }, { 180.0f, 0.0f, -90.0f });
    }
    
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

void CorvusEditor::AddModelToScene(const std::string& modelPath, const std::string& albedoPath, const std::string& normalPath, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale, bool transparent)
{
    auto model = std::make_shared<RenderItem>();
    model->ImportMesh(m_renderer, modelPath);

    Uploader uploader = m_renderer->CreateUploader();
    Image albedoImg;
    Image normalImg;

    if(!albedoPath.empty())
    {
        auto albedoTexture = m_resourceManager->LoadTexture(albedoPath, uploader, albedoImg);
        model->GetMaterial().HasAlbedo = true;
        model->GetMaterial().Albedo = albedoTexture;
    }

    if(!normalPath.empty())
    {
        auto normalTexture = m_resourceManager->LoadTexture(normalPath, uploader, normalImg);
        model->GetMaterial().HasNormal = true;
        model->GetMaterial().Normal = normalTexture;
    }

    if(uploader.HasCommands())
        m_renderer->FlushUploader(uploader);

    DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&model->GetPrimitives()[0].Transform);
    mat *= DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotation.x));
    mat *= DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotation.y));
    mat *= DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotation.z));
    mat *= DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
    mat *= DirectX::XMMatrixTranslation(position.x, position.y, position.z);
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
}

void CorvusEditor::AddLightToScene(DirectX::XMFLOAT3 position, DirectX::XMFLOAT4 color, bool randomColor)
{
    auto RandFloatRange = [](float min, float max) -> float
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distribution(min, max);
        return distribution(gen);
    };
    
    PointLight pointLight;
    pointLight.Position = position;
    if(randomColor)
        pointLight.Color = { RandFloatRange(0.0f, 1.0f), RandFloatRange(0.0f, 1.0f), RandFloatRange(0.0f, 1.0f), 1.0f };
    else
        pointLight.Color = color;
    pointLight.ConstantAttenuation = m_testLightConstAttenuation;
    pointLight.LinearAttenuation = m_testLightLinearAttenuation;
    pointLight.QuadraticAttenuation = m_testLightQuadraticAttenuation;
    
    m_pointLights.emplace_back(pointLight);
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

        for(auto& pointLight : m_pointLights)
        {
            if(m_movePointLights)
            {
                auto pos = pointLight.Position;
                auto posY = cos(m_elapsedTime * m_movePointLightsSpeed) * 10.0f;
                pointLight.Position = { pos.x, posY, pos.z };
            }
            
            pointLight.ConstantAttenuation = m_testLightConstAttenuation;
            pointLight.LinearAttenuation = m_testLightLinearAttenuation;
            pointLight.QuadraticAttenuation = m_testLightQuadraticAttenuation;
        }

        std::vector<PointLight> pl = {};
        GlobalPassData passData = {};
        passData.DeltaTime = dt;
        passData.ElapsedTime = m_elapsedTime;
        passData.ViewMode = m_viewMode;
        passData.PointLights = m_enablePointLights ? m_pointLights : pl;
        passData.DirectionalInfo.Direction = { m_dirLightDirection[0], m_dirLightDirection[1], m_dirLightDirection[2] };
        passData.DirectionalInfo.Intensity = m_dirLightIntensity;

        auto commandList = m_renderer->GetCurrentCommandList();
        auto backbuffer = m_renderer->GetBackBuffer();
        
        commandList->Begin();
        commandList->ImageBarrier(backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandList->SetViewport(0, 0, width, height);
        commandList->BindRenderTargets({ backbuffer }, nullptr);
        commandList->ClearRenderTarget(backbuffer, 0.0f, 0.0f, 0.0f, 1.0f);

        m_deferredPass->Pass(m_renderer, passData, m_camera, m_opaqueRenderItems);
        // m_transparencyPass->Pass(m_renderer, passData, m_camera, m_transparentRenderItems);
        
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
        static const char* modes[] = { "Default", "Albedo", "Normal", "Depth", "WorldPosition", "Debug" };
        ImGui::Combo("View Mode", (int*)&m_viewMode, modes, 6);
        ImGui::SliderFloat3("DirLight Direction", m_dirLightDirection, -1.0f, 1.0f);
        ImGui::SliderFloat("DirLight Intensity", &m_dirLightIntensity, 0.0f, 1.0f);
        ImGui::End();

        ImGui::Begin("Debug Point Lights");
        ImGui::Checkbox("Enable Point Lights", &m_enablePointLights);
        ImGui::Checkbox("Move", &m_movePointLights);
        ImGui::SliderFloat("MoveSpeed", &m_movePointLightsSpeed, 0.0f, 8.0f);
        ImGui::SliderFloat("Constant", &m_testLightConstAttenuation, 0.0f, 1.0f);
        ImGui::SliderFloat("Linear", &m_testLightLinearAttenuation, 0.0f, 1.0f);
        ImGui::SliderFloat("Quadratic", &m_testLightQuadraticAttenuation, 0.0f, 1.0f);
        // float color[4] = { m_testLightColor.x, m_testLightColor.y, m_testLightColor.z, m_testLightColor.w };
        // ImGui::ColorEdit4("Color", color);
        // m_testLightColor.x = color[0];
        // m_testLightColor.y = color[1];
        // m_testLightColor.z = color[2];
        // m_testLightColor.w = color[3];
        ImGui::End();

        auto GBuffer = std::static_pointer_cast<DeferredRenderPass>(m_deferredPass)->GetGBuffer();
        
        ImGui::Begin("Debug GBuffer");
        ImGui::Image((ImTextureID)GBuffer.AlbedoRenderTarget->m_srvUav.GPU.ptr, ImVec2(480, 260));
        ImGui::Image((ImTextureID)GBuffer.NormalRenderTarget->m_srvUav.GPU.ptr, ImVec2(480, 260));
        ImGui::Image((ImTextureID)GBuffer.WorldPositionRenderTarget->m_srvUav.GPU.ptr, ImVec2(480, 260));
        ImGui::Image((ImTextureID)GBuffer.DepthBuffer->m_srvUav.GPU.ptr, ImVec2(480, 260));
        ImGui::End();
        
        m_renderer->EndImGuiFrame();

        commandList->ImageBarrier(backbuffer, D3D12_RESOURCE_STATE_PRESENT);
        commandList->End();
        m_renderer->ExecuteCommandBuffers({ commandList }, D3D12_COMMAND_LIST_TYPE_DIRECT);

        // m_renderer->WaitForGPU();

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
