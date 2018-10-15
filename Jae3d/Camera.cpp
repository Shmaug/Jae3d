#include "Camera.h"

#include <DirectXMath.h>
#include "Graphics.h"

using namespace DirectX;

Camera::Camera() {
	m_Rotation = XMQuaternionIdentity();
}
Camera::CameraCB* Camera::GetData() {
	m_Data.View = View();
	m_Data.Projection = Projection();
	m_Data.ViewProjection = m_Data.View * m_Data.Projection;
	m_Data.CameraPosition = m_Position;
	return &m_Data;
}
XMMATRIX Camera::View() const {
	XMVECTOR fwd = { 0, 0, -1, 0 };
	XMVECTOR up = { 0, 1, 0, 0 };
	fwd = XMVector3Rotate(fwd, m_Rotation);
	up = XMVector3Rotate(up, m_Rotation);
	return XMMatrixLookAtLH(m_Position, m_Position + fwd, up);
}
XMMATRIX Camera::Projection() const {
	return XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FieldOfView), m_Aspect, m_Near, m_Far);
}
XMMATRIX Camera::ViewProjection() const {
	return View() * Projection();
}