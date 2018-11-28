#pragma once

#include <memory>

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>

class Camera;
class Mesh;
class Shader;
class Material;

class CommandList {
public:
	CommandList(_WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, _WRL::ComPtr<ID3D12CommandAllocator> allocator);
	~CommandList();

	void Reset(_WRL::ComPtr<ID3D12CommandAllocator> allocator);

	void TransitionResource(_WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);

	void SetShader(std::shared_ptr<Shader> shader);
	void SetMaterial(std::shared_ptr<Material> material);
	void SetCamera(std::shared_ptr<Camera> camera);

	void DrawMesh(Mesh &mesh);

	_WRL::ComPtr<ID3D12GraphicsCommandList2> D3DCommandList() const { return m_CommandList; }

private:
	std::shared_ptr<Material> m_ActiveMaterial;
	std::shared_ptr<Shader> m_ActiveShader;
	std::shared_ptr<Camera> m_ActiveCamera;

	_WRL::ComPtr<ID3D12GraphicsCommandList2> m_CommandList;
};

