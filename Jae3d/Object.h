#pragma once

#include <string>
#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>

class Object {
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
	void LocalPosition(DirectX::XMVECTOR p) { m_LocalPosition = p; SetDirty(); }
	void LocalRotation(DirectX::XMVECTOR r) { m_LocalRotation = r; SetDirty(); }
	void LocalScale(DirectX::XMVECTOR s) { m_LocalScale = s; SetDirty(); }

	void Parent(Object* p) {
		if (m_Parent)
			for (int i = 0; i < m_Parent->m_Children.size(); i++)
				if (m_Parent->m_Children[i] == this)
					m_Parent->m_Children.erase(m_Parent->m_Children.begin() + i);
		m_Parent = p;
		if (p) p->m_Children.push_back(this);
		SetDirty();
	}
	Object* Parent() { return m_Parent; }

private:
	DirectX::XMVECTOR m_WorldPosition = { 0, 0, 0, 0 };
	DirectX::XMVECTOR m_WorldRotation = { 0, 0, 0, 1 };
	DirectX::XMVECTOR m_LocalPosition = { 0, 0, 0, 0 };
	DirectX::XMVECTOR m_LocalRotation = { 0, 0, 0, 1 };
	DirectX::XMVECTOR m_LocalScale    = { 1, 1, 1, 1 };

	DirectX::XMMATRIX m_ObjectToWorld;
	DirectX::XMMATRIX m_WorldToObject;

	Object* m_Parent;
	std::vector<Object*> m_Children;

	void SetDirty() {
		m_TransformDirty = true;
		for (size_t i = 0; i < m_Children.size(); i++)
			m_Children[i]->SetDirty();
	}
};

