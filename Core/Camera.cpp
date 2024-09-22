#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::UpdatePerspectiveFOV(float FovY, float aspectRatio)
{
    XMMATRIX P = XMMatrixPerspectiveFovLH(FovY, aspectRatio, 1.0f, 1000.0f);
    XMStoreFloat4x4(&m_proj, P);
}

void Camera::Walk(float d)
{
    // pos += d * look
    XMVECTOR s = XMVectorReplicate(d);
    XMVECTOR l = XMLoadFloat3(&m_look);
    XMVECTOR p = XMLoadFloat3(&m_position);
    XMStoreFloat3(&m_position, XMVectorMultiplyAdd(s, l, p));
}

void Camera::Strafe(float d)
{
    // pos += d * right
    XMVECTOR s = XMVectorReplicate(d);
    XMVECTOR r = XMLoadFloat3(&m_right);
    XMVECTOR p = XMLoadFloat3(&m_position);
    XMStoreFloat3(&m_position, XMVectorMultiplyAdd(s, r, p));
}

void Camera::Pitch(float angle)
{
    // rotate up and look vector about the right vector
    XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_right), angle);
    XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), R));
    XMStoreFloat3(&m_look, XMVector3TransformNormal(XMLoadFloat3(&m_look), R));
}

void Camera::RotateY(float angle)
{
    // rotate the basis vectors about the world y-axis
    XMMATRIX R = XMMatrixRotationY(angle);
    XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), R));
    XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), R));
    XMStoreFloat3(&m_look, XMVector3TransformNormal(XMLoadFloat3(&m_look), R));
}

void Camera::UpdateViewMatrix()
{
    XMVECTOR R = XMLoadFloat3(&m_right);
    XMVECTOR U = XMLoadFloat3(&m_up);
    XMVECTOR L = XMLoadFloat3(&m_look);
    XMVECTOR P = XMLoadFloat3(&m_position);

    L = XMVector3Normalize(L);
    U = XMVector3Normalize(XMVector3Cross(L, R));

    R = XMVector3Cross(U, L);

    float x = -XMVectorGetX(XMVector3Dot(P, R));
    float y = -XMVectorGetX(XMVector3Dot(P, U));
    float z = -XMVectorGetX(XMVector3Dot(P, L));

    XMStoreFloat3(&m_right, R);
    XMStoreFloat3(&m_up, U);
    XMStoreFloat3(&m_look, L);

    m_view(0, 0) = m_right.x;
    m_view(1, 0) = m_right.y;
    m_view(2, 0) = m_right.z;
    m_view(3, 0) = x;

    m_view(0, 1) = m_up.x;
    m_view(1, 1) = m_up.y;
    m_view(2, 1) = m_up.z;
    m_view(3, 1) = y;

    m_view(0, 2) = m_look.x;
    m_view(1, 2) = m_look.y;
    m_view(2, 2) = m_look.z;
    m_view(3, 2) = z;
    
    m_view(0, 3) = 0.0f;
    m_view(1, 3) = 0.0f;
    m_view(2, 3) = 0.0f;
    m_view(3, 3) = 1.0f;
}

void Camera::UpdateInvViewProjMatrix(float width, float height)
{
    XMMATRIX view = GetViewMatrix();
    XMMATRIX proj = GetProjMatrix();
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    XMVECTOR viewProjDet = XMMatrixDeterminant(viewProj);
    XMStoreFloat4x4(&m_invViewProj, XMMatrixInverse(&viewProjDet, viewProj));
}
