#pragma once

#include <IJaeGame.hpp>

class TestGame : public IJaeGame {
public:
	~TestGame();

	void Initialize();
	void OnResize();
	void Update(double totalTime, double deltaTime);
	void Render(std::shared_ptr<CommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv);
	void DoFrame();

private:
	double mfps;
};
