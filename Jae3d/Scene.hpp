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

	JAE_API void Draw(std::shared_ptr<CommandList> commandList, DirectX::BoundingFrustum cullFrustum);
	JAE_API void DebugDraw(std::shared_ptr<CommandList> commandList, DirectX::BoundingFrustum cullFrustum);
	JAE_API void CollectLights(DirectX::BoundingFrustum &frustum, jvector<Light*> &lights);

	// Creates an object in this scene
	template<class T>
	std::shared_ptr<T> AddObject(jwstring name) {
		static_assert(std::is_base_of<Object, T>::value, "T must be an Object!");

		std::shared_ptr<T> object = std::shared_ptr<T>(new T(name));
		object->mScene = this;

		mObjects.push_back(object);
		if (Renderer* r = dynamic_cast<Renderer*>(object.get())) mRenderers.push_back(r);
		if (Light* l = dynamic_cast<Light*>(object.get())) mLights.push_back(l);

		return object;
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

private:
	jvector<std::shared_ptr<Object>> mObjects;
	jvector<Renderer*> mRenderers;
	jvector<Light*> mLights;

	std::shared_ptr<Shader> mDebugShader;
	std::shared_ptr<Mesh> mDebugCube;
};

