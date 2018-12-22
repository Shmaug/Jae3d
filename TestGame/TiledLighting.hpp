#pragma once

#include <Camera.hpp>
#include <Scene.hpp>
#include <ConstantBuffer.hpp>
#include <Texture.hpp>
#include <memory>

class TiledLighting {
public:
	TiledLighting();
	~TiledLighting();

	uint64_t CalculateScreenLights(std::shared_ptr<Camera> camera, std::shared_ptr<Scene> scene, unsigned int frameIndex);

	std::shared_ptr<ConstantBuffer> GetBuffer() const { return mLightBuffer; }
	std::shared_ptr<Texture> GetTexture(unsigned int frameIndex) const { return mLightIndexTexture[frameIndex]; }

private:
	unsigned int mLightCount;
	std::shared_ptr<Texture>* mLightIndexTexture;
	std::shared_ptr<ConstantBuffer> mLightBuffer;
};

