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
#include "Profiler.hpp"

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

CommandList::CommandList(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12CommandAllocator> allocator) :
	mTrianglesDrawn(0), mActiveVBO(nullptr), mActiveIBO(nullptr), mActiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED) {
	ThrowIfFailed(device->CreateCommandList(0, type, allocator.Get(), nullptr, IID_PPV_ARGS(&mCommandList)));
}
CommandList::~CommandList() {}

void CommandList::Reset(ComPtr<ID3D12CommandAllocator> allocator, unsigned int frameIndex) {
	ThrowIfFailed(mCommandList->Reset(allocator.Get(), nullptr));
	mActiveShader = nullptr;
	mActiveRootParameters.clear();
	mActiveVBO = nullptr;
	mActiveIBO = nullptr;
	mActiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	mActivePSO.Reset();
	mFrameIndex = frameIndex;
	mTrianglesDrawn = 0;
	mState = ShaderState();
	while (!mStateStack.empty()) mStateStack.pop();
	mRTWidth = 0;
	mRTHeight = 0;
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
	if (shader) shader->SetCompute(mCommandList, mState);
	mActiveShader = shader;
}
void CommandList::SetShader(shared_ptr<Shader> shader) {
	if (mActiveShader == shader) return;

	if (shader) shader->SetActive(mCommandList);
	mActiveRootParameters.clear();
	mActiveShader = shader;
}
void CommandList::SetMaterial(shared_ptr<Material> material) {
	if (material) {
		for (const auto &it : mGlobals) {
			switch (it.second.type) {
			case SHADER_PARAM_TYPE_CBUFFER:
				material->SetCBuffer(it.first, it.second.value.cbufferValue, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_SRV:
				material->SetTexture(it.first, it.second.value.textureValue, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_TABLE:
				material->SetDescriptorTable(it.first, it.second.value.tableValue, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_FLOAT:
				material->SetFloat(it.first, it.second.value.floatValue, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_FLOAT2:
				material->SetFloat2(it.first, it.second.value.float2Value, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_FLOAT3:
				material->SetFloat3(it.first, it.second.value.float3Value, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_FLOAT4:
				material->SetFloat4(it.first, it.second.value.float4Value, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_COLOR3:
				material->SetColor3(it.first, it.second.value.float3Value, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_COLOR4:
				material->SetColor4(it.first, it.second.value.float4Value, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_INT:
				material->SetInt(it.first, it.second.value.intValue, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_INT2:
				material->SetInt2(it.first, it.second.value.int2Value, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_INT3:
				material->SetInt3(it.first, it.second.value.int3Value, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_INT4:
				material->SetInt4(it.first, it.second.value.int4Value, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_UINT:
				material->SetUInt(it.first, it.second.value.uintValue, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_UINT2:
				material->SetUInt2(it.first, it.second.value.uint2Value, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_UINT3:
				material->SetUInt3(it.first, it.second.value.uint3Value, mFrameIndex);
				break;
			case SHADER_PARAM_TYPE_UINT4:
				material->SetUInt4(it.first, it.second.value.uint4Value, mFrameIndex);
				break;
			}
		}
		material->SetActive(this);
	} else
		mState.keywords.clear();
}
void CommandList::SetCamera(shared_ptr<Camera> camera) {
	SetCamera(camera, CD3DX12_VIEWPORT(0.f, 0.f, (float)camera->mPixelWidth, (float)camera->mPixelHeight));
}
void CommandList::SetCamera(shared_ptr<Camera> camera, D3D12_VIEWPORT& vp) {
	if (camera) {
		mRTWidth = camera->mPixelWidth;
		mRTHeight = camera->mPixelHeight;
		D3D12_RECT sr = { 0, 0, (LONG)camera->mPixelWidth, (LONG)camera->mPixelHeight };
		mCommandList->RSSetViewports(1, &vp);
		mCommandList->RSSetScissorRects(1, &sr);
		mCommandList->OMSetRenderTargets(1, &camera->RTVHandle(), FALSE, &camera->DSVHandle());

		camera->WriteCBuffer(mFrameIndex);
		SetGlobalCBuffer("CameraBuffer", camera->mCBuffer);
		mState.msaaSamples = camera->mSampleCount;
		mState.depthFormat = camera->mDepthFormat;
		mState.renderFormat = camera->mRenderFormat;
	}
}

void CommandList::SetRootCBV(unsigned int index, D3D12_GPU_VIRTUAL_ADDRESS cbuffer) {
	if (mActiveRootParameters.size() > index && mActiveRootParameters[index].cbufferValue == cbuffer) return;
	while (mActiveRootParameters.size() <= index) mActiveRootParameters.push_back({ 0 });

	mActiveRootParameters[index].cbufferValue = cbuffer;
	mCommandList->SetGraphicsRootConstantBufferView(index, cbuffer);
}
void CommandList::SetRootDescriptorTable(unsigned int index, ID3D12DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE table) {
	if (mActiveRootParameters.size() > index && mActiveRootParameters[index].tableValue.ptr == table.ptr) return;
	while (mActiveRootParameters.size() <= index) mActiveRootParameters.push_back({ 0 });
	mActiveRootParameters[index].tableValue.ptr = table.ptr;

	mCommandList->SetDescriptorHeaps(1, &heap);
	mCommandList->SetGraphicsRootDescriptorTable(index, table);
}

bool CommandList::IsKeywordEnabled(jstring keyword) {
	for (unsigned int i = 0; i < mState.keywords.size(); i++)
		if (mState.keywords[i] == keyword)
			return true;
	return false;
}
void CommandList::EnableKeyword(jstring keyword) {
	if (IsKeywordEnabled(keyword)) return;
	mState.keywords.push_back(keyword);
}
void CommandList::DisableKeyword(jstring keyword) {
	for (unsigned int i = 0; i < mState.keywords.size(); i++)
		if (mState.keywords[i] == keyword) {
			mState.keywords.remove(i);
			break;
		}
}
void CommandList::SetKeywords(jvector<jstring> &keywords) {
	mState.keywords = keywords;
}

void CommandList::SetGlobalFloat(jstring param, float val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_FLOAT;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_FLOAT;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalFloat2(jstring param, XMFLOAT2 val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_FLOAT2;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_FLOAT2;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalFloat3(jstring param, XMFLOAT3 val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_FLOAT3;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_FLOAT3;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalFloat4(jstring param, XMFLOAT4 val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_FLOAT4;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_FLOAT4;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalColor3(jstring param, XMFLOAT3 val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_COLOR3;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_COLOR3;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalColor4(jstring param, XMFLOAT4 val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_COLOR4;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_COLOR4;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalInt(jstring param, int val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_INT;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_INT;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalInt2(jstring param, XMINT2 val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_INT2;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_INT2;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalInt3(jstring param, XMINT3 val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_INT3;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_INT3;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalInt4(jstring param, XMINT4 val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_INT4;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_INT4;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalUInt(jstring param, unsigned int val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_UINT;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_UINT;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalUInt2(jstring param, XMUINT2 val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_UINT2;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_UINT2;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalUInt3(jstring param, XMUINT3 val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_UINT3;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_UINT3;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalUInt4(jstring param, XMUINT4 val) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_UINT4;
		p.value.set(val);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_UINT4;
		p.value.set(val);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalTexture(jstring param, shared_ptr<Texture> tex) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_SRV;
		p.value.set(tex);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_SRV;
		p.value.set(tex);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalTable(jstring param, shared_ptr<DescriptorTable> tex) {
	if (mGlobals.count(param)) {
		GlobalParam& p = mGlobals.at(param);
		p.type = SHADER_PARAM_TYPE_TABLE;
		p.value.set(tex);
	} else {
		GlobalParam p;
		p.type = SHADER_PARAM_TYPE_TABLE;
		p.value.set(tex);
		mGlobals.emplace(param, p);
	}
}
void CommandList::SetGlobalCBuffer(jstring param, shared_ptr<ConstantBuffer> cbuf) {
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
}

void CommandList::SetBlendState(D3D12_RENDER_TARGET_BLEND_DESC blend) {
	mState.blendState = blend;
}
void CommandList::DepthWriteEnabled(bool depthWrite) {
	mState.zwrite = depthWrite;
}
void CommandList::DepthTestEnabled(bool depthTest) {
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
	auto pso = mActiveShader->GetOrCreatePSO(mState);
	if (pso != mActivePSO) {
		mActivePSO = pso;
		mCommandList->SetPipelineState(pso.Get());
	}
}
void CommandList::DrawMesh(Mesh &mesh, unsigned int submesh) {
	if (!mesh.mDataUploaded) return;
	if (mesh.mSubmeshes[submesh].mIndexCount == 0) return;

	DrawUserMesh(mesh.Semantics(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	Draw(&mesh.mVertexBufferView, &mesh.mIndexBufferView, mesh.mTopology, 
		mesh.mSubmeshes[submesh].mIndexCount, mesh.mSubmeshes[submesh].mStartIndex, 0);
}
void CommandList::Draw(const D3D12_VERTEX_BUFFER_VIEW* vb, const D3D12_INDEX_BUFFER_VIEW* ib, D3D12_PRIMITIVE_TOPOLOGY topology, 
	UINT indexCount, UINT baseIndex, UINT baseVertex) {

	if (topology != mActiveTopology) {
		mCommandList->IASetPrimitiveTopology(topology);
		mActiveTopology = topology;
	}
	if (vb != mActiveVBO) {
		mCommandList->IASetVertexBuffers(0, 1, vb);
		mActiveVBO = vb;
	}
	if (ib != mActiveIBO) {
		mCommandList->IASetIndexBuffer(ib);
		mActiveIBO = ib;
	}

	mCommandList->DrawIndexedInstanced(indexCount, 1, baseIndex, baseVertex, 0);

	if (topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
		mTrianglesDrawn += indexCount / 3;
	else if (topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP)
		mTrianglesDrawn += indexCount - 2;
}