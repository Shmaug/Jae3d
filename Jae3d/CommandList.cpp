#include "CommandList.hpp"

#include "Camera.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "AssetDatabase.hpp"

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

CommandList::CommandList(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12CommandAllocator> allocator) {
	ThrowIfFailed(device->CreateCommandList(0, type, allocator.Get(), nullptr, IID_PPV_ARGS(&mCommandList)));
}
CommandList::~CommandList() {}

void CommandList::Reset(ComPtr<ID3D12CommandAllocator> allocator, unsigned int frameIndex) {
	ThrowIfFailed(mCommandList->Reset(allocator.Get(), nullptr));
	mActiveShader = nullptr;
	mActiveCamera = nullptr;
	mActiveMaterial = nullptr;
	mFrameIndex = frameIndex;
	mState = ShaderState();
}

void CommandList::TransitionResource(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) {
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), from, to));
}

void CommandList::SetCompute(shared_ptr<Shader> shader) {
	if (mActiveShader == shader) return;
	if (shader) shader->SetCompute(mCommandList);
	mActiveShader = shader;
}
void CommandList::SetShader(shared_ptr<Shader> shader) {
	if (mActiveShader == shader) return;

	if (shader) shader->SetActive(mCommandList);
	mActiveShader = shader;
}
void CommandList::SetMaterial(shared_ptr<Material> material) {
	if (mActiveMaterial == material) return;

	if (material) {
		material->SetActive(this);
		SetGlobals();
	}

	if (mActiveMaterial) {
		for (int i = 0; i < mActiveMaterial->mActive.size(); i++)
			if (mActiveMaterial->mActive[i] == this) {
				mActiveMaterial->mActive.remove(i);
				break;
			}
	}
	mActiveMaterial = material;
}
void CommandList::SetCamera(shared_ptr<Camera> camera) {
	if (mActiveCamera == camera) return;

	if (camera) {
		camera->WriteCBuffer(mFrameIndex);
		SetGlobalCBuffer(L"CameraBuffer", camera->mCBuffer);
	}
	mActiveCamera = camera;
}

void CommandList::SetGlobalTexture(jwstring param, std::shared_ptr<Texture> tex) {
	if (mGlobals.has(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_TEXTURE;
		p.value = tex;
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_TEXTURE;
		p.value = tex;
		mGlobals.emplace(param, p);
	}

	if (mActiveMaterial) mActiveMaterial->SetTexture(param, tex, mFrameIndex);
}
void CommandList::SetGlobalCBuffer(jwstring param, std::shared_ptr<ConstantBuffer> cbuf) {
	if (mGlobals.has(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_CBUFFER;
		p.value = cbuf;
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_CBUFFER;
		p.value = cbuf;
		mGlobals.emplace(param, p);
	}

	if (mActiveMaterial) mActiveMaterial->SetCBuffer(param, cbuf, mFrameIndex);
}
void CommandList::SetGlobals() {
	if (!mActiveMaterial) return;

	auto it = mGlobals.begin();
	while (it.Valid()) {
		switch ((*it).Value().type) {
		case SHADER_PARAM_TYPE_CBUFFER:
			mActiveMaterial->SetCBuffer((*it).Key(), get<shared_ptr<ConstantBuffer>>((*it).Value().value), mFrameIndex);
			break;
		case SHADER_PARAM_TYPE_TEXTURE:
			mActiveMaterial->SetTexture((*it).Key(), get<shared_ptr<Texture>>((*it).Value().value), mFrameIndex);
			break;
		}
		it++;
	}
}

void CommandList::SetBlendState(D3D12_RENDER_TARGET_BLEND_DESC blend) {
	mState.blendState = blend;
}
void CommandList::SetDepthWrite(bool depthWrite) {
	mState.zwrite = depthWrite;
}
void CommandList::SetDepthTest(bool depthTest) {
	mState.ztest = depthTest;
}
void CommandList::SetFillMode(D3D12_FILL_MODE fillMode) {
	mState.fillMode = fillMode;
}
void CommandList::SetCullMode(D3D12_CULL_MODE cullMode) {
	mState.cullMode = cullMode;
}

void CommandList::DrawUserMesh(MESH_SEMANTIC input, D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) {
	assert(mActiveShader);
	mState.input = input;
	mState.topology = topology;
	mActiveShader->SetPSO(mCommandList, mState);
}
void CommandList::DrawMesh(Mesh &mesh) {
	DrawUserMesh(mesh.Semantics(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	mesh.Draw(mCommandList);
}