#include "Object.h"

#include <d3d12.h>

using namespace DirectX;

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