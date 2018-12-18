#include "Scene.hpp"
#include "CommandList.hpp"
#include "Light.hpp"
#include "Renderer.hpp"
#include "Camera.hpp"

void Scene::Draw(std::shared_ptr<CommandList> commandList, std::shared_ptr<Camera> camera) {
	camera->CalculateScreenLights(CollectLights(camera), commandList->GetFrameIndex());
	commandList->SetCamera(camera);
	for (int i = 0; i < mRenderers.size(); i++)
		mRenderers[i]->Draw(commandList);
}

jvector<std::shared_ptr<Light>> Scene::CollectLights(std::shared_ptr<Camera> camera) {
	jvector<std::shared_ptr<Light>> lights(64);
	for (int i = 0; i < mLights.size(); i++)
		lights.push_back(mLights[i]);
	return lights;
}