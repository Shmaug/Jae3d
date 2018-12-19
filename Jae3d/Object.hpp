#pragma once

#include "Util.hpp"
#include "jstring.hpp"
#include "jvector.hpp"

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <d3d12.h>
#include <memory>

#define DX DirectX

class Scene;

class Object : public std::enable_shared_from_this<Object> {
protected:
	bool mTransformDirty = true;

public:
	JAE_API Object(jwstring name);
	JAE_API ~Object();

	jwstring mName;

	// Getters
	DX::XMFLOAT4X4 ObjectToWorld() { UpdateTransform(); return mObjectToWorld; }
	DX::XMFLOAT4X4 WorldToObject() { UpdateTransform(); return mWorldToObject; }
	DX::XMFLOAT3 LocalPosition() { UpdateTransform(); return mLocalPosition; }
	DX::XMFLOAT4 LocalRotation() { UpdateTransform(); return mLocalRotation; }
	DX::XMFLOAT3 LocalScale() const { return mLocalScale; }

	// World-space getters
	DX::XMFLOAT4 WorldRotation() { UpdateTransform(); return mWorldRotation; }
	DX::XMFLOAT3 WorldPosition() { UpdateTransform(); return mWorldPosition; }
	DX::XMFLOAT3 WorldScale() { UpdateTransform(); return mWorldScale; }

	// Setters
	void LocalPosition(DX::XMVECTOR p) { DX::XMStoreFloat3(&mLocalPosition, p); SetTransformsDirty(); }
	void LocalRotation(DX::XMVECTOR r) { DX::XMStoreFloat4(&mLocalRotation, r); SetTransformsDirty(); }
	void LocalScale(DirectX::XMVECTOR s) { DX::XMStoreFloat3(&mLocalScale, s); SetTransformsDirty(); }
	void LocalPosition(DX::XMFLOAT3 p) { mLocalPosition = p; SetTransformsDirty(); }
	void LocalRotation(DX::XMFLOAT4 r) { mLocalRotation = r; SetTransformsDirty(); }
	void LocalScale(DirectX::XMFLOAT3 s) { mLocalScale = s; SetTransformsDirty(); }
	void LocalPosition(float x, float y, float z) { LocalPosition(DX::XMFLOAT3(x, y, z)); }
	void LocalScale(float x, float y, float z) { LocalScale(DX::XMFLOAT3(x, y, z)); }

	JAE_API void Parent(std::shared_ptr<Object> p);
	std::shared_ptr<Object> Parent() { return mParent; }

	JAE_API void SetScene(Scene* scene);
	JAE_API virtual bool UpdateTransform();

	virtual DirectX::BoundingOrientedBox Bounds() { return DirectX::BoundingOrientedBox(WorldPosition(), DirectX::XMFLOAT3(0, 0, 0), WorldRotation()); }

private:
	DX::XMFLOAT3 mLocalPosition = DX::XMFLOAT3(0, 0, 0);
	DX::XMFLOAT4 mLocalRotation = DX::XMFLOAT4(0, 0, 0, 1);
	DX::XMFLOAT3 mLocalScale    = DX::XMFLOAT3(1, 1, 1);

	DX::XMFLOAT3 mWorldPosition = DX::XMFLOAT3(0, 0, 0);
	DX::XMFLOAT4 mWorldRotation = DX::XMFLOAT4(0, 0, 0, 1);
	DX::XMFLOAT3 mWorldScale    = DX::XMFLOAT3(1, 1, 1);

	DX::XMFLOAT4X4 mObjectToWorld;
	DX::XMFLOAT4X4 mWorldToObject;

	std::shared_ptr<Object> mParent;
	jvector<std::weak_ptr<Object>> mChildren;

	friend class Scene;
	Scene* mScene;

	JAE_API void SetTransformsDirty();
};

