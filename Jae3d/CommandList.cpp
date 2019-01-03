#include "CommandList.hpp"

#include "Camera.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "ConstantBuffer.hpp"
#include "Texture.hpp"
#include "AssetDatabase.hpp"
#include "Graphics.hpp"
#include "Window.hpp"

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
	mGlobals.clear();
}

void CommandList::TransitionResource(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) {
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), from, to));
}
void CommandList::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) {
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, from, to));
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
		D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.f, 0.f, (float)camera->mPixelWidth, (float)camera->mPixelHeight);
		D3D12_RECT sr = { 0, 0, (long)camera->mPixelWidth, (long)camera->mPixelHeight };
		mCommandList->RSSetViewports(1, &vp);
		mCommandList->RSSetScissorRects(1, &sr);
		mCommandList->OMSetRenderTargets(1, &camera->RTVHandle(), FALSE, &camera->DSVHandle());
		
		camera->WriteCBuffer(mFrameIndex);
		SetGlobalCBuffer(L"CameraBuffer", camera->mCBuffer);
		mState.msaaSamples = camera->mMSAACount;
		mState.depthFormat = camera->mDepthFormat;
		mState.renderFormat = camera->mRenderFormat;
	}
	mActiveCamera = camera;
}

void CommandList::SetGlobalTexture(jwstring param, shared_ptr<Texture> tex) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_TEXTURE;
		p.value.set(tex);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_TEXTURE;
		p.value.set(tex);
		mGlobals.emplace(param, p);
	}

	if (mActiveMaterial) mActiveMaterial->SetTexture(param, tex, mFrameIndex);
}
void CommandList::SetGlobalCBuffer(jwstring param, shared_ptr<ConstantBuffer> cbuf) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_CBUFFER;
		p.value.set(cbuf);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_CBUFFER;
		p.value.set(cbuf);
		mGlobals.emplace(param, p);
	}

	if (mActiveMaterial) mActiveMaterial->SetCBuffer(param, cbuf, mFrameIndex);
}
void CommandList::SetGlobals() {
	if (!mActiveMaterial) return;

	for (const auto &it : mGlobals){
		switch (it.second.type) {
		case SHADER_PARAM_TYPE_CBUFFER:
			mActiveMaterial->SetCBuffer(it.first, it.second.value.cbufferValue, mFrameIndex);
			break;
		case SHADER_PARAM_TYPE_TEXTURE:
			mActiveMaterial->SetTexture(it.first, it.second.value.textureValue, mFrameIndex);
			break;
		case SHADER_PARAM_TYPE_TABLE:
			mActiveMaterial->SetDescriptorTable(it.first, it.second.value.tableValue, mFrameIndex);
			break;
			// TODO: the rest of the types
		}
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