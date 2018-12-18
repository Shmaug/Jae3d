#pragma once

#include <memory>
#include <type_traits>

#include "Util.hpp"
#include "jvector.hpp"
#include "jstring.hpp"

#include "Renderer.hpp"
#include "Light.hpp"

class CommandList;

class Scene : std::enable_shared_from_this<Scene> {
public:
	JAE_API void Draw(std::shared_ptr<CommandList> commandList, std::shared_ptr<Camera> camera);

	template<class T>
	std::shared_ptr<T> AddObject(jwstring name) {
		static_assert(std::is_base_of<Object, T>::value, "T must be an Object!");

		std::shared_ptr<T> obj = std::shared_ptr<T>(new T(name));

		mObjects.push_back(obj);
		if (std::is_base_of<Light, T>::value) mLights.push_back(std::dynamic_pointer_cast<Light>(obj));
		if (std::is_base_of<Renderer, T>::value) mRenderers.push_back(std::dynamic_pointer_cast<Renderer>(obj));

		return obj;
	}

	template<class T>
	void AddObject(std::shared_ptr<T> obj) {
		static_assert(std::is_base_of<Object, T>::value, "T must be an Object!");

		mObjects.push_back(obj);
		if (std::is_base_of<Light, T>::value) mLights.push_back(std::dynamic_pointer_cast<Light>(obj));
		if (std::is_base_of<Renderer, T>::value) mRenderers.push_back(std::dynamic_pointer_cast<Renderer>(obj));
		return obj;
	}

	template<class T>
	bool RemoveObject(std::shared_ptr<T> object) {
		static_assert(std::is_base_of<Object, T>::value, "T must be an Object!");

		bool r = false;

		if (std::is_base_of<Light, T>::value)
			for (int i = 0; i < mLights.size(); i++)
				if (mLights[i] == object) {
					mLights.remove(i);
					i--;
					r = true;
				}

		if (std::is_base_of<Renderer, T>::value)
			for (int i = 0; i < mRenderers.size(); i++)
				if (mRenderers[i] == object) {
					mRenderers.remove(i);
					i--;
					r = true;
				}

		for (int i = 0; i < mObjects.size(); i++)
			if (mObjects[i] == object) {
				mObjects.remove(i);
				i--;
				r = true;
			}

		return r;
	}

	JAE_API jvector<std::shared_ptr<Light>> CollectLights(std::shared_ptr<Camera> camera);

private:
	jvector<std::shared_ptr<Object>> mObjects;
	jvector<std::shared_ptr<Light>> mLights;
	jvector<std::shared_ptr<Renderer>> mRenderers;
};

