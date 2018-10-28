#include "Camera.h"

#include <DirectXMath.h>
#include "Graphics.h"

using namespace DirectX;

Camera::Camera(std::string name) : Object(name) {}

void Camera::UpdateTransform(){
	Object::UpdateTransform();

	XMVECTOR fwd = { 0, 0, -1, 0 };
	XMVECTOR up = { 0, 1, 0, 0 };
	fwd = XMVector3Rotate(fwd, Rotation());
	up = XMVector3Rotate(up, Rotation());
	m_View = XMMatrixLookAtLH(Position(), Position() + fwd, up);
	m_Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FieldOfView), m_Aspect, m_Near, m_Far);
	m_ViewProjection = m_View * m_Projection;
}