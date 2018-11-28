#include "CommandList.hpp"

#include "Camera.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

#include "Util.hpp"

using namespace std;
using namespace Microsoft::WRL;

CommandList::CommandList(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12CommandAllocator> allocator) {
	ThrowIfFailed(device->CreateCommandList(0, type, allocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
}
CommandList::~CommandList() {}

void CommandList::Reset(ComPtr<ID3D12CommandAllocator> allocator) {
	ThrowIfFailed(m_CommandList->Reset(allocator.Get(), nullptr));
	m_ActiveShader = nullptr;
	m_ActiveCamera = nullptr;
}

void CommandList::TransitionResource(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) {
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), from, to);
	m_CommandList->ResourceBarrier(1, &barrier);
}

void CommandList::SetShader(shared_ptr<Shader> shader) {
	if (m_ActiveShader == shader) return;
	if (shader) {
		shader->SetActive(m_CommandList);
		if (m_ActiveCamera)
			m_ActiveCamera->SetActive(m_CommandList); // set camera data again since root signature changed
	}
	m_ActiveShader = shader;
}
void CommandList::SetMaterial(shared_ptr<Material> material) {
	if (m_ActiveMaterial == material) return;
	if (material) {
		if (material->m_Shader)
			SetShader(material->m_Shader);
		material->SetActive(m_CommandList);
	}
	m_ActiveMaterial = material;
}
void CommandList::SetCamera(shared_ptr<Camera> camera) {
	if (m_ActiveCamera == camera) return;

	if (camera && m_ActiveShader) camera->SetActive(m_CommandList);
	m_ActiveCamera = camera;
}
void CommandList::DrawMesh(Mesh &mesh) {
	// set PSO
	assert(m_ActiveShader && m_ActiveCamera);

	m_ActiveShader->SetPSO(m_CommandList, mesh.Semantics());
	mesh.Draw(m_CommandList);
}