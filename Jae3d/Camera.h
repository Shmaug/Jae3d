#pragma once

#include "Object.h"
#include "d3dx12.h"

class Camera : public Object {
protected:
	void UpdateTransform();

public:
	Camera(std::string name);

	float FieldOfView() const { return m_FieldOfView; }
	float Aspect() const { return m_Aspect; }
	float Near() const { return m_Near; }
	float Far() const { return m_Far; }
	void FieldOfView(float f) { m_FieldOfView = f; m_TransformDirty = true; }
	void Aspect(float a) { m_Aspect = a; m_TransformDirty = true; }
	void Near(float n) { m_Near = n; m_TransformDirty = true; }
	void Far(float f) { m_Far = f; m_TransformDirty = true; }

	DirectX::XMMATRIX View() { if (m_TransformDirty) UpdateTransform(); return m_View; }
	DirectX::XMMATRIX Projection() { if (m_TransformDirty) UpdateTransform(); return m_Projection; }
	DirectX::XMMATRIX ViewProjection() { if (m_TransformDirty) UpdateTransform(); return m_ViewProjection; }

private:
	float m_FieldOfView = 70.0f;
	float m_Aspect = 1.0f;
	float m_Near = .01f;
	float m_Far = 1000.0f;

	DirectX::XMMATRIX m_View;
	DirectX::XMMATRIX m_Projection;
	DirectX::XMMATRIX m_ViewProjection;
};

