#pragma once

#include "Util.hpp"
#include "../Common/jstring.hpp"
#include "../Common/jvector.hpp"

#include <DirectXMath.h>
#include <d3d12.h>
#include <memory>

#define DX DirectX

class Object : public std::enable_shared_from_this<Object> {
protected:
	virtual bool UpdateTransform();
	bool mTransformDirty = true;

public:
	Object(jstring name);
	~Object();

	jstring mName;

	// Getters
	DX::XMMATRIX ObjectToWorld() { UpdateTransform(); return mObjectToWorld; }
	DX::XMMATRIX WorldToObject() { UpdateTransform(); return mWorldToObject; }
	DX::XMVECTOR LocalPosition() { UpdateTransform(); return mLocalPosition; }
	DX::XMVECTOR LocalRotation() { UpdateTransform(); return mLocalRotation; }
	DX::XMVECTOR LocalScale() const { return mLocalScale; }
	// World-space getters
	DX::XMVECTOR WorldRotation() { UpdateTransform(); return mWorldRotation; }
	DX::XMVECTOR WorldPosition() { UpdateTransform(); return mWorldPosition; }

	// Setters
	void LocalPosition(DX::XMVECTOR p) { mLocalPosition = p; SetTransformsDirty(); }
	void LocalRotation(DX::XMVECTOR r) { mLocalRotation = r; SetTransformsDirty(); }
	void LocalScale(DirectX::XMVECTOR s) { mLocalScale = s; SetTransformsDirty(); }
	void LocalPosition(float x, float y, float z) { LocalPosition(DX::XMVectorSet(x, y, z, 0)); }
	void LocalScale(float x, float y, float z) { LocalScale(DX::XMVectorSet(x, y, z, 0)); }

	void Parent(std::shared_ptr<Object> p);
	std::shared_ptr<Object> Parent() { return mParent; }

private:
	DX::XMVECTOR mWorldPosition = DX::XMVectorSet(0, 0, 0, 0);
	DX::XMVECTOR mWorldRotation = DX::XMVectorSet(0, 0, 0, 1);

	DX::XMVECTOR mLocalPosition = DX::XMVectorSet(0, 0, 0, 0);
	DX::XMVECTOR mLocalRotation = DX::XMVectorSet(0, 0, 0, 1);
	DX::XMVECTOR mLocalScale    = DX::XMVectorSet(1, 1, 1, 1);

	DX::XMMATRIX mObjectToWorld;
	DX::XMMATRIX mWorldToObject;

	std::shared_ptr<Object> mParent;
	jvector<std::weak_ptr<Object>> mChildren;

	void SetTransformsDirty();
};

