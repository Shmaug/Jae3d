#pragma once

#include <d3d12.h>
#include <wrl.h>
#define _WRL Microsoft::WRL

class Game {
public:
	~Game();

	double m_fps;
	void Initialize(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);
	void Update(double totalTime, double deltaTime);
	void Render(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnResize();
};