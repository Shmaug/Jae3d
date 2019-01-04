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

shared_ptr<Mesh> quadMesh;
shared_ptr<Shader> textureShader;
shared_ptr<Shader> meshShader;
shared_ptr<Camera> camera;

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

	jvector<jstring> keywords;
	meshShader = shared_ptr<Shader>(new Shader(L"Mesh Shader"));
	meshShader->CompileShaderStage(meshrootsig, "RootSig", SHADER_STAGE_ROOTSIG, keywords);
	meshShader->CompileShaderStage(meshshader, "vsmain", SHADER_STAGE_VERTEX, keywords);
	meshShader->CompileShaderStage(meshshader, "psmain", SHADER_STAGE_PIXEL, keywords);
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
	textureShader->CompileShaderStage(texrootsig, "RootSig", SHADER_STAGE_ROOTSIG, keywords);
	textureShader->CompileShaderStage(texshader, "vsmain", SHADER_STAGE_VERTEX, keywords);
	textureShader->CompileShaderStage(texshader, "psmain", SHADER_STAGE_PIXEL, keywords);
	textureShader->Upload();

	quadMesh = shared_ptr<Mesh>(new Mesh(L"Quad"));
	quadMesh->LoadQuad(1.0f);
	quadMesh->UploadStatic();

	camera = shared_ptr<Camera>(new Camera(L"Camera"));
}

void Viewport::Resize() {
	if (!Graphics::IsInitialized()) return;
	Graphics::GetWindow()->Resize();
	camera->PixelWidth(Graphics::GetWindow()->GetWidth());
	camera->PixelHeight(Graphics::GetWindow()->GetHeight());
	camera->CreateRenderBuffers();
	DoFrame();
}
void Viewport::DoFrame() {
	auto commandQueue = Graphics::GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue->GetCommandList(Graphics::CurrentFrameIndex());
	auto d3dCommandList = commandList->D3DCommandList();
	auto window = Graphics::GetWindow();

	window->PrepareRender(commandList);
	commandList->SetCamera(camera);

	if (shownAsset.asset) {
		switch (shownAsset.asset->TypeId()) {
		case ASSET_TYPE_TEXTURE:
		{
			shared_ptr<Texture> tex = static_pointer_cast<Texture>(shownAsset.asset);

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
			if (window->GetWidth() > window->GetHeight())
				scale.x /= (float)window->GetWidth() / window->GetHeight();
			else
				scale.y *= (float)window->GetWidth() / window->GetHeight();
			commandList->SetShader(textureShader);
			d3dCommandList->SetGraphicsRoot32BitConstants(1, 2, &scale, 0);
			ID3D12DescriptorHeap* heap = { tex->GetSRVDescriptorHeap().Get() };
			d3dCommandList->SetDescriptorHeaps(1, &heap);
			d3dCommandList->SetGraphicsRootDescriptorTable(0, tex->GetSRVGPUDescriptor());
			commandList->DrawMesh(*quadMesh);
			break;
		}
		case ASSET_TYPE_MESH:
		{
			commandList->SetBlendState(BLEND_STATE_DEFAULT);

			XMVECTOR cp = XMVectorSet(.5f, 1, 1, 0);
			XMVECTOR fwd = -cp;
			XMVECTOR up = XMVector3Cross(XMVector3Cross(fwd, XMVectorSet(0, 1, 0, 0)), fwd);

			XMMATRIX o2w = XMMatrixIdentity();
			XMVECTOR det = XMMatrixDeterminant(o2w);

			XMMATRIX v = XMMatrixLookToLH(cp, fwd, up);
			XMMATRIX p = XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(70.0f), (float)window->GetWidth() / window->GetHeight(), .01f, 100.0f);

			XMFLOAT4X4 w2o;
			XMFLOAT4X4 mvp;
			XMStoreFloat4x4(&w2o, XMMatrixInverse(&det, o2w));
			XMStoreFloat4x4(&mvp, v * p);

			shared_ptr<Mesh> mesh = static_pointer_cast<Mesh>(shownAsset.asset);
			commandList->SetShader(meshShader);
			d3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &w2o, 0);
			d3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &mvp, 16);
			commandList->DrawMesh(*mesh);
			break;
		}
		case ASSET_TYPE_FONT:
		{
			float w = (float)Graphics::GetWindow()->GetWidth();
			float h = (float)Graphics::GetWindow()->GetHeight();

			auto sb = Graphics::GetSpriteBatch();
			sb->DrawText(static_pointer_cast<Font>(shownAsset.asset), XMFLOAT4(0, h * .3f, w, h), 1.0f, {1,1,1,1}, L"the quick brown fox jumped\nover the lazy brown dog.");
			sb->Flush(commandList);
		}
		}
	}

	ID3D12Resource* camrt = camera->RenderBuffer().Get();
	ID3D12Resource* winrt = window->RenderBuffer().Get();
	commandList->TransitionResource(camrt, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	commandList->TransitionResource(winrt, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_DEST);
	d3dCommandList->ResolveSubresource(winrt, 0, camrt, 0, camera->RenderFormat());
	commandList->TransitionResource(camrt, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->TransitionResource(winrt, D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

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

void Viewport::Show(AssetMetadata &asset) {
	if (asset.asset) {
		switch (asset.asset->TypeId()) {
		case ASSET_TYPE_TEXTURE:
			ShowTexture(static_pointer_cast<Texture>(asset.asset));
			break;
		case ASSET_TYPE_MESH:
			ShowMesh(static_pointer_cast<Mesh>(asset.asset));
			break;
		case ASSET_TYPE_FONT:
			ShowFont(static_pointer_cast<Font>(asset.asset));
			break;
		}
	}
	shownAsset = asset;
	DoFrame();
}