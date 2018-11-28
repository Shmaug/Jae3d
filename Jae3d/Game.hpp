#pragma once

#include <memory>

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>

class CommandList;

class Game {
public:
	~Game();

	double m_fps;
	void Initialize();
	void Update(double totalTime, double deltaTime);
	void Render(std::shared_ptr<CommandList> commandList);
	void OnResize();
};