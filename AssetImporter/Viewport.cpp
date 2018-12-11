#include "Viewport.hpp"

#include <Graphics.hpp>
#include <SpriteBatch.hpp>
#include <CommandQueue.hpp>
#include <CommandList.hpp>
#include <Window.hpp>
#include <AssetDatabase.hpp>
#include <MeshRenderer.hpp>
#include <ConstantBuffer.hpp>
#include <Camera.hpp>
#include <Material.hpp>
#include <Mesh.hpp>
#include <Texture.hpp>
#include <Shader.hpp>
#include <Font.hpp>
#include <AssetFile.hpp>

#include "ShaderImporter.hpp"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

Viewport::Viewport() {}
Viewport::~Viewport() {}

shared_ptr<Mesh> quadMesh;
shared_ptr<Shader> textureShader;
shared_ptr<Shader> meshShader;

shared_ptr<Asset> shownAsset;

void Viewport::Init(HWND hwnd){
	Graphics::Initialize(hwnd, 2);
	
	const char meshrootsig[] = 
" #define RootSig "
" \"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS | DENY_HULL_SHADER_ROOT_ACCESS ),"
" RootConstants(num32BitConstants=32, b0, visibility=SHADER_VISIBILITY_VERTEX)\""
;
	const char meshshader[] =
" struct ObjectBuffer { float4x4 WorldToObject; float4x4 MVP; }; "
" ConstantBuffer<ObjectBuffer> Object : register(b0); "
" struct v2f {"
" 	float4 pos : SV_Position;"
" 	float3 normal : NORMAL;"
" };"
" v2f vsmain(float3 vertex : POSITION, float3 normal : NORMAL) {"
"       v2f o;"
" 		o.pos = mul(Object.MVP, float4(vertex, 1.0));"
" 		o.normal = mul(float4(normal, 1.0), Object.WorldToObject).xyz;"
" 		return o;"
" 	}"
" float4 psmain(v2f i) : SV_Target {"
"		return float4(normalize(i.normal) * .5 + .5, 1.0);"
" }"
;

	meshShader = shared_ptr<Shader>(new Shader(L"Mesh Shader"));
	meshShader->CompileShaderStage(meshrootsig, "RootSig", SHADERSTAGE_ROOTSIG);
	meshShader->CompileShaderStage(meshshader, "vsmain", SHADERSTAGE_VERTEX);
	meshShader->CompileShaderStage(meshshader, "psmain", SHADERSTAGE_PIXEL);
	meshShader->Upload();
	
	const char texrootsig[] = 
" #define RootSig "
" \"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS | DENY_HULL_SHADER_ROOT_ACCESS),"
" DescriptorTable(SRV(t0), visibility=SHADER_VISIBILITY_PIXEL),"
" RootConstants(num32BitConstants=2, b0, visibility=SHADER_VISIBILITY_VERTEX),"
" StaticSampler(s0,"
"addressU = TEXTURE_ADDRESS_CLAMP, "
"addressV = TEXTURE_ADDRESS_CLAMP, "
"addressW = TEXTURE_ADDRESS_CLAMP, "
"minLOD = 0.f, maxLOD = 1.f, "
"filter=FILTER_MIN_MAG_MIP_LINEAR, visibility=SHADER_VISIBILITY_PIXEL)\""
;
	const char texshader[] =
" struct ObjectBuffer { float2 Scale; }; "
" ConstantBuffer<ObjectBuffer> Object : register(b0); "
" Texture2D<float4> Tex : register(t0);"
" sampler Sampler : register(s0);"
" struct v2f {"
" 	float4 pos : SV_Position;"
" 	float2 tex : TEXCOORD0;"
" };"
" v2f vsmain(float3 vertex : POSITION, float2 tex : TEXCOORD0) {"
" 		v2f o;"
" 		o.pos = float4(vertex.xy * Object.Scale, 0, 1.0);"
" 		o.tex = tex;"
" 		return o;"
" 	}"
" float4 psmain(v2f i) : SV_Target {"
"		return Tex.Sample(Sampler, i.tex);"
" }"
;

	textureShader = shared_ptr<Shader>(new Shader(L"Texture Shader"));
	textureShader->CompileShaderStage(texrootsig, "RootSig", SHADERSTAGE_ROOTSIG);
	textureShader->CompileShaderStage(texshader, "vsmain", SHADERSTAGE_VERTEX);
	textureShader->CompileShaderStage(texshader, "psmain", SHADERSTAGE_PIXEL);
	textureShader->Upload();

	quadMesh = shared_ptr<Mesh>(new Mesh(L"Quad"));
	quadMesh->LoadQuad(1.0f);
	quadMesh->UploadStatic();
}

void Viewport::Resize() {
	if (!Graphics::IsInitialized()) return;
	Graphics::GetWindow()->Resize();
}
void Viewport::DoFrame() {
	auto commandQueue = Graphics::GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue->GetCommandList(Graphics::CurrentFrameIndex());
	auto d3dCommandList = commandList->D3DCommandList();
	auto window = Graphics::GetWindow();

	window->PrepareRenderTargets(commandList);

	auto rtv = window->GetCurrentRenderTargetView();
	auto dsv = window->GetDepthStencilView();

	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.f, 0.f, (float)window->GetWidth(), (float)window->GetHeight());
	D3D12_RECT sr = { 0, 0, window->GetWidth(), window->GetHeight() };
	commandList->D3DCommandList()->RSSetViewports(1, &vp);
	commandList->D3DCommandList()->RSSetScissorRects(1, &sr);
	commandList->D3DCommandList()->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	DirectX::XMFLOAT4 clearColor = { 37.0f / 255.0f, 37.0f / 255.0f, 38.0f / 255.0f, 1.f };
	commandList->D3DCommandList()->ClearRenderTargetView(rtv, (float*)&clearColor, 0, nullptr);
	commandList->D3DCommandList()->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	if (shownAsset) {
		switch (shownAsset->TypeId()) {
		case AssetFile::TYPEID_TEXTURE:
		{
			shared_ptr<Texture> tex = static_pointer_cast<Texture>(shownAsset);

			switch (tex->AlphaMode()) {
			case ALPHA_MODE_TRANSPARENCY:
				commandList->SetBlendState(BLEND_STATE_ALPHA);
				break;
			case ALPHA_MODE_PREMULTIPLIED:
				commandList->SetBlendState(BLEND_STATE_PREMUL);
				break;
			default:
				commandList->SetBlendState(BLEND_STATE_DEFAULT);
				break;
			}

			XMFLOAT2 scale = tex->Width() > tex->Height() ? XMFLOAT2(1.0f, (float)tex->Height() / tex->Width()) : XMFLOAT2((float)tex->Width() / tex->Height(), 1.0f);
			commandList->SetShader(textureShader);
			d3dCommandList->SetGraphicsRoot32BitConstants(1, 2, &scale, 0);
			ID3D12DescriptorHeap* heaps[] = { tex->GetDescriptorHeap().Get() };
			d3dCommandList->SetDescriptorHeaps(1, heaps);
			d3dCommandList->SetGraphicsRootDescriptorTable(0, tex->GetGPUDescriptor());
			commandList->DrawMesh(*quadMesh);
			break;
		}
		case AssetFile::TYPEID_MESH:
		{
			commandList->SetBlendState(BLEND_STATE_DEFAULT);

			XMVECTOR cp = XMVectorSet(.5f, 1, 1, 0);
			XMVECTOR fwd = -cp;
			XMVECTOR up = XMVector3Cross(XMVector3Cross(fwd, XMVectorSet(0, 1, 0, 0)), fwd);

			XMMATRIX o2w = XMMatrixIdentity();
			XMVECTOR det = XMMatrixDeterminant(o2w);

			XMMATRIX v = XMMatrixLookToLH(cp, fwd, up);
			XMMATRIX p = XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(70.0f), 1.0f, .01f, 100.0f);

			XMFLOAT4X4 w2o;
			XMFLOAT4X4 mvp;
			XMStoreFloat4x4(&w2o, XMMatrixInverse(&det, o2w));
			XMStoreFloat4x4(&mvp, v * p);

			shared_ptr<Mesh> mesh = static_pointer_cast<Mesh>(shownAsset);
			commandList->SetShader(meshShader);
			d3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &w2o, 0);
			d3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &mvp, 16);
			commandList->DrawMesh(*mesh);
			break;
		}
		case AssetFile::TYPEID_FONT:
		{
			/*
			shared_ptr<Texture> tex = static_pointer_cast<Font>(shownAsset)->GetTexture();

			switch (tex->AlphaMode()) {
			case ALPHA_MODE_TRANSPARENCY:
				commandList->SetBlendState(BLEND_STATE_ALPHA);
				break;
			case ALPHA_MODE_PREMULTIPLIED:
				commandList->SetBlendState(BLEND_STATE_PREMUL);
				break;
			default:
				commandList->SetBlendState(BLEND_STATE_DEFAULT);
				break;
			}

			XMFLOAT2 scale = tex->Width() > tex->Height() ? XMFLOAT2(1.0f, (float)tex->Height() / tex->Width()) : XMFLOAT2((float)tex->Width() / tex->Height(), 1.0f);
			commandList->SetShader(textureShader);
			d3dCommandList->SetGraphicsRoot32BitConstants(1, 2, &scale, 0);
			ID3D12DescriptorHeap* heaps[] = { tex->GetDescriptorHeap().Get() };
			d3dCommandList->SetDescriptorHeaps(1, heaps);
			d3dCommandList->SetGraphicsRootDescriptorTable(0, tex->GetGPUDescriptor());
			commandList->DrawMesh(*quadMesh);

			/*/
			float w = (float)Graphics::GetWindow()->GetWidth();
			float h = (float)Graphics::GetWindow()->GetHeight();

			auto sb = Graphics::GetSpriteBatch();
			sb->DrawText(static_pointer_cast<Font>(shownAsset), XMFLOAT4(0, h * .3f, w, h), 1.0f, {1,1,1,1}, L"the quick brown fox jumped\nover the lazy brown dog.");
			sb->Flush(commandList);
			//*/
		}
		}
	}
	window->Present(commandList, commandQueue);
}

void ShowTexture(shared_ptr<Texture> texture) {
	texture->Upload();
}
void ShowMesh(shared_ptr<Mesh> mesh) {
	mesh->UploadStatic();
}
void ShowFont(shared_ptr<Font> font) {
	font->GetTexture()->Upload();
}

void Viewport::Show(std::shared_ptr<Asset> asset) {
	switch (asset->TypeId()) {
	case AssetFile::TYPEID_TEXTURE:
		ShowTexture(static_pointer_cast<Texture>(asset));
		break;
	case AssetFile::TYPEID_MESH:
		ShowMesh(static_pointer_cast<Mesh>(asset));
		break;
	case AssetFile::TYPEID_FONT:
		ShowFont(static_pointer_cast<Font>(asset));
		break;
	}
	shownAsset = asset;
	DoFrame();
}