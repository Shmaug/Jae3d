#include "Mesh.hpp"

#include "Graphics.hpp"
#include "CommandQueue.hpp"
#include "Camera.hpp"
#include "Util.hpp"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.hpp"
#include <fbxsdk.h>

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

const D3D12_INPUT_ELEMENT_DESC Vertex::InputElements[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

Mesh::Mesh(string name) : m_Name(name) {}
Mesh::~Mesh() { Release(); }

void Mesh::UploadData(ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** dst, ID3D12Resource** intermediate,
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

void Mesh::LoadObj(string path, float scale) {
	vertices.clear();
	indices.clear();
	m_IndexCount = 0;

	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str());

	if (!err.empty()) {
		OutputDebugString(err.c_str());
		OutputDebugString("\n");
	}
	if (!ret) return;

	vertices.reserve(attrib.vertices.size() / 3);
	indices.reserve(attrib.vertices.size() / 3);

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];

			indices.push_back((int)vertices.size());
			indices.push_back((int)vertices.size() + 2);
			indices.push_back((int)vertices.size() + 1);

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				Vertex vertex;
				if (3 * idx.vertex_index + 2 < attrib.vertices.size()) {
					vertex.position.x = attrib.vertices[3 * idx.vertex_index + 0] * scale;
					vertex.position.y = attrib.vertices[3 * idx.vertex_index + 1] * scale;
					vertex.position.z = attrib.vertices[3 * idx.vertex_index + 2] * scale;
				}
				if (3 * idx.normal_index+2 < attrib.normals.size()) {
					vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
					vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
					vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];
				}
				if (2 * idx.texcoord_index + 1 < attrib.texcoords.size()) {
					vertex.uv.x = attrib.texcoords[2 * idx.texcoord_index + 0];
					vertex.uv.y = attrib.texcoords[2 * idx.texcoord_index + 1];
				}
				vertices.push_back(vertex);
			}
			index_offset += fv;
			
			//shapes[s].mesh.material_ids[f];
		}
	}

	m_IndexCount = (UINT)indices.size();

	char str[128];
	sprintf_s(str, "Loaded %s, %d vertices, %d faces\n", path.c_str(), (int)vertices.size(), (int)indices.size() / 3);
	OutputDebugString(str);
}

XMVECTOR FbxToDirectX(FbxVector4 vec) {
	return { (float)vec[0], (float)vec[1], (float)vec[2], (float)vec[3] };
}
XMVECTOR FbxToDirectX(FbxVector2 vec) {
	return { (float)vec[0], (float)vec[1], 0.0f, 0.0f };
}
void Mesh::LoadFbx(string path, float scale) {
	FbxManager* sdkManager = FbxManager::Create();
	FbxImporter* importer = FbxImporter::Create(sdkManager, "");
	if (!importer->Initialize(path.c_str(), -1, sdkManager->GetIOSettings())) {
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
				XMStoreFloat3(&v.position, FbxToDirectX(verts[i]) * scale);

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
				for (int j = 2; j < s; j++) {
					indices.push_back(mesh->GetPolygonVertex(i, j-2));
					indices.push_back(mesh->GetPolygonVertex(i, j));
					indices.push_back(mesh->GetPolygonVertex(i, j-1));
				}
			}
		}
	}
	sdkManager->Destroy();


	m_IndexCount = (UINT)indices.size();

	char str[128];
	sprintf_s(str, "Loaded %s, %d vertices, %d tris\n", path.c_str(), (int)vertices.size(), (int)indices.size() / 3);
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
		0,  1,  2,  2,  1,  3,
		4,  5,  6,  6,  5,  7,
		8,  10, 9,  9,  10, 11,
		12, 13, 14, 14, 13, 15,
		16, 17, 18, 18, 17, 19,
		20, 21, 22, 22, 21, 23,
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
}

void Mesh::Release() {
	m_VertexBuffer.Reset();
	m_IndexBuffer.Reset();
}
void Mesh::Draw(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	commandList->IASetIndexBuffer(&m_IndexBufferView);
	commandList->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
}