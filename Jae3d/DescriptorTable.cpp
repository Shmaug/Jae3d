#include "DescriptorTable.hpp"
#include "Graphics.hpp"
#include "Texture.hpp"
#include "ConstantBuffer.hpp"

DescriptorTable::DescriptorTable(unsigned int size) : mSize(size) {
	mHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, size, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}
DescriptorTable::~DescriptorTable() {
	mHeap.Reset();
}

void DescriptorTable::SetTexture(unsigned int i, std::shared_ptr<Texture> tex, bool uav) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(i, Graphics::DescriptorIncrement());

	D3D12_RESOURCE_DESC desc = tex->GetTexture()->GetDesc();

	D3D12_SRV_DIMENSION srvdim;
	D3D12_UAV_DIMENSION uavdim;
	switch (desc.Dimension) {
	default:
	case (D3D12_RESOURCE_DIMENSION_UNKNOWN):
		srvdim = D3D12_SRV_DIMENSION_UNKNOWN;
		uavdim = D3D12_UAV_DIMENSION_UNKNOWN;
		break;
	case (D3D12_RESOURCE_DIMENSION_BUFFER):
		srvdim = D3D12_SRV_DIMENSION_BUFFER;
		uavdim = D3D12_UAV_DIMENSION_BUFFER;
		break;
	case (D3D12_RESOURCE_DIMENSION_TEXTURE1D):
		srvdim = D3D12_SRV_DIMENSION_TEXTURE1D;
		uavdim = D3D12_UAV_DIMENSION_TEXTURE1D;
		break;
	case (D3D12_RESOURCE_DIMENSION_TEXTURE2D):
		srvdim = D3D12_SRV_DIMENSION_TEXTURE2D;
		uavdim = D3D12_UAV_DIMENSION_TEXTURE2D;
		break;
	case (D3D12_RESOURCE_DIMENSION_TEXTURE3D):
		srvdim = D3D12_SRV_DIMENSION_TEXTURE3D;
		uavdim = D3D12_UAV_DIMENSION_TEXTURE3D;
		break;
	}

	if (uav) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = uavdim;
		uavDesc.Format = desc.Format;
		Graphics::GetDevice()->CreateUnorderedAccessView(tex->GetTexture().Get(), 0, &uavDesc, handle);
	} else {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = srvdim;
		srvDesc.Format = desc.Format;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		Graphics::GetDevice()->CreateShaderResourceView(tex->GetTexture().Get(), &srvDesc, handle);

	}

}