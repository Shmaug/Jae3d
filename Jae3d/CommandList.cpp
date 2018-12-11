#include "CommandList.hpp"

#include "Camera.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

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
	mFrameIndex = frameIndex;
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
	if (shader) {
		shader->SetActive(mCommandList);
		if (mActiveCamera)
			mActiveCamera->SetActive(mCommandList, mFrameIndex); // set camera data again since root signature changed
	}
	mActiveShader = shader;
}
void CommandList::SetMaterial(shared_ptr<Material> material) {
	if (mActiveMaterial == material) return;
	if (material) {
		if (material->mShader)
			SetShader(material->mShader);
		material->SetActive(mCommandList, mFrameIndex);
	}
	mActiveMaterial = material;
}
void CommandList::SetCamera(shared_ptr<Camera> camera) {
	if (mActiveCamera == camera) return;

	if (camera && mActiveShader) camera->SetActive(mCommandList, mFrameIndex);
	mActiveCamera = camera;
}
void CommandList::SetBlendState(D3D12_RENDER_TARGET_BLEND_DESC blend) {
	mState.blendState = blend;
}
void CommandList::SetInputElements(MESH_SEMANTIC input) {
	mState.input = input;
}
void CommandList::DrawMesh(Mesh &mesh) {
	assert(mActiveShader);

	mState.input = mesh.Semantics();
	mState.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	mActiveShader->SetPSO(mCommandList, mState);
	mesh.Draw(mCommandList);
}
void CommandList::DrawUserMesh(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) {
	assert(mActiveShader);
	mState.topology = topology;
	mActiveShader->SetPSO(mCommandList, mState);
}