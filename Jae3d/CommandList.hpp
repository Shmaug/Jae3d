#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <memory>

#include <d3d12.h>

class Camera;
class Mesh;
class Shader;
class Material;

class CommandList {
public:
	JAE_API CommandList(_WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, _WRL::ComPtr<ID3D12CommandAllocator> allocator);
	JAE_API ~CommandList();

	JAE_API void Reset(_WRL::ComPtr<ID3D12CommandAllocator> allocator, unsigned int frameIndex);

	JAE_API void TransitionResource(_WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);

	JAE_API void SetCompute(std::shared_ptr<Shader> shader);
	JAE_API void SetShader(std::shared_ptr<Shader> shader);
	JAE_API void SetMaterial(std::shared_ptr<Material> material);
	JAE_API void SetCamera(std::shared_ptr<Camera> camera);

	JAE_API void DrawMesh(Mesh &mesh);

	inline _WRL::ComPtr<ID3D12GraphicsCommandList2> D3DCommandList() const { return mCommandList; }

private:
	std::shared_ptr<Material> mActiveMaterial;
	std::shared_ptr<Shader> mActiveShader;
	std::shared_ptr<Camera> mActiveCamera;

	_WRL::ComPtr<ID3D12GraphicsCommandList2> mCommandList;

	unsigned int mFrameIndex;
};

