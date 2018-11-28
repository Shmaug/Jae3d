#include "Material.hpp"

#include "Shader.hpp"
#include "Texture.hpp"
#include "CommandList.hpp"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

Material::Material(string name, shared_ptr<Shader> shader) : m_Name(name), m_Shader(shader) {}
Material::~Material() { m_Params.clear(); }

void Material::SetTexture(string param, shared_ptr<Texture> tex) {
	m_Params[param].texValue = tex;
}
void Material::SetFloat(string param, float v) {
	m_Params[param].floatValue = v;
}
void Material::SetVector(string param, XMFLOAT4 vec) {
	m_Params[param].vecValue = vec;
}

void Material::SetActive(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	for (auto const&[name, p] : m_Params) {
		Shader::Parameter* sp = m_Shader->GetParameter(name);
		switch (sp->Type()) {
		case Shader::PARAM_TYPE_SRV:
			ID3D12DescriptorHeap* heaps[] = { p.texValue->GetDescriptorHeap().Get() };
			commandList->SetDescriptorHeaps(1, heaps);
			commandList->SetGraphicsRootDescriptorTable(sp->Index(), p.texValue->GetGPUDescriptor());
			break;
		}
	}
}