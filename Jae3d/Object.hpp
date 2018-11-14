#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include <memory>

class Object : public std::enable_shared_from_this<Object> {
protected:
	virtual bool UpdateTransform();
	bool m_TransformDirty = true;

public:
	Object(std::string name);
	~Object();

	std::string m_Name;

	DirectX::XMMATRIX ObjectToWorld() { UpdateTransform(); return m_ObjectToWorld; }
	DirectX::XMMATRIX WorldToObject() { UpdateTransform(); return m_WorldToObject; }
	DirectX::XMVECTOR LocalPosition() { UpdateTransform(); return m_LocalPosition; }
	DirectX::XMVECTOR LocalRotation() { UpdateTransform(); return m_LocalRotation; }
	DirectX::XMVECTOR LocalScale() const { return m_LocalScale; }
	void LocalPosition(DirectX::XMVECTOR p) { m_LocalPosition = p; SetTransformsDirty(); }
	void LocalRotation(DirectX::XMVECTOR r) { m_LocalRotation = r; SetTransformsDirty(); }
	void LocalScale(DirectX::XMVECTOR s) { m_LocalScale = s; SetTransformsDirty(); }

	void Parent(std::shared_ptr<Object> p);
	std::shared_ptr<Object> Parent() { return m_Parent; }

private:
	DirectX::XMVECTOR m_WorldPosition = { 0, 0, 0, 0 };
	DirectX::XMVECTOR m_WorldRotation = { 0, 0, 0, 1 };
	DirectX::XMVECTOR m_LocalPosition = { 0, 0, 0, 0 };
	DirectX::XMVECTOR m_LocalRotation = { 0, 0, 0, 1 };
	DirectX::XMVECTOR m_LocalScale    = { 1, 1, 1, 1 };

	DirectX::XMMATRIX m_ObjectToWorld;
	DirectX::XMMATRIX m_WorldToObject;

	std::shared_ptr<Object> m_Parent;
	std::vector<std::weak_ptr<Object>> m_Children;

	void SetTransformsDirty();
};

