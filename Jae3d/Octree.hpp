#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "jvector.hpp"
#include <memory>

template<typename T>
class Octree {
public:
	Octree(float size) {
		mRootNode = new Node();
		mRootNode->mBounds.Extents.x = size;
		mRootNode->mBounds.Extents.y = size;
		mRootNode->mBounds.Extents.z = size;
	}
	~Octree() {
		delete mRootNode;
	}

	void GetLeaves(jvector<DirectX::BoundingBox> &leaves, DirectX::BoundingFrustum &frustum) const {
		mRootNode->GetLeaves(leaves, frustum);
	}
	void GetBoxes(jvector<DirectX::BoundingOrientedBox> &boxes, DirectX::BoundingFrustum &frustum) const {
		mRootNode->GetBoxes(boxes, frustum);
	}

	void Insert(T* obj){
		mRootNode->Insert(OctreeObject(obj));
	}
	void Remove(T* obj, DirectX::BoundingOrientedBox &box){
		mRootNode->Remove(obj, box);
	}

	void Move(T* obj, DirectX::BoundingOrientedBox &from) {
		mRootNode->Remove(obj, from, false);
		mRootNode->Insert(OctreeObject(obj), false);
		mRootNode->Validate();
	}

	template<typename B>
	void Test(jvector<T*> &hits, B &bounds) const { return mRootNode->Test(hits, bounds); }

private:
	struct OctreeObject {
		T* object;
		DirectX::BoundingOrientedBox bounds;

		OctreeObject(T* obj) : object(obj), bounds(obj->Bounds()) {}
		OctreeObject(const OctreeObject &obj) : object(obj.object), bounds(obj.bounds) {}
	};
	struct Node {
		static const unsigned int MAX_OBJECTS_PER_NODE = 8;
		static const unsigned int MAX_LEVEL = 8;

		DirectX::BoundingBox mBounds;
		Node* mChildren;
		jvector<OctreeObject> mObjects;
		unsigned int mLevel;

		Node() : mChildren(nullptr), mLevel(0), mObjects() {};
		~Node() { mObjects.free(); if (IsSplit()) delete[] mChildren; }

		bool IsSplit() const { return mChildren != nullptr; }
		void Split() {
			if (IsSplit() || mLevel == MAX_LEVEL) return;

			mChildren = new Node[8];
			DirectX::XMFLOAT3 corners[DirectX::BoundingBox::CORNER_COUNT];
			mBounds.GetCorners(corners);
			for (unsigned int i = 0; i < 8; i++) {
				mChildren[i].mLevel = mLevel + 1;
				mChildren[i].mBounds.Center.x = (corners[i].x + mBounds.Center.x) * .5f;
				mChildren[i].mBounds.Center.y = (corners[i].y + mBounds.Center.y) * .5f;
				mChildren[i].mBounds.Center.z = (corners[i].z + mBounds.Center.z) * .5f;
				mChildren[i].mBounds.Extents.x = mBounds.Extents.x * .5f;
				mChildren[i].mBounds.Extents.y = mBounds.Extents.y * .5f;
				mChildren[i].mBounds.Extents.z = mBounds.Extents.z * .5f;

				for (unsigned int j = 0; j < mObjects.size(); j++)
					mChildren[i].Insert(mObjects[j]);
			}

			mObjects.free();
		}
		void Join() {
			if (!IsSplit()) return;

			size_t c = 0;
			for (int i = 0; i < 8; i++)
				c += mChildren[i].mObjects.size();

			mObjects.reserve(c);
			for (unsigned int i = 0; i < 8; i++)
				for (unsigned int j = 0; j < mChildren[i].mObjects.size(); j++)
					mObjects.push_back(mChildren[i].mObjects[j]);

			delete[] mChildren;
			mChildren = nullptr;
		}
		
		unsigned int ContainedCount() const {
			if (IsSplit()) {
				unsigned int c = 0;
				for (unsigned int i = 0; i < 8; i++)
					c += mChildren[i].ContainedCount();
				return c;
			} else {
				DirectX::BoundingBox beps;
				beps.Center = mBounds.Center;
				beps.Extents.x = mBounds.Extents.x - .001f;
				beps.Extents.y = mBounds.Extents.y - .001f;
				beps.Extents.z = mBounds.Extents.z - .001f;
				unsigned int c = 0;
				for (unsigned int i = 0; i < mObjects.size(); i++)
					if (beps.Contains(XMLoadFloat3(&mObjects[i].bounds.Center)) == DirectX::ContainmentType::CONTAINS)
						c++;
				return c;
			}
		}

		void Insert(OctreeObject &obj, bool split = true) {
			if (!mBounds.Intersects(obj.bounds)) return;

			if (IsSplit())
				for (unsigned int i = 0; i < 8; i++)
					mChildren[i].Insert(obj, split);
			else {
				mObjects.push_back(obj);
				if (split && ContainedCount() > MAX_OBJECTS_PER_NODE)
					Split();
			}
		}
		void Remove(T* obj, DirectX::BoundingOrientedBox &box, bool join = true) {
			if (!mBounds.Intersects(box)) return;

			if (IsSplit()) {
				for (unsigned int i = 0; i < 8; i++)
					mChildren[i].Remove(obj, box, join);
				if (join && ContainedCount() <= MAX_OBJECTS_PER_NODE)
					Join();
			} else {
				for (unsigned int i = 0; i < mObjects.size(); i++)
					if (mObjects[i].object == obj)
						mObjects.remove(i);
			}
		}
		void Validate() {
			if (IsSplit()) {
				if (ContainedCount() <= MAX_OBJECTS_PER_NODE)
					Join();
				else
					for (unsigned int i = 0; i < 8; i++)
						mChildren[i].Validate();
			} else {
				if (ContainedCount() > MAX_OBJECTS_PER_NODE && !IsSplit())
					Split();
			}
		}

		template<typename B>
		void Test(jvector<T*> &hits, B &bounds) const {
			if (!mBounds.Intersects(bounds)) return;
			if (IsSplit())
				for (unsigned int i = 0; i < 8; i++)
					mChildren[i].Test(hits, bounds);
			else
				for (unsigned int i = 0; i < mObjects.size(); i++) {
					if (mObjects[i].bounds.Intersects(bounds) && hits.find(mObjects[i].object) == -1)
						hits.push_back(mObjects[i].object);
				}
		}

		void GetBoxes(jvector<DirectX::BoundingOrientedBox> &boxes, DirectX::BoundingFrustum &frustum) const {
			if (IsSplit())
				for (unsigned int i = 0; i < 8; i++)
					mChildren[i].GetBoxes(boxes, frustum);
			else
				for (unsigned int i = 0; i < mObjects.size(); i++)
					if (frustum.Intersects(mObjects[i].bounds))
						boxes.push_back(mObjects[i].bounds);
		}
		void GetLeaves(jvector<DirectX::BoundingBox> &leaves, DirectX::BoundingFrustum &frustum) const {
			if (IsSplit())
				for (unsigned int i = 0; i < 8; i++)
					mChildren[i].GetLeaves(leaves, frustum);
			else if (frustum.Intersects(mBounds))
				leaves.push_back(mBounds);
		}
	};

	Node* mRootNode;
};