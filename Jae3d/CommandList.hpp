#pragma once

#include "Common.hpp"

#include <unordered_map>
#include <stack>

// Wraps D3D12CommandList. Use to do anything on the GPU
class CommandList : public std::enable_shared_from_this<CommandList> {
public:
	JAE_API CommandList(const _WRL::ComPtr<ID3D12Device2>& device, D3D12_COMMAND_LIST_TYPE type, const _WRL::ComPtr<ID3D12CommandAllocator>& allocator);
	JAE_API ~CommandList();

	// Frame index for double/triple-buffering. Between 0 and Graphics::BufferCount()-1
	unsigned int GetFrameIndex() const { return mFrameIndex; }

	JAE_API void Reset(const _WRL::ComPtr<ID3D12CommandAllocator>& allocator, unsigned int frameIndex);

	JAE_API void TransitionResource(const _WRL::ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
	JAE_API void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);

	JAE_API void SetCompute(const std::shared_ptr<Shader>& shader);
	JAE_API void SetShader (const std::shared_ptr<Shader>& shader);
	// Sets the material's parameters on the GPU
	JAE_API void SetMaterial(const std::shared_ptr<Material>& material);
	// Sets the camera's constant buffer in the "CameraBuffer" material parameter
	// Sets the camera's render/depth buffers as active
	JAE_API void SetCamera(const std::shared_ptr<Camera>& camera);
	// Sets the camera's constant buffer in the "CameraBuffer" material parameter
	// Sets the camera's render/depth buffers as active
	JAE_API void SetCamera(const std::shared_ptr<Camera>& camera, D3D12_VIEWPORT& vp);

	inline unsigned int CurrentRenderTargetWidth() const { return mRTWidth; }
	inline unsigned int CurrentRenderTargetHeight() const { return mRTHeight; }

	JAE_API bool IsKeywordEnabled(const char* keyword);
	JAE_API void EnableKeyword(const char* keyword);
	JAE_API void DisableKeyword(const char* keyword);
	JAE_API void SetKeywords(const jvector<jstring>& keywords);
	JAE_API void ClearKeywords();

	JAE_API void SetGlobalFloat (const jstring& param, const float& v);
	JAE_API void SetGlobalFloat2(const jstring& param, const DirectX::XMFLOAT2& v);
	JAE_API void SetGlobalFloat3(const jstring& param, const DirectX::XMFLOAT3& v);
	JAE_API void SetGlobalFloat4(const jstring& param, const DirectX::XMFLOAT4& v);
	JAE_API void SetGlobalColor3(const jstring& param, const DirectX::XMFLOAT3& v);
	JAE_API void SetGlobalColor4(const jstring& param, const DirectX::XMFLOAT4& v);
	JAE_API void SetGlobalInt (const jstring& param, const int& v);
	JAE_API void SetGlobalInt2(const jstring& param, const DirectX::XMINT2& v);
	JAE_API void SetGlobalInt3(const jstring& param, const DirectX::XMINT3& v);
	JAE_API void SetGlobalInt4(const jstring& param, const DirectX::XMINT4& v);
	JAE_API void SetGlobalUInt(const jstring& param, const unsigned int& v);
	JAE_API void SetGlobalUInt2(const jstring& param, const DirectX::XMUINT2& v);
	JAE_API void SetGlobalUInt3(const jstring& param, const DirectX::XMUINT3& v);
	JAE_API void SetGlobalUInt4(const jstring& param, const DirectX::XMUINT4& v);
	JAE_API void SetGlobalTexture(const jstring& param, const std::shared_ptr<Texture>& tex);
	JAE_API void SetGlobalTable(const jstring& param, const  std::shared_ptr<DescriptorTable>& tex);
	JAE_API void SetGlobalCBuffer(const jstring& param, const  std::shared_ptr<ConstantBuffer>& tex);

	JAE_API void SetRootCBV(unsigned int index, D3D12_GPU_VIRTUAL_ADDRESS cbuffer);
	JAE_API void SetGraphicsRootDescriptorTable(unsigned int index, ID3D12DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE table);
	JAE_API void SetComputeRootDescriptorTable(unsigned int index, ID3D12DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE table);

	JAE_API void SetBlendState(D3D12_RENDER_TARGET_BLEND_DESC blend);
	JAE_API void DepthWriteEnabled(bool depthWrite);
	JAE_API void DepthTestEnabled(bool depthTest);
	bool DepthWriteEnabled() { return mState.zwrite; };
	bool DepthTestEnabled() const { return mState.ztest; };
	JAE_API void SetFillMode(D3D12_FILL_MODE fillMode);
	JAE_API void SetCullMode(D3D12_CULL_MODE cullMode);

	// Draws a submesh with the active shader. Same as calling DrawUserMesh() with the mesh's input and topology, and then calling mesh->Draw(submesh)
	JAE_API void DrawMesh(const Mesh &mesh, unsigned int submesh = 0);
	// Sets up the command list to render a mesh with the given input and topology, with the active shader
	JAE_API void DrawUserMesh(MESH_SEMANTIC input, D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	JAE_API void CommandList::Draw(const D3D12_VERTEX_BUFFER_VIEW& vb, const D3D12_INDEX_BUFFER_VIEW& ib, D3D12_PRIMITIVE_TOPOLOGY topology, UINT indexCount, UINT baseIndex, UINT baseVertex);

	inline _WRL::ComPtr<ID3D12GraphicsCommandList2> D3DCommandList() const { return mCommandList; }

	inline void PushState() { mStateStack.push(mState); }
	inline void PopState() { mState = mStateStack.top(); mStateStack.pop(); }

	// To be populated by the user
	unsigned int mTrianglesDrawn;

private:
	struct GlobalParam {
		MaterialValue value;
		SHADER_PARAM_TYPE type;

		GlobalParam() : type(SHADER_PARAM_TYPE_FLOAT), value(MaterialValue()) {}
		GlobalParam(const GlobalParam &mv) : type(mv.type), value(mv.value) {}
		~GlobalParam() {}

		GlobalParam& operator=(const GlobalParam &rhs) {
			if (&rhs == this) return *this;
			memcpy(&value, &rhs.value, sizeof(MaterialValue));
			return *this;
		}
	};

	union RootParameterEntry {
		D3D12_GPU_VIRTUAL_ADDRESS cbufferValue;
		D3D12_GPU_DESCRIPTOR_HANDLE tableValue;
	};

	std::unordered_map<jstring, GlobalParam> mGlobals;

	std::shared_ptr<Shader> mActiveShader;
	jvector<RootParameterEntry> mActiveRootParameters;
	jvector<RootParameterEntry> mActiveComputeRootParameters;
	const D3D12_VERTEX_BUFFER_VIEW* mActiveVBO;
	const D3D12_INDEX_BUFFER_VIEW* mActiveIBO;
	D3D12_PRIMITIVE_TOPOLOGY mActiveTopology;
	_WRL::ComPtr<ID3D12PipelineState> mActivePSO;

	unsigned int mRTWidth;
	unsigned int mRTHeight;

	ShaderState mState;

	std::stack<ShaderState> mStateStack;
	_WRL::ComPtr<ID3D12GraphicsCommandList2> mCommandList;

	unsigned int mFrameIndex;
};

