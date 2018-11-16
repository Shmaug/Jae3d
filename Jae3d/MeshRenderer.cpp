#include "MeshRenderer.hpp"
#include "d3dx12.hpp"
#include "Graphics.hpp"
#include "Util.hpp"

using namespace Microsoft::WRL;
using namespace std;

MeshRenderer::MeshRenderer(string name) : Object(name) { Create(); }
MeshRenderer::~MeshRenderer() { Release();  }

void MeshRenderer::Create() {
	Release();

	auto device = Graphics::GetDevice();
	// Create Constant Buffer
	ZeroMemory(&m_ObjectBufferData, sizeof(m_ObjectBufferData));
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(AlignUp(sizeof(ObjectBuffer), 256)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_CBuffer)));
	m_CBuffer->SetName(L"CB Object");

	CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_CBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_MappedCBuffer)));
	ZeroMemory(m_MappedCBuffer, (UINT)AlignUp(sizeof(ObjectBuffer), 256));
}

void MeshRenderer::Release() {
	if (m_CBuffer) {
		CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
		m_CBuffer->Unmap(0, &readRange);
		m_CBuffer.Reset();
	}
}
bool MeshRenderer::UpdateTransform() {
	if (!Object::UpdateTransform()) return false;

	XMStoreFloat4x4(&m_ObjectBufferData.ObjectToWorld, ObjectToWorld());
	XMStoreFloat4x4(&m_ObjectBufferData.WorldToObject, WorldToObject());
	memcpy(m_MappedCBuffer, &m_ObjectBufferData, sizeof(ObjectBuffer));

	return true;
}
void MeshRenderer::Draw(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	if (!m_Mesh) return;
	UpdateTransform();

	commandList->SetGraphicsRootConstantBufferView(0, GetCBuffer());

	m_Mesh->Draw(commandList);
}