#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "Graphics.h"

class Game {
private:
	Graphics *graphics;
public:
	float m_fps;
	virtual void Initialize(Graphics *graphics);
	virtual void Update(double totalTime, double deltaTime);
	virtual void Render(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
};