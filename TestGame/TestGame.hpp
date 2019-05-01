#pragma once

#include <IJaeGame.hpp>
#include <Camera.hpp>
#include <Scene.hpp>
#include <Font.hpp>
#include <AssetDatabase.hpp>

class Lighting;

class TestGame : public IJaeGame {
public:
	TestGame() : mFrameTimeIndex(0), mFpsYaw(0), mFpsPitch(0), mDebugDraw(false), mWireframe(false), mLoading(true), mCursorVisible(true), mPerfOverlay(false) {};
	~TestGame();

	void Initialize() override;
	void OnResize() override;
	void DoFrame() override;

private:
	double mfps;

	void InitializeScene();
	void Update(double totalTime, double deltaTime);
	void Render(std::shared_ptr<CommandList> commandList);

	std::shared_ptr<Lighting> mLighting;
	std::shared_ptr<Scene> mScene;
	std::shared_ptr<Camera> mCamera;
	std::shared_ptr<Font> mArialFont;

	wchar_t mPerfBuffer[8192];

	float mFrameTimes[128];
	unsigned int mFrameTimeIndex;

	float mFpsYaw;
	float mFpsPitch;
	bool mLoading;
	bool mCursorVisible;

	bool mPerfOverlay;
	bool mDebugDraw;
	bool mWireframe;

	jvector<AssetDatabase::LoadAssetOperation*> mLoadingOperations;
};