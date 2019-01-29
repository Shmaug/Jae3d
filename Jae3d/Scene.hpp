#pragma once

#include <memory>
#include <type_traits>

#include "Util.hpp"
#include "jvector.hpp"
#include "jstring.hpp"

#include "Renderer.hpp"
#include "Light.hpp"
#include "Octree.hpp"

class CommandList;
class Shader;
class Mesh;

class Scene : std::enable_shared_from_this<Scene> {
public:
	JAE_API Scene();
	JAE_API ~Scene();

	JAE_API void DrawSkybox(std::shared_ptr<CommandList> commandList);
	// Draws the scene, according to each renderer's RenderQueue()
	JAE_API void Draw(std::shared_ptr<CommandList> commandList, std::shared_ptr<Camera> camera, std::shared_ptr<Material> materialOverride = nullptr);
	JAE_API void DebugDraw(std::shared_ptr<CommandList> commandList, std::shared_ptr<Camera> camera);
	JAE_API void CollectLights(const DirectX::BoundingFrustum &frustum, jvector<Light*> &lights);

	JAE_API void IntersectPoint(DirectX::XMVECTOR point, jvector<std::shared_ptr<Object>>& result) const;
	JAE_API void IntersectPoint(DirectX::XMFLOAT3 point, jvector<std::shared_ptr<Object>>& result) const;
	JAE_API void IntersectBounds(DirectX::BoundingOrientedBox bounds, jvector<std::shared_ptr<Object>>& result) const;

	// Creates an object in this scene
	template<class T>
	std::shared_ptr<T> AddObject(jwstring name) {
		static_assert(std::is_base_of<Object, T>::value, "T must be an Object!");

		T* obj = new T(name);
		obj->mScene = this;
		auto ptr = std::shared_ptr<T>(obj);

		mObjects.push_back(ptr);
		if (Renderer* r = dynamic_cast<Renderer*>(obj)) mRenderers.push_back(r);
		if (Light* l = dynamic_cast<Light*>(obj)) mLights.push_back(l);

		return ptr;
	}

	// Adds an object to this scene
	// does NOT check for duplicates
	template<class T>
	void AddObject(std::shared_ptr<T> object) {
		static_assert(std::is_base_of<Object, T>::value, "T must be an Object!");

		object->mScene = this;
		mObjects.push_back(object);
		if (Renderer* r = dynamic_cast<Renderer*>(object.get())) mRenderers.push_back(r);
		if (Light* l = dynamic_cast<Light*>(object.get())) mLights.push_back(l);
	}

	template<class T>
	bool RemoveObject(std::shared_ptr<T> object) {
		static_assert(std::is_base_of<Object, T>::value, "T must be an Object!");

		if (Renderer* r = dynamic_cast<Renderer*>(object.get())) {
			for (int i = 0; i < mRenderers.size(); i++)
				if (mRenderers[i] == r) {
					mRenderers.remove(i);
					i--;
				}
		}

		if (Light* l = dynamic_cast<Light*>(object.get())) {
			for (int i = 0; i < mLights.size(); i++)
				if (mLights[i] == l) {
					mLights.remove(i);
					i--;
				}
		}

		bool r = false;
		for (int i = 0; i < mObjects.size(); i++)
			if (mObjects[i] == object) {
				mObjects.remove(i);
				i--;
				r = true;
			}

		return r;
	}

	void Skybox(std::shared_ptr<Material> s) { mSkybox = s; }
	std::shared_ptr<Material> Skybox() const { return mSkybox; }

private:
	jvector<std::shared_ptr<Object>> mObjects;
	jvector<Renderer*> mRenderers;
	jvector<Light*> mLights;

	std::shared_ptr<Shader> mDebugShader;
	std::shared_ptr<Mesh> mDebugCube;

	std::shared_ptr<Mesh> mSkyCube;
	std::shared_ptr<Material> mSkybox;

	JAE_API void DebugDrawBox(std::shared_ptr<CommandList> commandList, const DirectX::BoundingOrientedBox &box, const DirectX::XMMATRIX &vp);
};

