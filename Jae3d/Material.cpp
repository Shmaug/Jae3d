#include "Material.hpp"

#include "Shader.hpp"
#include "Texture.hpp"
#include "CommandList.hpp"
#include "ConstantBuffer.hpp"
#include "Graphics.hpp"
#include "DescriptorTable.hpp"
#include "Profiler.hpp"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

Material::Material(const jwstring& name, const shared_ptr<Shader>& shader)
	: mName(name), mRenderQueue(1000), mZTest(true), mZWrite(true), mBlend(BLEND_STATE_DEFAULT), mCullMode(D3D12_CULL_MODE_BACK) {
	SetShader(shader, true);
}
Material::~Material() {
	if (mParamCbuffers) delete[] mParamCbuffers;
	mParamValues.clear();
}

void Material::SetShader(const shared_ptr<Shader>& shader, bool reset) {
	//OutputDebugf("MATERIAL %s: %s\n", mName.c_str(), shader->mName.c_str());

	mShader = shader;

	mParamCbufferCount = 0;
	if (mParamCbuffers) delete[] mParamCbuffers;

	if (!shader) return;

	// make cbuffers for shader parameters
	mParamCbufferCount = shader->GetParameterBufferCount();
	if (mParamCbufferCount > 0) {
		mParamCbuffers = new MaterialParameterCBuffer[mParamCbufferCount];
		for (int i = 0; i < mParamCbufferCount; i++) {
			ShaderParameterBuffer pb = shader->GetParameterBuffer(i);
			jwstring name = L"Material Constant Buffer:" + mName + L", " + to_wstring(pb.rootIndex);
			mParamCbuffers[i] = MaterialParameterCBuffer(pb.rootIndex, shared_ptr<ConstantBuffer>(new ConstantBuffer(pb.size, name, Graphics::BufferCount())));
			
			//OutputDebugf("Root Parameter %d: Integral Constant Buffer (%d)\n", pb.rootIndex, pb.size);
		}
	}

	// create or update shader parameters to match new shader
	if (shader->HasParameters()) {
		auto it = shader->ParameterBegin();
		while (it != shader->ParameterEnd()) {
			ShaderParameter sp = it->second;
			MaterialValue v;

			bool f = mParamValues.count(it->first);
			if (f) v = mParamValues.at(it->first);

			// update material cbuffer index
			for (int i = 0; i < mParamCbufferCount; i++) {
				if (mParamCbuffers[i].rootIndex == sp.RootIndex()) {
					v.cbufferIndex = i;
					break;
				}
			}

			if (reset || !f) {
				//OutputDebugf("resetting value\n");
				// set value to the shader's default values
				switch (sp.Type()) {
				case SHADER_PARAM_TYPE_CBUFFER:
					v.set(shared_ptr<ConstantBuffer>(nullptr));
					break;
				case SHADER_PARAM_TYPE_SRV:
					v.set(shared_ptr<Texture>(nullptr));
					break;
				case SHADER_PARAM_TYPE_TABLE:
					v.set(shared_ptr<DescriptorTable>(nullptr));
					break;

				case SHADER_PARAM_TYPE_FLOATRANGE:
					v.set(sp.GetDefaultValue().floatValue);
					v.range.floatRange = sp.GetDefaultValue().floatRange;
					break;
				case SHADER_PARAM_TYPE_INTRANGE:
					v.set(sp.GetDefaultValue().intValue);
					v.range.intRange = sp.GetDefaultValue().intRange;
					break;
				case SHADER_PARAM_TYPE_UINTRANGE:
					v.set(sp.GetDefaultValue().uintValue);
					v.range.uintRange = sp.GetDefaultValue().uintRange;
					break;

				case SHADER_PARAM_TYPE_COLOR3:
					v.set(sp.GetDefaultValue().float3Value);
					break;
				case SHADER_PARAM_TYPE_COLOR4:
					v.set(sp.GetDefaultValue().float4Value);
					break;

				case SHADER_PARAM_TYPE_FLOAT:
					v.set(sp.GetDefaultValue().floatValue);
					break;
				case SHADER_PARAM_TYPE_FLOAT2:
					v.set(sp.GetDefaultValue().float2Value);
					break;
				case SHADER_PARAM_TYPE_FLOAT3:
					v.set(sp.GetDefaultValue().float3Value);
					break;
				case SHADER_PARAM_TYPE_FLOAT4:
					v.set(sp.GetDefaultValue().float4Value);
					break;

				case SHADER_PARAM_TYPE_INT:
					v.set(sp.GetDefaultValue().intValue);
					break;
				case SHADER_PARAM_TYPE_INT2:
					v.set(sp.GetDefaultValue().int2Value);
					break;
				case SHADER_PARAM_TYPE_INT3:
					v.set(sp.GetDefaultValue().int3Value);
					break;
				case SHADER_PARAM_TYPE_INT4:
					v.set(sp.GetDefaultValue().int4Value);
					break;

				case SHADER_PARAM_TYPE_UINT:
					v.set(sp.GetDefaultValue().uintValue);
					break;
				case SHADER_PARAM_TYPE_UINT2:
					v.set(sp.GetDefaultValue().uint2Value);
					break;
				case SHADER_PARAM_TYPE_UINT3:
					v.set(sp.GetDefaultValue().uint3Value);
					break;
				case SHADER_PARAM_TYPE_UINT4:
					v.set(sp.GetDefaultValue().uint4Value);
					break;
				}
			}

			// write value to cbuffer
			switch (sp.Type()) {
			case SHADER_PARAM_TYPE_CBUFFER:
			case SHADER_PARAM_TYPE_SRV:
			case SHADER_PARAM_TYPE_TABLE:
				break;

			case SHADER_PARAM_TYPE_FLOATRANGE:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat(v.floatValue, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_INTRANGE:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteInt(v.intValue, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_UINTRANGE:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteUInt(v.uintValue, sp.CBufferOffset(), -1);
				break;

			case SHADER_PARAM_TYPE_COLOR3:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat3(v.float3Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_COLOR4:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat4(v.float4Value, sp.CBufferOffset(), -1);
				break;

			case SHADER_PARAM_TYPE_FLOAT:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat(v.floatValue, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_FLOAT2:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat2(v.float2Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_FLOAT3:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat3(v.float3Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_FLOAT4:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat4(v.float4Value, sp.CBufferOffset(), -1);
				break;

			case SHADER_PARAM_TYPE_INT:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteInt(v.intValue, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_INT2:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteInt2(v.int2Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_INT3:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteInt3(v.int3Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_INT4:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteInt4(v.int4Value, sp.CBufferOffset(), -1);
				break;

			case SHADER_PARAM_TYPE_UINT:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteUInt(v.uintValue, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_UINT2:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteUInt2(v.uint2Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_UINT3:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteUInt3(v.uint3Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_UINT4:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteUInt4(v.uint4Value, sp.CBufferOffset(), -1);
				break;
			}

			//if (sp.Type() > SHADER_PARAM_TYPE_TEXTURE)
			//	OutputDebugf("Root Parameter %d offset %d: %s (%s)\n", sp.RootIndex(), sp.CBufferOffset(), (*it).Key().c_str(), sp.ToString().c_str());
			//else
			//	OutputDebugf("Root Parameter %d: %s (%s)\n", sp.RootIndex(), (*it).Key().c_str(), sp.ToString().c_str());

			mParamValues.emplace(it->first, v);
			it++;
		} 
	}
}

bool Material::IsKeywordEnabled(const char* keyword) {
	for (unsigned int i = 0; i < mKeywords.size(); i++)
		if (mKeywords[i] == keyword)
			return true;
	return false;
}
void Material::EnableKeyword(const char* keyword) {
	if (IsKeywordEnabled(keyword)) return;
	mKeywords.push_back(keyword);
}
void Material::DisableKeyword(const char* keyword) {
	for (unsigned int i = 0; i < mKeywords.size(); i++)
		if (mKeywords[i] == keyword) {
			mKeywords.remove(i);
			break;
		}
}

void Material::SetFloat (const char* param, const float& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteFloat(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}
void Material::SetFloat2(const char* param, const XMFLOAT2& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteFloat2(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}
void Material::SetFloat3(const char* param, const XMFLOAT3& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteFloat3(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}
void Material::SetFloat4(const char* param, const XMFLOAT4& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteFloat4(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}

void Material::SetInt (const char* param, const int& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteInt(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}
void Material::SetInt2(const char* param, const XMINT2& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteInt2(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}
void Material::SetInt3(const char* param, const XMINT3& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteInt3(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}
void Material::SetInt4(const char* param, const XMINT4& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteInt4(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}

void Material::SetUInt (const char* param, const unsigned int& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteUInt(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}
void Material::SetUInt2(const char* param, const XMUINT2& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteUInt2(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}
void Material::SetUInt3(const char* param, const XMUINT3& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteUInt3(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}
void Material::SetUInt4(const char* param, const XMUINT4& v, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(v);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteUInt4(v, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}

void Material::SetColor3(const char* param, const XMFLOAT3& col, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(col);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteFloat3(col, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}
void Material::SetColor4(const char* param, const XMFLOAT4& col, unsigned int frameIndex) {
	if (!mShader || !mParamValues.count(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.set(col);
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteFloat4(col, mShader->GetParameter(param).CBufferOffset(), frameIndex);
}

void Material::SetTexture(const char* param, const shared_ptr<Texture>& tex, unsigned int frameIndex) {
	mParamValues[param].set(tex);
}
void Material::SetCBuffer(const char* param, const shared_ptr<ConstantBuffer>& cbuf, unsigned int frameIndex) {
	mParamValues[param].set(cbuf);
}
void Material::SetDescriptorTable(const char* param, const shared_ptr<DescriptorTable>& tbl, unsigned int frameIndex) {
	mParamValues[param].set(tbl);
}

bool Material::Equals(const Material& m, unsigned int frameIndex) const {
	if (mShader != m.mShader) return false;

	if (mCullMode != m.mCullMode) return false;
	if (mBlend != m.mBlend) return false;
	if (mZWrite != m.mZWrite) return false;
	if (mZTest != m.mZTest) return false;

	if (mShader->KeywordListToString(mKeywords) != m.mShader->KeywordListToString(m.mKeywords)) return false;

	for (int i = 0; i < mParamCbufferCount; i++)
		mParamCbuffers[i].cbuffer->Equals(*m.mParamCbuffers[i].cbuffer, frameIndex);

	// find values assigned to each root index

	unordered_map<unsigned int, pair<MaterialValue, ShaderParameter>> v1;
	unordered_map<unsigned int, pair<MaterialValue, ShaderParameter>> v2;
	unsigned int r = 0;

	for (const auto& it : mParamValues) {
		if (!mShader->HasParameter(it.first)) continue;
		const ShaderParameter& sp = mShader->GetParameter(it.first);
		v1[sp.RootIndex()] = make_pair(it.second, sp);
		r = max(r, sp.RootIndex());
	}

	for (const auto& it : m.mParamValues) {
		if (!mShader->HasParameter(it.first)) continue;
		const ShaderParameter& sp = mShader->GetParameter(it.first);
		v2[sp.RootIndex()] = make_pair(it.second, sp);
		r = max(r, sp.RootIndex());
	}

	for (unsigned int i = 0; i <= r; i++) {
		unsigned int c1 = (unsigned int)v1.count(i);
		unsigned int c2 = (unsigned int)v2.count(i);
		if (c1 != c2) return false; // materials dont have the same parameters set

		auto p1 = v1.at(i);
		auto p2 = v2.at(i);

		switch (p1.second.Type()) {
		case SHADER_PARAM_TYPE_CBUFFER:
		{
			if (!(p1.first.cbufferValue && p2.first.cbufferValue && p1.first.cbufferValue->Equals(*p2.first.cbufferValue, frameIndex))) return false;
			break;
		}
		case SHADER_PARAM_TYPE_SRV:
		{
			if (!(p1.first.textureValue && p2.first.textureValue && p1.first.textureValue == p2.first.textureValue)) return false;
			break;
		}
		case SHADER_PARAM_TYPE_TABLE:
		{
			if (!(p1.first.tableValue && p2.first.tableValue && p1.first.tableValue == p2.first.tableValue)) return false;
			break;
		}
		}
	}

	return true;
}

void Material::SetActive(CommandList* commandList) {
	assert(mShader);

	commandList->SetKeywords(mKeywords);
	commandList->SetShader(mShader);
	commandList->SetBlendState(mBlend);
	commandList->DepthTestEnabled(mZTest);
	commandList->DepthWriteEnabled(mZWrite);
	commandList->SetCullMode(mCullMode);
	
	for (int i = 0; i < mParamCbufferCount; i++)
		commandList->SetRootCBV(mParamCbuffers[i].rootIndex, mParamCbuffers[i].cbuffer->GetGPUAddress(commandList->GetFrameIndex()));

	jvector<ID3D12DescriptorHeap*> heaps;
	jvector<std::pair<UINT, D3D12_GPU_DESCRIPTOR_HANDLE>> tables;

	for (const auto& it : mParamValues) {
		if (!mShader->HasParameter(it.first)) continue;
		const ShaderParameter& sp = mShader->GetParameter(it.first);
		switch (sp.Type()) {
		case SHADER_PARAM_TYPE_CBUFFER:
		{
			if (it.second.cbufferValue)
				commandList->SetRootCBV(sp.RootIndex(), it.second.cbufferValue->GetGPUAddress(commandList->GetFrameIndex()));
			break;
		}
		case SHADER_PARAM_TYPE_SRV:
		{
			if (it.second.textureValue)
				commandList->SetGraphicsRootDescriptorTable(sp.RootIndex(), it.second.textureValue->GetSRVDescriptorHeap().Get(), it.second.textureValue->GetSRVGPUDescriptor());
			break;
		}
		case SHADER_PARAM_TYPE_TABLE:
		{
			if (it.second.tableValue && it.second.tableValue->Size() > 0)
				commandList->SetGraphicsRootDescriptorTable(sp.RootIndex(), it.second.tableValue->D3DHeap().Get(), it.second.tableValue->GpuDescriptor());
			break;
		}
		}
	}
}