#pragma once

#include <DirectXMath.h>
#include "d3dx12.h"

class Camera {
public:
	struct CameraCB {
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Projection;
		DirectX::XMMATRIX ViewProjection;
		DirectX::XMVECTOR CameraPosition;
	};

	DirectX::XMVECTOR m_Position;
	DirectX::XMVECTOR m_Rotation;
	float m_FieldOfView = 70.0f;
	float m_Aspect = 1.0f;
	float m_Near = .01f;
	float m_Far = 1000.0f;

	Camera();

	DirectX::XMMATRIX View() const;
	DirectX::XMMATRIX Projection() const;
	DirectX::XMMATRIX ViewProjection() const;
	CameraCB* GetData();

private:
	CameraCB m_Data;
};

