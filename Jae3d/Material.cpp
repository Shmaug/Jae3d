#include "Material.hpp"

#include "Shader.hpp"
#include "Texture.hpp"
#include "CommandList.hpp"
#include "ConstantBuffer.hpp"
#include "Graphics.hpp"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

Material::Material(jwstring name, shared_ptr<Shader> shader) : mName(name) {
	SetShader(shader, true);
}
Material::~Material() {
	if (mParamCbuffers) delete[] mParamCbuffers;
	mParamValues.clear();
}

void Material::SetShader(shared_ptr<Shader> shader, bool reset) {
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
		while (it.Valid()) {
			ShaderParameter sp = (*it).Value();
			MaterialValue v;

			bool f = mParamValues.has((*it).Key());
			if (f) v = mParamValues.at((*it).Key());

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
					v.value.cbufferValue = shared_ptr<ConstantBuffer>(nullptr);
					break;
				case SHADER_PARAM_TYPE_SRV:
				case SHADER_PARAM_TYPE_UAV:
				case SHADER_PARAM_TYPE_SAMPLER:
					break;
				case SHADER_PARAM_TYPE_TEXTURE:
					v.value.textureValue = shared_ptr<Texture>(nullptr);
					break;

				case SHADER_PARAM_TYPE_FLOATRANGE:
					v.value.floatValue = sp.GetDefaultValue().floatValue;
					v.range.floatRange = sp.GetDefaultValue().floatRange;
					break;
				case SHADER_PARAM_TYPE_INTRANGE:
					v.value.intValue = sp.GetDefaultValue().intValue;
					v.range.intRange = sp.GetDefaultValue().intRange;
					break;
				case SHADER_PARAM_TYPE_UINTRANGE:
					v.value.uintValue = sp.GetDefaultValue().uintValue;
					v.range.uintRange = sp.GetDefaultValue().uintRange;
					break;

				case SHADER_PARAM_TYPE_COLOR3:
					v.value.float3Value = sp.GetDefaultValue().float3Value;
					break;
				case SHADER_PARAM_TYPE_COLOR4:
					v.value.float4Value = sp.GetDefaultValue().float4Value;
					break;

				case SHADER_PARAM_TYPE_FLOAT:
					v.value.floatValue = sp.GetDefaultValue().floatValue;
					break;
				case SHADER_PARAM_TYPE_FLOAT2:
					v.value.float2Value = sp.GetDefaultValue().float2Value;
					break;
				case SHADER_PARAM_TYPE_FLOAT3:
					v.value.float3Value = sp.GetDefaultValue().float3Value;
					break;
				case SHADER_PARAM_TYPE_FLOAT4:
					v.value.float4Value = sp.GetDefaultValue().float4Value;
					break;

				case SHADER_PARAM_TYPE_INT:
					v.value.intValue = sp.GetDefaultValue().intValue;
					break;
				case SHADER_PARAM_TYPE_INT2:
					v.value.int2Value = sp.GetDefaultValue().int2Value;
					break;
				case SHADER_PARAM_TYPE_INT3:
					v.value.int3Value = sp.GetDefaultValue().int3Value;
					break;
				case SHADER_PARAM_TYPE_INT4:
					v.value.int4Value = sp.GetDefaultValue().int4Value;
					break;

				case SHADER_PARAM_TYPE_UINT:
					v.value.uintValue = sp.GetDefaultValue().uintValue;
					break;
				case SHADER_PARAM_TYPE_UINT2:
					v.value.uint2Value = sp.GetDefaultValue().uint2Value;
					break;
				case SHADER_PARAM_TYPE_UINT3:
					v.value.uint3Value = sp.GetDefaultValue().uint3Value;
					break;
				case SHADER_PARAM_TYPE_UINT4:
					v.value.uint4Value = sp.GetDefaultValue().uint4Value;
					break;
				}
			}

			// write value to cbuffer
			switch (sp.Type()) {
			case SHADER_PARAM_TYPE_CBUFFER:
			case SHADER_PARAM_TYPE_SRV:
			case SHADER_PARAM_TYPE_UAV:
			case SHADER_PARAM_TYPE_SAMPLER:
			case SHADER_PARAM_TYPE_TEXTURE:
				break;

			case SHADER_PARAM_TYPE_FLOATRANGE:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat(v.value.floatValue, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_INTRANGE:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteInt(v.value.intValue, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_UINTRANGE:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteUInt(v.value.uintValue, sp.CBufferOffset(), -1);
				break;

			case SHADER_PARAM_TYPE_COLOR3:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat3(v.value.float3Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_COLOR4:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat4(v.value.float4Value, sp.CBufferOffset(), -1);
				break;

			case SHADER_PARAM_TYPE_FLOAT:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat(v.value.floatValue, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_FLOAT2:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat2(v.value.float2Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_FLOAT3:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat3(v.value.float3Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_FLOAT4:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteFloat4(v.value.float4Value, sp.CBufferOffset(), -1);
				break;

			case SHADER_PARAM_TYPE_INT:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteInt(v.value.intValue, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_INT2:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteInt2(v.value.int2Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_INT3:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteInt3(v.value.int3Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_INT4:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteInt4(v.value.int4Value, sp.CBufferOffset(), -1);
				break;

			case SHADER_PARAM_TYPE_UINT:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteUInt(v.value.uintValue, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_UINT2:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteUInt2(v.value.uint2Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_UINT3:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteUInt3(v.value.uint3Value, sp.CBufferOffset(), -1);
				break;
			case SHADER_PARAM_TYPE_UINT4:
				mParamCbuffers[v.cbufferIndex].cbuffer->WriteUInt4(v.value.uint4Value, sp.CBufferOffset(), -1);
				break;
			}

			//if (sp.Type() > SHADER_PARAM_TYPE_TEXTURE)
			//	OutputDebugf("Root Parameter %d offset %d: %s (%s)\n", sp.RootIndex(), sp.CBufferOffset(), (*it).Key().c_str(), sp.ToString().c_str());
			//else
			//	OutputDebugf("Root Parameter %d: %s (%s)\n", sp.RootIndex(), (*it).Key().c_str(), sp.ToString().c_str());

			mParamValues.emplace((*it).Key(), v);

			it++;
		} 
	}

	// re-upload data
	for (unsigned int i = 0; i < mActive.size(); i++)
		SetActive(mActive[i]);
}

void Material::SetFloat(jwstring param, float v, unsigned int frameIndex) {
	if (!mParamValues.has(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.value.floatValue = v;
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteFloat(v, mShader->GetParameter(param)->CBufferOffset(), frameIndex);
}
void Material::SetColor3(jwstring param, XMFLOAT3 col, unsigned int frameIndex) {
	if (!mParamValues.has(param)) return;
	MaterialValue& mv = mParamValues.at(param);
	mv.value.float3Value = col;
	mParamCbuffers[mv.cbufferIndex].cbuffer->WriteFloat3(col, mShader->GetParameter(param)->CBufferOffset(), frameIndex);
}

void Material::SetTexture(jwstring param, shared_ptr<Texture> tex, unsigned int frameIndex) {
	if (!mParamValues.has(param)) return;
	mParamValues.at(param).value.textureValue = tex;

	if (tex) {
		ShaderParameter* sp = mShader->GetParameter(param);
		ID3D12DescriptorHeap* heap = { tex->GetSRVDescriptorHeap().Get() };
		for (unsigned int i = 0; i < mActive.size(); i++) {
			mActive[i]->D3DCommandList()->SetDescriptorHeaps(1, &heap);
			mActive[i]->D3DCommandList()->SetGraphicsRootDescriptorTable(sp->RootIndex(), tex->GetSRVGPUDescriptor());
		}
	}
}
void Material::SetCBuffer(jwstring param, shared_ptr<ConstantBuffer> cbuf, unsigned int frameIndex) {
	if (!mParamValues.has(param)) return;
	mParamValues.at(param).value.cbufferValue = cbuf;

	if (cbuf) {
		ShaderParameter* sp = mShader->GetParameter(param);
		for (unsigned int i = 0; i < mActive.size(); i++)
			mActive[i]->D3DCommandList()->SetGraphicsRootConstantBufferView(sp->RootIndex(), cbuf->GetGPUAddress(frameIndex));
	}
}

void Material::SetActive(CommandList* commandList) {
	if (!mShader) return;
	commandList->SetShader(mShader);

	bool f = false;
	for (unsigned int i = 0; i < mActive.size(); i++)
		if (mActive[i] == commandList) {
			f = true;
			break;
		}
	if (!f) mActive.push_back(commandList);

	auto d3dlist = commandList->D3DCommandList();

	auto it = mParamValues.begin();
	while (it.Valid()) {
		ShaderParameter* sp = mShader->GetParameter((*it).Key());
		if (sp){
			switch (sp->Type()) {
			case SHADER_PARAM_TYPE_CBUFFER:
			{
				shared_ptr<ConstantBuffer> cb = (*it).Value().value.cbufferValue;
				if (cb) d3dlist->SetGraphicsRootConstantBufferView(sp->RootIndex(), cb->GetGPUAddress(commandList->GetFrameIndex()));
				break;
			}
			case SHADER_PARAM_TYPE_TEXTURE:
			{
				shared_ptr<Texture> tex = (*it).Value().value.textureValue;
				if (tex) {
					ID3D12DescriptorHeap* heap = { tex->GetSRVDescriptorHeap().Get() };
					d3dlist->SetDescriptorHeaps(1, &heap);
					d3dlist->SetGraphicsRootDescriptorTable(sp->RootIndex(), tex->GetSRVGPUDescriptor());
				}
				break;
			}
			case SHADER_PARAM_TYPE_SRV:
			{

				break;
			}
			}
		}
		it++;
	}

	for (int i = 0; i < mParamCbufferCount; i++)
		d3dlist->SetGraphicsRootConstantBufferView(mParamCbuffers[i].rootIndex, mParamCbuffers[i].cbuffer->GetGPUAddress(commandList->GetFrameIndex()));
}