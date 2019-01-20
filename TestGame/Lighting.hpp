#pragma once

#include <Camera.hpp>
#include <Scene.hpp>
#include <Shader.hpp>
#include <ConstantBuffer.hpp>
#include <Texture.hpp>
#include <DescriptorTable.hpp>
#include <Material.hpp>
#include <CommandList.hpp>
#include <Light.hpp>
#include <memory>

class Lighting {
public:
	Lighting();
	~Lighting();

	void CalculateScreenLights(std::shared_ptr<CommandList> commandList, std::shared_ptr<Camera> camera, std::shared_ptr<Scene> scene);

	void SetEnvironmentTexture(std::shared_ptr<Texture> t) { mEnvironmentTexture = t; }
	std::shared_ptr<Texture> CalculateEnvironmentTexture();

	std::shared_ptr<Texture> GetShadowAtlas(unsigned int frameIndex) const { return mShadowAtlas[frameIndex]; };

private:
	struct GPULight {
		DirectX::XMFLOAT3 position;
		float range;

		DirectX::XMFLOAT3 color;
		float mode;

		DirectX::XMFLOAT3 direction;
		float angle;

		int shadowIndex;
		unsigned int pad0;
		unsigned int pad1;
		unsigned int pad2;
	};
	struct GPUShadow {
		DirectX::XMUINT4 uv;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 viewProjection;
		float strength;
		float bias;
		float normalBias;
		unsigned int pad;
	};
	struct LightingBuffer {
		GPULight lights[64];
		GPUShadow shadows[64];
		unsigned int lightCount;
		DirectX::XMUINT2 indexBufferSize;
		unsigned int pad;
		DirectX::XMFLOAT4 groundColor;
		DirectX::XMFLOAT4 skyColor;
	};

	std::shared_ptr<Texture>* mLightIndexTexture;
	std::shared_ptr<Texture>* mShadowAtlas;
	std::shared_ptr<DescriptorTable>* mLightingTextureTable;

	LightingBuffer mLightData;
	std::shared_ptr<ConstantBuffer> mLightBuffer;

	std::shared_ptr<Texture> mEnvironmentTexture;
	std::shared_ptr<Object> mShadowCameraMount;
	std::shared_ptr<Camera> mShadowCamera;
	std::shared_ptr<Material> mDepthMaterial;
	std::shared_ptr<Shader> mLightCompute;

	std::shared_ptr<Camera> RenderShadows(std::shared_ptr<CommandList> commandList, std::shared_ptr<Camera> camera, Light* light, std::shared_ptr<Scene> scene, DirectX::XMUINT4 vp);
};