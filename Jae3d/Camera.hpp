#pragma once

#include "Object.hpp"
#include "d3dx12.hpp"

#include <wrl.h>
#define _WRL Microsoft::WRL

class Camera : public Object {
protected:
	bool UpdateTransform();

public:
	Camera(std::string name);
	~Camera();

	float FieldOfView() const { return m_FieldOfView; }
	float Aspect() const { return m_Aspect; }
	float Near() const { return m_Near; }
	float Far() const { return m_Far; }
	void FieldOfView(float f) { m_FieldOfView = f; m_TransformDirty = true; }
	void Aspect(float a) { m_Aspect = a; m_TransformDirty = true; }
	void Near(float n) { m_Near = n; m_TransformDirty = true; }
	void Far(float f) { m_Far = f; m_TransformDirty = true; }
	D3D12_GPU_VIRTUAL_ADDRESS GetCBuffer() { if (m_TransformDirty) UpdateTransform(); return m_CBuffer->GetGPUVirtualAddress(); }

	DirectX::XMMATRIX View() { if (m_TransformDirty) UpdateTransform(); return m_View; }
	DirectX::XMMATRIX Projection() { if (m_TransformDirty) UpdateTransform(); return m_Projection; }
	DirectX::XMMATRIX ViewProjection() { if (m_TransformDirty) UpdateTransform(); return m_ViewProjection; }

	void CreateCB();
	void ReleaseCB();
private:
	float m_FieldOfView = 70.0f;
	float m_Aspect = 1.0f;
	float m_Near = .01f;
	float m_Far = 1000.0f;

	DirectX::XMMATRIX m_View;
	DirectX::XMMATRIX m_Projection;
	DirectX::XMMATRIX m_ViewProjection;

	struct CameraBuffer {
	public:
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 ViewProjection;
		DirectX::XMFLOAT3 CameraPosition;
	} m_CameraBufferData;
	_WRL::ComPtr<ID3D12Resource> m_CBuffer;
	UINT8* m_MappedCBuffer;
};

