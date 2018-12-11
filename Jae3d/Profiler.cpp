#include "Profiler.hpp"
#include <chrono>

using namespace Profiler;

int frameCount = 256;
ProfilerFrame frames[256];
unsigned long curFrame = 0;
double lastFrameTime = 0;

ProfilerSample *currentSample; // most recent sample created from the most recent BeginSample call

const std::chrono::high_resolution_clock timer;
auto start = timer.now();

Profiler::ProfilerSample::~ProfilerSample() {
	for (int i = 0; i < children.size(); i++)
		delete children[i];
	children.clear();
}

void Profiler::BeginSample(jwstring name) {
	ProfilerSample *s = new ProfilerSample(name, (timer.now() - start).count() * 1e-9);
	if (currentSample) {
		s->parent = currentSample;
		currentSample->children.push_back(s);
	} else {
		frames[curFrame % frameCount].samples.push_back(s);
	}
	currentSample = s;
}
void Profiler::EndSample() {
	currentSample->endTime = (timer.now() - start).count() * 1e-9;
	currentSample = currentSample->parent;
}

void Profiler::FrameStart() {
	int i = curFrame % frameCount;
	for (int j = 0; j < frames[i].samples.size(); j++)
		delete(frames[i].samples[j]);
	frames[i].samples.clear();
	frames[i].index = curFrame;
	frames[i].startTime = (timer.now() - start).count() * 1e-9;
}
void Profiler::FrameEnd() {
	frames[curFrame % frameCount].endTime = (timer.now() - start).count() * 1e-9;
	lastFrameTime = frames[curFrame % frameCount].endTime - frames[curFrame % frameCount].startTime;
	currentSample = nullptr;
	curFrame++;
}
double Profiler::LastFrameTime() {
	return lastFrameTime;
}

void PrintSample(wchar_t *buffer, int size, int &c, ProfilerSample *s, int tabLevel) {
	for (int i = 0; i < tabLevel; i++)
		c += swprintf_s(buffer + c, size - c, L"  ");
	c += swprintf_s(buffer + c, size - c, L"%s: %.2fms\n", s->name.c_str(), (s->endTime - s->startTime) * 1000);
	for (int i = 0; i < s->children.size(); i++)
		PrintSample(buffer, size, c, s->children[i], tabLevel + 1);
}

void Profiler::PrintLastFrame(wchar_t *buffer, int size) {
	ProfilerFrame *pf = &frames[(curFrame - 1) % frameCount];
	int c = 0;
	c += swprintf_s(buffer, size, L"Frame %d: %.2fms\n", curFrame - 1, (pf->endTime - pf->startTime) * 1000);

	for (int i = 0; i < pf->samples.size(); i++)
		PrintSample(buffer, size, c, pf->samples[i], 1);
}