#include "Mesh.hpp"

#include "Graphics.hpp"
#include "CommandQueue.hpp"
#include "Camera.hpp"
#include "Util.hpp"

#include <map>
#include <fstream>
#include <fbxsdk.h>

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

const D3D12_INPUT_ELEMENT_DESC Vertex::InputElements[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

Mesh::Mesh(std::string name) : Object(name) {}
Mesh::~Mesh() { Release(); }

void Mesh::UploadData(ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** dst,
	ID3D12Resource** intermediate,
	size_t count, size_t stride, const void* data) {

	auto device = Graphics::GetDevice();

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(stride * count),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(dst)));

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(count * stride),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(intermediate)));

	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = data;
	subresourceData.RowPitch = count * stride;
	subresourceData.SlicePitch = subresourceData.RowPitch;

	UpdateSubresources(commandList.Get(), *dst, *intermediate, 0, 0, 1, &subresourceData);
}

void Mesh::LoadObj(LPCSTR path) {
	FILE *file;
	errno_t e = fopen_s(&file, path, "r");
	if (file == NULL) {
		throw std::exception();
		return;
	}

	XMFLOAT3 min(0,0,0);
	XMFLOAT3 max(0,0,0);

	std::vector<XMFLOAT3> tvertices;
	std::vector<XMFLOAT3> tnormals;
	std::vector<XMFLOAT2> tuvs;

	std::map<unsigned long, uint32_t> indexMap;

	int ff = 0;

	unsigned int smooth = 0;
	vertices.clear();
	indices.clear();
	while (1) {
		char header[128];
		unsigned int res = fscanf_s(file, "%s", header, 128);
		if (res == EOF) break;
		if (strcmp(header, "v") == 0) {
			// read vertex
			XMFLOAT3 v;
			fscanf_s(file, "%f %f %f\n", &v.x, &v.y, &v.z);
			tvertices.push_back(v);
			max.x = fmax(max.x, v.x);
			max.y = fmax(max.y, v.y);
			max.z = fmax(max.z, v.z);
			min.x = fmin(min.x, v.x);
			min.y = fmin(min.y, v.y);
			min.z = fmin(min.z, v.z);
		} else if (strcmp(header, "vn") == 0) {
			// read normal
			XMFLOAT3 n;
			fscanf_s(file, "%f %f %f\n", &n.x, &n.y, &n.z);
			tnormals.push_back(n);
		} else if (strcmp(header, "vt") == 0) {
			// read tex coord
			XMFLOAT2 t;
			fscanf_s(file, "%f %f\n", &t.x, &t.y);
			tuvs.push_back(t);
		} else if (strcmp(header, "s") == 0) {
			// smoothing?
			fscanf_s(file, "%d\n", &smooth);
		} else if (strcmp(header, "f") == 0) {
			// read face
			unsigned int iv0, in0;
			unsigned int iv1, in1;
			unsigned int iv2, in2;
			fscanf_s(file, "%u//%u %u//%u %u//%u\n", &iv0, &in0, &iv1, &in1, &iv2, &in2);

			iv0--;
			iv1--;
			iv2--;
			in0--;
			in1--;
			in2--;

			unsigned long h0 = ((unsigned long)iv0 + (unsigned long)in0)*((unsigned long)iv0 + (unsigned long)in0 + 1) / 2 + (unsigned long)in0;
			unsigned long h1 = ((unsigned long)iv1 + (unsigned long)in1)*((unsigned long)iv1 + (unsigned long)in1 + 1) / 2 + (unsigned long)in1;
			unsigned long h2 = ((unsigned long)iv2 + (unsigned long)in2)*((unsigned long)iv2 + (unsigned long)in2 + 1) / 2 + (unsigned long)in2;

			if (indexMap.count(h0) == 0) {
				uint32_t index0 = (uint32_t)vertices.size();
				indexMap.emplace(h0, index0);
				indices.push_back(index0);
				Vertex v0(tvertices[iv0], tnormals[in0], XMFLOAT2(0, 0));
				vertices.push_back(v0);
			} else
				indices.push_back(indexMap.at(h0));

			if (indexMap.count(h1) == 0) {
				uint32_t index1 = (uint32_t)vertices.size();
				indexMap.emplace(h1, index1);
				indices.push_back(index1);
				Vertex v1(tvertices[iv1], tnormals[in1], XMFLOAT2(0, 0));
				vertices.push_back(v1);
			} else
				indices.push_back(indexMap.at(h1));

			if (indexMap.count(h2) == 0) {
				uint32_t index2 = (uint32_t)vertices.size();
				indexMap.emplace(h2, index2);
				indices.push_back(index2);
				Vertex v2(tvertices[iv2], tnormals[in2], XMFLOAT2(0, 0));
				vertices.push_back(v2);
			} else
				indices.push_back(indexMap.at(h2));
		}
	}

	fclose(file);

	m_IndexCount = (UINT)indices.size();

	tvertices.clear();
	tnormals.clear();
	tuvs.clear();

	char str[128];
	sprintf_s(str, "Loaded %s, %d vertices, %d faces (%f, %f, %f)\n", path, (int)vertices.size(), (int)indices.size() / 3, max.x - min.x, max.y - min.y, max.z - min.z);
	OutputDebugString(str);

}


XMVECTOR FbxToDirectX(FbxVector4 vec) {
	return { (float)vec[0], (float)vec[1], (float)vec[2], (float)vec[3] };
}
XMVECTOR FbxToDirectX(FbxVector2 vec) {
	return { (float)vec[0], (float)vec[1], 0.0f, 0.0f };
}
void Mesh::LoadFbx(LPCSTR path) {
	FbxManager* sdkManager = FbxManager::Create();
	FbxImporter* importer = FbxImporter::Create(sdkManager, "");
	if (!importer->Initialize(path, -1, sdkManager->GetIOSettings())) {
		OutputDebugString("Failed to import file: ");
		OutputDebugString(importer->GetStatus().GetErrorString());
		OutputDebugString("\n");
	}

	FbxScene *scene = FbxScene::Create(sdkManager, "import scene");
	importer->Import(scene);
	importer->Destroy();

	FbxNode *rootNode = scene->GetRootNode();
	if (rootNode) {
		for (int i = 0; i < rootNode->GetChildCount(); i++) {
			FbxNode *child = rootNode->GetChild(i);
			FbxNodeAttribute *attrib = child->GetNodeAttribute();

			if (attrib == NULL || attrib->GetAttributeType() != FbxNodeAttribute::eMesh) continue;

			vertices.clear();
			indices.clear();

			FbxMesh *mesh = (FbxMesh*)attrib;
			if (!mesh->IsTriangleMesh()) {
				FbxGeometryConverter clsConverter(sdkManager);
				mesh = (FbxMesh*)clsConverter.Triangulate(mesh, true);
			}

			int nc = 0;
			int nu = 0;

			FbxVector4 *verts = mesh->GetControlPoints();
			for (int i = 0; i < mesh->GetControlPointsCount(); i++) {
				Vertex v;
				XMStoreFloat3(&v.position, FbxToDirectX(verts[i]));

				// TODO: Normals and UVs aren't read in
				// Normal
				for (int j = 0; j < mesh->GetElementNormalCount(); j++) {
					FbxGeometryElementNormal *norms = mesh->GetElementNormal(j);
					if (norms->GetMappingMode() == FbxGeometryElement::eByControlPoint && norms->GetReferenceMode() == FbxGeometryElement::eDirect) {
						XMStoreFloat3(&v.normal, FbxToDirectX(norms->GetDirectArray().GetAt(i)));
					}
				}
				// UVs
				if (mesh->GetElementUVCount() >= 1) {
					FbxGeometryElementUV *uvs = mesh->GetElementUV(0);
					if (uvs->GetMappingMode() == FbxGeometryElement::eByControlPoint && uvs->GetReferenceMode() == FbxGeometryElement::eDirect) {
						XMStoreFloat2(&v.uv, FbxToDirectX(uvs->GetDirectArray().GetAt(i)));
					}
				}
				vertices.push_back(v);
			}

			for (int i = 0; i < mesh->GetPolygonCount(); i++) {
				int s = mesh->GetPolygonSize(i);
				for (int j = 0; j < s; j++) {
					if (s < 3)
						indices.push_back(mesh->GetPolygonVertex(i, j));
					else {
						indices.push_back(mesh->GetPolygonVertex(i, j-2));
						indices.push_back(mesh->GetPolygonVertex(i, j-1));
						indices.push_back(mesh->GetPolygonVertex(i, j));
					}
				}
			}
		}
	}
	sdkManager->Destroy();


	m_IndexCount = (UINT)indices.size();

	char str[128];
	sprintf_s(str, "Loaded %s, %d vertices, %d tris\n", path, (int)vertices.size(), (int)indices.size() / 3);
	OutputDebugString(str);
}

void Mesh::LoadCube(float s) {
	vertices = {
		// top
		{ XMFLOAT3( s, s,  s), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(1, 1) }, // 0
		{ XMFLOAT3(-s, s,  s), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(0, 1) }, // 1
		{ XMFLOAT3( s, s, -s), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(1, 0) }, // 2
		{ XMFLOAT3(-s, s, -s), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(0, 0) }, // 3
		
		// bottom
		{ XMFLOAT3( s, -s,  s), XMFLOAT3(0, -1.0f, 0), XMFLOAT2(1, 1) }, // 4
		{ XMFLOAT3( s, -s, -s), XMFLOAT3(0, -1.0f, 0), XMFLOAT2(1, 0) }, // 5
		{ XMFLOAT3(-s, -s,  s), XMFLOAT3(0, -1.0f, 0), XMFLOAT2(0, 1) }, // 6
		{ XMFLOAT3(-s, -s, -s), XMFLOAT3(0, -1.0f, 0), XMFLOAT2(0, 0) }, // 7
		
		// right
		{ XMFLOAT3(s,  s,  s), XMFLOAT3(1.0f, 0, 0), XMFLOAT2(1, 1) }, // 8
		{ XMFLOAT3(s, -s,  s), XMFLOAT3(1.0f, 0, 0), XMFLOAT2(1, 0) }, // 9
		{ XMFLOAT3(s,  s, -s), XMFLOAT3(1.0f, 0, 0), XMFLOAT2(0, 1) }, // 10
		{ XMFLOAT3(s, -s, -s), XMFLOAT3(1.0f, 0, 0), XMFLOAT2(0, 0) }, // 11

		// left
		{ XMFLOAT3(-s,  s,  s), XMFLOAT3(-1.0f, 0, 0), XMFLOAT2(1, 1) }, // 12
		{ XMFLOAT3(-s, -s,  s), XMFLOAT3(-1.0f, 0, 0), XMFLOAT2(1, 0) }, // 13
		{ XMFLOAT3(-s,  s, -s), XMFLOAT3(-1.0f, 0, 0), XMFLOAT2(0, 1) }, // 14
		{ XMFLOAT3(-s, -s, -s), XMFLOAT3(-1.0f, 0, 0), XMFLOAT2(0, 0) }, // 15

		// forward
		{ XMFLOAT3( s,  s, -s), XMFLOAT3(0, 0, -1.0f), XMFLOAT2(1, 1) }, // 16
		{ XMFLOAT3(-s,  s, -s), XMFLOAT3(0, 0, -1.0f), XMFLOAT2(1, 0) }, // 17
		{ XMFLOAT3( s, -s, -s), XMFLOAT3(0, 0, -1.0f), XMFLOAT2(0, 1) }, // 18
		{ XMFLOAT3(-s, -s, -s), XMFLOAT3(0, 0, -1.0f), XMFLOAT2(0, 0) }, // 19

		// back
		{ XMFLOAT3( s,  s, s), XMFLOAT3(0, 0, 1.0f), XMFLOAT2(1, 1) }, // 20
		{ XMFLOAT3( s, -s, s), XMFLOAT3(0, 0, 1.0f), XMFLOAT2(0, 1) }, // 21
		{ XMFLOAT3(-s,  s, s), XMFLOAT3(0, 0, 1.0f), XMFLOAT2(1, 0) }, // 22
		{ XMFLOAT3(-s, -s, s), XMFLOAT3(0, 0, 1.0f), XMFLOAT2(0, 0) }, // 23
	};
	indices = {
		0,  2,  1,  1,  2,  3,
		4,  6,  5,  5,  6,  7,
		8,  9,  10, 10, 9, 11,
		12, 14, 13, 13, 14, 15,
		16, 18, 17, 17, 18, 19,
		20, 22, 21, 21, 22, 23,
	};
	m_IndexCount = (UINT)indices.size();
}

void Mesh::Create() {
	auto device = Graphics::GetDevice();
	auto commandList = Graphics::GetCommandQueue()->GetCommandList();

	ComPtr<ID3D12Resource> ivb;
	UploadData(commandList, &m_VertexBuffer, &ivb, vertices.size(), sizeof(Vertex), vertices.data());
	ComPtr<ID3D12Resource> iib;
	UploadData(commandList, &m_IndexBuffer, &iib, indices.size(), sizeof(uint32_t), indices.data());

	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.SizeInBytes = (UINT)vertices.size() * sizeof(Vertex);
	m_VertexBufferView.StrideInBytes = sizeof(Vertex);
	
	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.SizeInBytes = (UINT)indices.size() * sizeof(uint32_t);
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

	Graphics::TransitionResource(commandList, m_VertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	Graphics::TransitionResource(commandList, m_IndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	ivb->SetName(L"Vertex Buffer Upload");
	m_VertexBuffer->SetName(L"Vertex Buffer");
	iib->SetName(L"Index Buffer Upload");
	m_IndexBuffer->SetName(L"Index Buffer");

	auto commandQueue = Graphics::GetCommandQueue();
	auto fenceValue = commandQueue->Execute(commandList);
	commandQueue->WaitForFenceValue(fenceValue);

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

void Mesh::Release() {
	CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
	m_CBuffer->Unmap(0, &readRange);
	m_CBuffer.Reset();
	m_VertexBuffer.Reset();
	m_IndexBuffer.Reset();
}
bool Mesh::UpdateTransform() {
	if (!Object::UpdateTransform()) return false;

	XMStoreFloat4x4(&m_ObjectBufferData.ObjectToWorld, ObjectToWorld());
	XMStoreFloat4x4(&m_ObjectBufferData.WorldToObject, WorldToObject());
	memcpy(m_MappedCBuffer, &m_ObjectBufferData, sizeof(ObjectBuffer));

	return true;
}
void Mesh::Draw(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	UpdateTransform();

	commandList->SetGraphicsRootConstantBufferView(0, GetCBuffer());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	commandList->IASetIndexBuffer(&m_IndexBufferView);
	commandList->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
}