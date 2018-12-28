#pragma once

#include "Common.hpp"
#include <stack>

// Wraps D3D12CommandList, tracks active objects. Use to do anything on the GPU
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

	JAE_API void SetGlobalTexture(jwstring param, std::shared_ptr<Texture> tex);
	JAE_API void SetGlobalCBuffer(jwstring param, std::shared_ptr<ConstantBuffer> tex);

	inline std::shared_ptr<Camera> GetCamera() const { return mActiveCamera; }

	JAE_API void SetBlendState(D3D12_RENDER_TARGET_BLEND_DESC blend);
	JAE_API void SetDepthWrite(bool depthWrite);
	JAE_API void SetDepthTest(bool depthTest);
	JAE_API void SetFillMode(D3D12_FILL_MODE fillMode);
	JAE_API void SetCullMode(D3D12_CULL_MODE cullMode);

	// Draws a mesh with the active shader. Same as calling DrawUserMesh() with the mesh's input and topology, and then calling mesh->Draw()
	JAE_API void DrawMesh(Mesh &mesh);
	// Sets up the command list to render a mesh with the given input and topology, with the active shader
	JAE_API void DrawUserMesh(MESH_SEMANTIC input, D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	inline _WRL::ComPtr<ID3D12GraphicsCommandList2> D3DCommandList() const { return mCommandList; }

	inline void PushState() { mStateStack.push(mState); }
	inline void PopState() { mState = mStateStack.top(); mStateStack.pop(); }

private:
	struct GlobalParam {
		MaterialValue::Value value;
		SHADER_PARAM_TYPE type;

		GlobalParam() : type(SHADER_PARAM_TYPE_FLOAT) { value = 0.0f; }
		GlobalParam(const GlobalParam &mv) : type(mv.type) {
			memcpy(&value, &mv.value, sizeof(MaterialValue::Value));
		}
		~GlobalParam() {}

		GlobalParam& operator=(const GlobalParam &rhs) {
			if (&rhs == this) return *this;
			memcpy(&value, &rhs.value, sizeof(MaterialValue::Value));
			return *this;
		}
	};

	std::shared_ptr<Material> mActiveMaterial;
	std::shared_ptr<Shader> mActiveShader;
	std::shared_ptr<Camera> mActiveCamera;
	ShaderState mState;

	std::stack<ShaderState> mStateStack;

	_WRL::ComPtr<ID3D12GraphicsCommandList2> mCommandList;

	jmap<jwstring, GlobalParam> mGlobals;

	void SetGlobals();

	unsigned int mFrameIndex;
};

