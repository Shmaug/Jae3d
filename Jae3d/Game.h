#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "Graphics.h"

class Game {
private:
	Graphics *graphics;
public:
	float m_fps;
	void Initialize(Graphics *graphics);
	void Update(double totalTime, double deltaTime);
	void Render(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
};