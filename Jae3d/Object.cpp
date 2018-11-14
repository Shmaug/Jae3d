#include "Object.hpp"

#include <d3d12.h>

using namespace DirectX;
using namespace std;

Object::Object(std::string name) : m_Name(name) {}
Object::~Object() {}

bool Object::UpdateTransform() {
	if (!m_TransformDirty) return false;

	m_ObjectToWorld = XMMatrixTransformation({ 0,0,0,0 }, { 0,0,0,1 }, m_LocalScale, { 0,0,0,0 }, m_LocalRotation, m_LocalPosition);
	if (m_Parent) m_ObjectToWorld = XMMatrixMultiply(m_ObjectToWorld, m_Parent->ObjectToWorld());

	DirectX::XMVECTOR det = XMMatrixDeterminant(m_ObjectToWorld);
	m_WorldToObject = XMMatrixInverse(&det, m_ObjectToWorld);
	
	m_TransformDirty = false;
	return true;
}
void Object::Parent(shared_ptr<Object> p){
	if (m_Parent) {
		shared_ptr<Object> t = shared_from_this();
		for (int i = 0; i < m_Parent->m_Children.size(); i++) {
			shared_ptr<Object> c = m_Parent->m_Children[i].lock();
			if (c && c == t)
				m_Parent->m_Children.erase(m_Parent->m_Children.begin() + i);
		}
	}
	m_Parent = p;
	if (p) p->m_Children.push_back(weak_from_this());
	SetTransformsDirty();
}
void Object::SetTransformsDirty(){
	m_TransformDirty = true;
	for (size_t i = 0; i < m_Children.size(); i++) {
		shared_ptr<Object> o = m_Children[i].lock();
		if (o) o->SetTransformsDirty();
	}
}