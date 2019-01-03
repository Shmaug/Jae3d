#pragma once

#include <IJaeGame.hpp>
#include <Camera.hpp>

class TestGame : public IJaeGame {
public:
	~TestGame();

	void Initialize();
	void OnResize();
	void Update(double totalTime, double deltaTime);
	void Render(std::shared_ptr<Camera> camera, std::shared_ptr<CommandList> commandList);
	void DoFrame();

private:
	double mfps;
};

