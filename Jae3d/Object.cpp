#include "Object.hpp"
#include "Scene.hpp"
#include <d3d12.h>

using namespace DirectX;
using namespace std;

Object::Object(jwstring name) : mName(name), mScene(nullptr) {}
Object::~Object() {}

bool Object::UpdateTransform() {
	if (!mTransformDirty) return false;

	XMVECTOR localScale = XMLoadFloat3(&mLocalScale);
	XMVECTOR localPos = XMLoadFloat3(&mLocalPosition);
	XMVECTOR localRot = XMLoadFloat4(&mLocalRotation);

	XMMATRIX o2w = XMMatrixAffineTransformation(localScale, XMVectorZero(), localRot, localPos);
	if (mParent) {
		o2w = o2w * XMLoadFloat4x4(&mParent->ObjectToWorld());

		XMStoreFloat3(&mWorldPosition, XMVector3Transform(localPos, o2w));
		XMStoreFloat4(&mWorldRotation, XMQuaternionMultiply(XMLoadFloat4(&mParent->WorldRotation()), localRot));
		mWorldScale.x = mParent->mWorldScale.x * mLocalScale.x;
		mWorldScale.y = mParent->mWorldScale.y * mLocalScale.y;
		mWorldScale.z = mParent->mWorldScale.z * mLocalScale.z;
	}else{
		mWorldPosition = mLocalPosition;
		mWorldRotation = mLocalRotation;
		mWorldScale = mLocalScale;
	}

	XMStoreFloat4x4(&mObjectToWorld, o2w);
	XMStoreFloat4x4(&mWorldToObject, XMMatrixInverse(&XMMatrixDeterminant(o2w), o2w));

	mTransformDirty = false;
	return true;
}
void Object::Parent(shared_ptr<Object> p){
	if (mParent) {
		shared_ptr<Object> t = shared_from_this();
		for (int i = 0; i < mParent->mChildren.size(); i++) {
			shared_ptr<Object> c = mParent->mChildren[i].lock();
			if (c && c == t)
				mParent->mChildren.remove(i);
		}
	}
	mParent = p;
	if (p) p->mChildren.push_back(weak_from_this());
	SetTransformsDirty();
}
void Object::SetTransformsDirty(){
	mTransformDirty = true;
	for (size_t i = 0; i < mChildren.size(); i++) {
		shared_ptr<Object> o = mChildren[i].lock();
		if (o) o->SetTransformsDirty();
	}
}

void Object::SetScene(Scene* scene) {
	if (scene == mScene) return;
	if (mScene) mScene->RemoveObject(shared_from_this());
	mScene = scene;
	mScene->AddObject(shared_from_this());
}