#include "DescriptorTable.hpp"
#include "Graphics.hpp"
#include "Texture.hpp"
#include "ConstantBuffer.hpp"

DescriptorTable::DescriptorTable(unsigned int size) : DescriptorTable(L"", size) {}
DescriptorTable::DescriptorTable(jwstring name, unsigned int size) : mSize(size) {
	mHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, size, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	mHeap->SetName(name.c_str());
	mTable.reserve(size);
	for (unsigned int i = 0; i < size; i++)
		mTable.push_back(nullptr);
}
DescriptorTable::~DescriptorTable() {
	mHeap.Reset();
	mTable.free();
}

void DescriptorTable::SetTexture(unsigned int i, std::shared_ptr<Texture> tex, bool uav) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(i, Graphics::DescriptorIncrement());

	D3D12_RESOURCE_DESC desc = tex->GetTexture()->GetDesc();

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = desc.Format;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = desc.Format;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;

	switch (desc.Dimension) {
	default:
	case (D3D12_RESOURCE_DIMENSION_UNKNOWN):
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_UNKNOWN;
		break;
	case (D3D12_RESOURCE_DIMENSION_BUFFER):
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		break;
	case (D3D12_RESOURCE_DIMENSION_TEXTURE1D):
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
		srvDesc.Texture1D.MipLevels = desc.MipLevels;
		break;
	case (D3D12_RESOURCE_DIMENSION_TEXTURE2D):
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		break;
	case (D3D12_RESOURCE_DIMENSION_TEXTURE3D):
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MipLevels = desc.MipLevels;
		uavDesc.Texture3D.WSize = -1;
		break;
	}

	if (uav)
		Graphics::GetDevice()->CreateUnorderedAccessView(tex->GetTexture().Get(), 0, &uavDesc, handle);
	else
		Graphics::GetDevice()->CreateShaderResourceView(tex->GetTexture().Get(), &srvDesc, handle);

	mTable[i] = tex;
}