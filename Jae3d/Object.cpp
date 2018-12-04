#include "Object.hpp"
#include <d3d12.h>

using namespace DirectX;
using namespace std;

Object::Object(jstring name) : mName(name) {}
Object::~Object() {}

bool Object::UpdateTransform() {
	if (!mTransformDirty) return false;

	mObjectToWorld = XMMatrixAffineTransformation(mLocalScale, XMVectorZero(), mLocalRotation, mLocalPosition);
	if (mParent) {
		mObjectToWorld = mObjectToWorld * mParent->ObjectToWorld();

		mWorldPosition = XMVector3Transform(mLocalPosition, mWorldToObject);
		mWorldRotation = mParent->WorldRotation() * mLocalRotation;
	}else{
		mWorldPosition = mLocalPosition;
		mWorldRotation = mLocalRotation;
	}

	DirectX::XMVECTOR det = XMMatrixDeterminant(mObjectToWorld);
	mWorldToObject = XMMatrixInverse(&det, mObjectToWorld);

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