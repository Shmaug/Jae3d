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
	m_Params[param] = tex;
}
void Material::SetFloat(string param, float v) {
	m_Params[param] = v;
}
void Material::SetVector(string param, XMFLOAT4 vec) {
	m_Params[param] = vec;
}

void Material::SetActive(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	for (auto const&[name, p] : m_Params) {
		Shader::Parameter* sp = m_Shader->GetParameter(name);
		switch (sp->Type()) {
		case Shader::PARAM_TYPE_SRV:
			shared_ptr<Texture> tex = get<shared_ptr<Texture>>(p);
			ID3D12DescriptorHeap* heaps[] = { tex->GetDescriptorHeap().Get() };
			commandList->SetDescriptorHeaps(1, heaps);
			commandList->SetGraphicsRootDescriptorTable(sp->Index(), tex->GetGPUDescriptor());
			break;
		}
	}
}