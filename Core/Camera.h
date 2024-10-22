﻿#pragma once
#include "Core.h"

class Camera
{
public:
    Camera();
    ~Camera();

    void UpdatePerspectiveFOV(float FovY, float aspectRatio);

    void Walk(float d);
    void Strafe(float d);

    void Pitch(float angle);
    void RotateY(float angle);

    void UpdateViewMatrix();
    void UpdateInvViewProjMatrix(float width, float height);

    DirectX::XMMATRIX GetViewMatrix() const { return XMLoadFloat4x4(&m_view); }
    DirectX::XMMATRIX GetProjMatrix() const { return XMLoadFloat4x4(&m_proj); }
    DirectX::XMFLOAT3 GetPosition() const { return m_position; }
    DirectX::XMMATRIX GetInvViewProjMatrix() const { return XMLoadFloat4x4(&m_invViewProj); }

private:
    DirectX::XMFLOAT4X4 m_view;
    DirectX::XMFLOAT4X4 m_proj;
    DirectX::XMFLOAT4X4 m_invViewProj;

    DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, -8.0f } ;
    DirectX::XMFLOAT3 m_right = { 1.0f, 0.0f, 0.0f } ;
    DirectX::XMFLOAT3 m_up = { 0.0f, 1.0f, 0.0f } ;
    DirectX::XMFLOAT3 m_look = { 0.0f, 0.0f, 1.0f } ;
};
