#include "CorvusEditor.h"

#include <random>
#include <set>

#include "Logger.h"
#include "ImGui/ImGuizmo.h"
#include <ImGui/imgui.h>

#include "Image.h"
#include "InputSystem.h"
#include "Rendering/DeferredRenderPass.h"
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

    m_scene = std::make_shared<Scene>("DemoScene");

    // ----------------------------------------------- POINT LIGHTS DEMO ------------------------------------------------
    
    AddModelToScene("SciFiHelmet", "Assets/SciFiHelmet.gltf", "Assets/SciFiHelmet_BaseColor.png", "Assets/SciFiHelmet_Normal.png",
            "Assets/SciFiHelmet_MetallicRoughness.png", { -6.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
    
    AddModelToScene("SciFiHelmet", "Assets/SciFiHelmet.gltf", "Assets/SciFiHelmet_BaseColor.png", "Assets/SciFiHelmet_Normal.png",
        "Assets/SciFiHelmet_MetallicRoughness.png", { -9.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });

    constexpr bool pointLightsDemo = true;
    if(pointLightsDemo)
    {
        m_dirLightIntensity = 0.1f;
        
        constexpr float space = 3.0f;
        constexpr int row = 12;
        constexpr int column = 12;
    
        // AddModelToScene("Assets/cube.obj", "", "", { space * row/2, -0.5f, space * column/2 }, {}, { 25.0f, 0.2f, 25.0f });

        for(int i = 0; i < row; i++)
        {
            for(int j = 0; j < column; j++)
            {
                float posX = space * (float)i;
                float posZ = space * (float)j;

                if(i % 2 == 0)
                {
                    AddModelToScene("Dragon", "Assets/dragon.obj", "", "", "", { posX, 0.0f, posZ }, {}, { 0.25f, 0.25f, 0.25f });
                }
                else
                    AddLightToScene({ posX, 1.0f, posZ }, {}, true);
            }
        }
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

void CorvusEditor::Run()
{
    while(m_window->IsRunning())
    {
        InputSystem::Get()->Update();

        // ------------------------------------------------------------- Scene Constants Update --------------------------------------------------------------------
        
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

        // --------------------------------------------- Terrible temporary way to ECS data -> renderer -----------------------------------------------
        
        // TODO this is WIP while implementing basic ECS. Find a proper way to iterate over meshes
        for(const auto go : m_scene->m_gameObjects)
        {
            if(const auto meshComp = go->GetComponent<MeshComponent>())
                meshComp->GetRenderItem()->m_transforms.clear();
        }
        
        std::set<std::shared_ptr<RenderItem>> ri;
        std::vector<PointLight> pointLights;
        for(const auto go : m_scene->m_gameObjects)
        {
            if(const auto tfComp = go->GetComponent<TransformComponent>())
            {
                if(const auto meshComp = go->GetComponent<MeshComponent>())
                {
                    auto renderItem = meshComp->GetRenderItem();
                    renderItem->m_transforms.emplace_back(tfComp->m_transform);
                    ri.insert(renderItem);
                }

                if(const auto pointLightComp = go->GetComponent<PointLightComponent>())
                {
                    pointLightComp->m_pointLight.Position = { tfComp->m_transform.m[3][0], tfComp->m_transform.m[3][1], tfComp->m_transform.m[3][2] };
                    pointLights.emplace_back(pointLightComp->m_pointLight);
                }
            }
        }
        
        std::vector<std::shared_ptr<RenderItem>> renderItems;
        for(auto r : ri)
            renderItems.emplace_back(r);

        // ------------------------------------------------------------- Lights Update --------------------------------------------------------------------
        for(auto& pointLight : pointLights)
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
        passData.PointLights = m_enablePointLights ? pointLights : pl;
        passData.DirectionalInfo.Direction = { m_dirLightDirection[0], m_dirLightDirection[1], m_dirLightDirection[2] };
        passData.DirectionalInfo.Intensity = m_dirLightIntensity;

        // ------------------------------------------------------------- Render Passes --------------------------------------------------------------------

        auto commandList = m_renderer->GetCurrentCommandList();
        auto backbuffer = m_renderer->GetBackBuffer();
        
        commandList->Begin();
        commandList->ImageBarrier(backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandList->SetViewport(0, 0, width, height);
        commandList->BindRenderTargets({ backbuffer }, nullptr);
        commandList->ClearRenderTarget(backbuffer, 0.0f, 0.0f, 0.0f, 1.0f);

        m_deferredPass->Pass(m_renderer, passData, m_camera, renderItems);
        // m_transparencyPass->Pass(m_renderer, passData, m_camera, m_transparentRenderItems);

        // ------------------------------------------------------------- UI Rendering --------------------------------------------------------------------
        
        if(m_displayUI)
        {
            commandList->BindRenderTargets({ backbuffer }, nullptr);
            m_renderer->BeginImGuiFrame();
            RenderUI((float)width, (float)height);
            m_renderer->EndImGuiFrame();
        }
        
        commandList->ImageBarrier(backbuffer, D3D12_RESOURCE_STATE_PRESENT);
        commandList->End();
        m_renderer->ExecuteCommandBuffers({ commandList }, D3D12_COMMAND_LIST_TYPE_DIRECT);

        // m_renderer->WaitForGPU();

        m_renderer->Present(false);
        m_renderer->EndFrame();

        m_window->BroadCast();
    }
}

std::shared_ptr<RenderItem> CorvusEditor::AddModelToScene(std::string name, const std::string& modelPath, const std::string& albedoPath, const std::string& normalPath,
    const std::string& mrPath, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale, bool transparent)
{
    auto model = m_resourceManager->LoadMesh(modelPath);

    Uploader uploader = m_renderer->CreateUploader();
    Image albedoImg;
    Image normalImg;
    Image mrImg;

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

    if(!mrPath.empty())
    {
        auto mrTexture = m_resourceManager->LoadTexture(mrPath, uploader, mrImg);
        model->GetMaterial().HasMetallicRoughness = true;
        model->GetMaterial().MetallicRoughness = mrTexture;
    }

    if(uploader.HasCommands())
        m_renderer->FlushUploader(uploader);

    if(model->m_instancesDataBuffer == nullptr)
        model->m_instancesDataBuffer = m_renderer->CreateBuffer(sizeof(InstanceData) * MAX_INSTANCES, sizeof(InstanceData), BufferType::Structured, false);

    auto go = m_scene->CreateGameObject(name, position, rotation, scale);
    auto meshComp = go->AddComponent<MeshComponent>();
    meshComp->SetRenderItem(model);
    
    return model;
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
    
    auto go = m_scene->CreateGameObject("PointLight", position);
    auto lightComp = go->AddComponent<PointLightComponent>();
    lightComp->m_pointLight = pointLight;
}

void CorvusEditor::RenderUI(float width, float height)
{
    if(ImGui::BeginMainMenuBar())
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
    ImGui::SliderFloat("Move Speed", &m_moveSpeed, 1.0f, 40.0f);
    static const char* modes[] = { "Default", "Albedo", "Normal", "Depth", "WorldPosition", "MetallicRoughness" };
    ImGui::Combo("View Mode", (int*)&m_viewMode, modes, 6);
    ImGui::Separator();
    ImGui::SliderFloat3("DirLight Direction", m_dirLightDirection, -1.0f, 1.0f);
    ImGui::SliderFloat("DirLight Intensity", &m_dirLightIntensity, 0.0f, 5.0f);
    ImGui::End();

    ImGui::Begin("Debug Point Lights");
    ImGui::Checkbox("Enable Point Lights", &m_enablePointLights);
    ImGui::Checkbox("Move", &m_movePointLights);
    ImGui::Separator();
    ImGui::SliderFloat("MoveSpeed", &m_movePointLightsSpeed, 0.0f, 8.0f);
    ImGui::SliderFloat("Constant", &m_testLightConstAttenuation, 0.0f, 1.0f);
    ImGui::SliderFloat("Linear", &m_testLightLinearAttenuation, 0.0f, 0.5f);
    ImGui::SliderFloat("Quadratic", &m_testLightQuadraticAttenuation, 0.0f, 0.5f);
    ImGui::End();

    ImGui::Begin("SceneHierarchy");
    if(ImGui::BeginListBox("Objects"))
    {
        for(auto go : m_scene->m_gameObjects)
        {
            bool isSelected = false;
            if(m_selectedGo != nullptr)
            {
                if(m_selectedGo->GetName() == go->GetName())
                    isSelected = true;
            }
    
            if(ImGui::Selectable(go->GetName().c_str(), isSelected))
                m_selectedGo = go;
        }

        ImGui::EndListBox();
    }
    ImGui::Separator();
    if(m_selectedGo != nullptr)
    {
        ImGui::Text(m_selectedGo->GetName().c_str());
        if(auto tfComp = m_selectedGo->GetComponent<TransformComponent>())
        {
            float pos[3] = { tfComp->m_transform.m[3][0], tfComp->m_transform.m[3][1], tfComp->m_transform.m[3][2] };
            ImGui::InputFloat3("Position", pos);

            float scale[3] = { tfComp->m_transform.m[0][0], tfComp->m_transform.m[1][1], tfComp->m_transform.m[2][2] };
            ImGui::InputFloat3("Scale", scale);

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::BeginFrame();
            ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList()); 
            ImGuizmo::SetRect(0, 0, (float)width, (float)height);

            if (ImGui::RadioButton("Translate", m_gizmoOperation == ImGuizmo::TRANSLATE))
                m_gizmoOperation = ImGuizmo::TRANSLATE;
            ImGui::SameLine();
            if (ImGui::RadioButton("Rotate", m_gizmoOperation == ImGuizmo::ROTATE))
                m_gizmoOperation = ImGuizmo::ROTATE;
            ImGui::SameLine();
            if (ImGui::RadioButton("Scale", m_gizmoOperation == ImGuizmo::SCALE))
                m_gizmoOperation = ImGuizmo::SCALE;

            if (m_gizmoOperation != ImGuizmo::SCALE)
            {
                if (ImGui::RadioButton("Local", m_gizmoMode == ImGuizmo::LOCAL))
                    m_gizmoMode = ImGuizmo::LOCAL;
                ImGui::SameLine();
                if (ImGui::RadioButton("World", m_gizmoMode == ImGuizmo::WORLD))
                    m_gizmoMode = ImGuizmo::WORLD;
            }
            
            DirectX::XMFLOAT4X4 view;
            DirectX::XMStoreFloat4x4(&view, m_camera.GetViewMatrix());

            DirectX::XMFLOAT4X4 projection;
            DirectX::XMStoreFloat4x4(&projection, m_camera.GetProjMatrix());

            auto tfMat = DirectX::XMLoadFloat4x4(&tfComp->m_transform);
            DirectX::XMFLOAT4X4 tfCopy;
            DirectX::XMStoreFloat4x4(&tfCopy, tfMat);
            
            ImGuizmo::Manipulate(view.m[0], projection.m[0], m_gizmoOperation, m_gizmoMode, tfCopy.m[0]);

            if(ImGuizmo::IsUsing())
                DirectX::XMStoreFloat4x4(&tfComp->m_transform, DirectX::XMLoadFloat4x4(&tfCopy));
        }
    }
    ImGui::End();

    auto deferredPass = std::static_pointer_cast<DeferredRenderPass>(m_deferredPass);
    auto GBuffer = deferredPass->GetGBuffer();
    
    ImGui::Begin("Debug GBuffer");
    ImGui::Image((ImTextureID)GBuffer.AlbedoRenderTarget->m_srvUav.GPU.ptr, ImVec2(440, 220));
    ImGui::Image((ImTextureID)GBuffer.NormalRenderTarget->m_srvUav.GPU.ptr, ImVec2(440, 220));
    ImGui::Image((ImTextureID)GBuffer.WorldPositionRenderTarget->m_srvUav.GPU.ptr, ImVec2(440, 220));
    ImGui::Image((ImTextureID)GBuffer.MetallicRoughnessRenderTarget->m_srvUav.GPU.ptr, ImVec2(440, 220));
    ImGui::Image((ImTextureID)GBuffer.DepthBuffer->m_srvUav.GPU.ptr, ImVec2(440, 220));
    ImGui::End();
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
    else if(key == 'R')
        m_displayUI = !m_displayUI;
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
