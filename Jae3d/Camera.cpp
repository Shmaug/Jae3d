#include "Camera.hpp"

#include <DirectXMath.h>
#include "Graphics.hpp"
#include "Util.hpp"

using namespace DirectX;
using namespace Microsoft::WRL;

Camera::Camera(std::string name) : Object(name) {}
Camera::~Camera() { ReleaseCB(); }

bool Camera::SetActive(ComPtr<ID3D12GraphicsCommandList2> commandList){
	commandList->SetGraphicsRootConstantBufferView(1, GetCBuffer());
	return true;
}

bool Camera::UpdateTransform(){
	if (!Object::UpdateTransform()) return false;

	XMVECTOR fwd = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), WorldRotation());
	XMVECTOR up = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), WorldRotation());
	m_View = XMMatrixLookToLH(WorldPosition(), fwd, up);
	m_Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FieldOfView), m_Aspect, m_Near, m_Far);
	m_ViewProjection = m_View * m_Projection;

	XMStoreFloat4x4(&m_CameraBufferData.View, m_View);
	XMStoreFloat4x4(&m_CameraBufferData.Projection, m_Projection);
	XMStoreFloat4x4(&m_CameraBufferData.ViewProjection, m_ViewProjection);
	XMStoreFloat3(&m_CameraBufferData.CameraPosition, WorldPosition());
	memcpy(m_MappedCBuffer, &m_CameraBufferData, sizeof(CameraBuffer));

	return true;
}

void Camera::CreateCB() {
	ComPtr<ID3D12Device> device = Graphics::GetDevice();

	ZeroMemory(&m_CameraBufferData, sizeof(m_CameraBufferData));

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(AlignUp(sizeof(CameraBuffer), 256)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_CBuffer)));

	m_CBuffer->SetName(L"CB Camera");

	CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_CBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_MappedCBuffer)));
	ZeroMemory(m_MappedCBuffer, (UINT)AlignUp(sizeof(CameraBuffer), 256));
}
void Camera::ReleaseCB() {
	CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
	m_CBuffer->Unmap(0, &readRange);
	m_CBuffer.Reset();
}