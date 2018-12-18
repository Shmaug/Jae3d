#pragma once

#include "Common.hpp"

#ifdef DrawText
#undef DrawText
#endif

class CommandList : public std::enable_shared_from_this<CommandList> {
public:
	JAE_API CommandList(_WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, _WRL::ComPtr<ID3D12CommandAllocator> allocator);
	JAE_API ~CommandList();

	unsigned int GetFrameIndex() const { return mFrameIndex; }

	JAE_API void Reset(_WRL::ComPtr<ID3D12CommandAllocator> allocator, unsigned int frameIndex);

	JAE_API void TransitionResource(_WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);

	JAE_API void SetCompute(std::shared_ptr<Shader> shader);
	JAE_API void SetShader(std::shared_ptr<Shader> shader);
	JAE_API void SetMaterial(std::shared_ptr<Material> material);
	JAE_API void SetCamera(std::shared_ptr<Camera> camera);

	JAE_API void SetBlendState(D3D12_RENDER_TARGET_BLEND_DESC blend);
	JAE_API void SetDepthWrite(bool depthWrite);
	JAE_API void SetDepthTest(bool depthTest);
	JAE_API void SetFillMode(D3D12_FILL_MODE fillMode);

	JAE_API void DrawMesh(Mesh &mesh);
	JAE_API void DrawUserMesh(MESH_SEMANTIC input, D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	inline _WRL::ComPtr<ID3D12GraphicsCommandList2> D3DCommandList() const { return mCommandList; }

private:
	std::shared_ptr<Material> mActiveMaterial;
	std::shared_ptr<Shader> mActiveShader;
	std::shared_ptr<Camera> mActiveCamera;
	ShaderState mState;

	_WRL::ComPtr<ID3D12GraphicsCommandList2> mCommandList;

	unsigned int mFrameIndex;
};

