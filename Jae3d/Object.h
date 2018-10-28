#pragma once

#include <string>
#include <DirectXMath.h>
#include <d3d12.h>

class Object {
protected:
	virtual void UpdateTransform();
	bool m_TransformDirty = true;

public:
	Object(std::string name);
	~Object();

	std::string m_Name;

	DirectX::XMMATRIX ObjectToWorld() { if (m_TransformDirty) UpdateTransform(); return m_ObjectToWorld; }
	DirectX::XMMATRIX WorldToObject() { if (m_TransformDirty) UpdateTransform(); return m_WorldToObject; }
	DirectX::XMVECTOR Position() const { return m_Position; }
	DirectX::XMVECTOR Rotation() const { return m_Rotation; }
	void Position(DirectX::XMVECTOR p) { m_Position = p; m_TransformDirty = true; }
	void Rotation(DirectX::XMVECTOR r) { m_Rotation = r; m_TransformDirty = true; }
	void Scale(DirectX::XMVECTOR s) { m_Scale = s; m_TransformDirty = true; }

private:
	DirectX::XMVECTOR m_Position = { 0, 0, 0, 0 };
	DirectX::XMVECTOR m_Rotation = { 0, 0, 0, 1 };
	DirectX::XMVECTOR m_Scale = { 1, 1, 1, 1 };

	DirectX::XMMATRIX m_ObjectToWorld;
	DirectX::XMMATRIX m_WorldToObject;
};

