#include "Object.h"

#include <d3d12.h>

using namespace DirectX;

Object::Object(std::string name) : m_Name(name) {}
Object::~Object() {}

void Object::UpdateTransform() {
	m_ObjectToWorld = XMMatrixTransformation({ 0,0,0,0 }, { 0,0,0,1 }, m_Scale, { 0,0,0,0 }, m_Rotation, m_Position);

	DirectX::XMVECTOR det = XMMatrixDeterminant(m_ObjectToWorld);
	m_WorldToObject = XMMatrixInverse(&det, m_ObjectToWorld);

	m_TransformDirty = false;
}