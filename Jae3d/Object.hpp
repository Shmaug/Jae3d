#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include <memory>

#define DX DirectX

class Object : public std::enable_shared_from_this<Object> {
protected:
	virtual bool UpdateTransform();
	bool m_TransformDirty = true;

public:
	Object(std::string name);
	~Object();

	std::string m_Name;

	// Getters
	DX::XMMATRIX ObjectToWorld() { UpdateTransform(); return m_ObjectToWorld; }
	DX::XMMATRIX WorldToObject() { UpdateTransform(); return m_WorldToObject; }
	DX::XMVECTOR LocalPosition() { UpdateTransform(); return m_LocalPosition; }
	DX::XMVECTOR LocalRotation() { UpdateTransform(); return m_LocalRotation; }
	DX::XMVECTOR LocalScale() const { return m_LocalScale; }
	// World-space getters
	DX::XMVECTOR WorldRotation() { UpdateTransform(); return m_WorldRotation; }
	DX::XMVECTOR WorldPosition() { UpdateTransform(); return m_WorldPosition; }

	// Setters
	void LocalPosition(DX::XMVECTOR p) { m_LocalPosition = p; SetTransformsDirty(); }
	void LocalRotation(DX::XMVECTOR r) { m_LocalRotation = r; SetTransformsDirty(); }
	void LocalScale(DirectX::XMVECTOR s) { m_LocalScale = s; SetTransformsDirty(); }
	void LocalPosition(float x, float y, float z) { LocalPosition(DX::XMVectorSet(x, y, z, 0)); }
	void LocalScale(float x, float y, float z) { LocalScale(DX::XMVectorSet(x, y, z, 0)); }

	void Parent(std::shared_ptr<Object> p);
	std::shared_ptr<Object> Parent() { return m_Parent; }

private:
	DX::XMVECTOR m_WorldPosition = DX::XMVectorSet(0, 0, 0, 0);
	DX::XMVECTOR m_WorldRotation = DX::XMVectorSet(0, 0, 0, 1);

	DX::XMVECTOR m_LocalPosition = DX::XMVectorSet(0, 0, 0, 0);
	DX::XMVECTOR m_LocalRotation = DX::XMVectorSet(0, 0, 0, 1);
	DX::XMVECTOR m_LocalScale    = DX::XMVectorSet(1, 1, 1, 1);

	DX::XMMATRIX m_ObjectToWorld;
	DX::XMMATRIX m_WorldToObject;

	std::shared_ptr<Object> m_Parent;
	std::vector<std::weak_ptr<Object>> m_Children;

	void SetTransformsDirty();
};

