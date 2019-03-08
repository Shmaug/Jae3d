#include "Profiler.hpp"
#include <chrono>
#include <cassert>

using namespace Profiler;

int frameCount = 256;
ProfilerSample frames[256];
unsigned long curFrame = 0;
double lastFrameTime = 0;

ProfilerSample* currentSample = nullptr; // most recent sample created from the most recent BeginSample call

const std::chrono::high_resolution_clock timer;
auto start = timer.now();

void Profiler::BeginSample(const jwstring& name, bool resume) {
	if (resume) {
		for (unsigned int i = 0; i < currentSample->children.size(); i++)
			if (currentSample->children[i].name == name) {
				currentSample->children[i].startTime = (timer.now() - start).count() * 1e-9;
				currentSample = &currentSample->children[i];
				return;
			}
	}
	ProfilerSample& s = currentSample->children.push_back(ProfilerSample(name, (timer.now() - start).count() * 1e-9));
	s.parent = currentSample;
	currentSample = &s;
}
void Profiler::EndSample() {
	assert(currentSample->parent);
	double now = (timer.now() - start).count() * 1e-9;
	currentSample->time += now - currentSample->startTime;
	currentSample = currentSample->parent;
}

void Profiler::FrameStart() {
	int i = curFrame % frameCount;
	wchar_t b[16];
	_itow_s(curFrame, b, 16, 10);
	frames[i].name = jwstring(L"Frame ") + b;
	frames[i].children.clear();
	frames[i].startTime = (timer.now() - start).count() * 1e-9;
	currentSample = &frames[i];
}
void Profiler::FrameEnd() {
	double now = (timer.now() - start).count() * 1e-9;
	lastFrameTime = frames[curFrame % frameCount].time = now - frames[curFrame % frameCount].startTime;
	curFrame++;
}
double Profiler::LastFrameTime() {
	return lastFrameTime;
}

void PrintSample(wchar_t *buffer, int size, int &c, ProfilerSample *s, int tabLevel) {
	for (int i = 0; i < tabLevel; i++)
		c += swprintf_s(buffer + c, size - c, L"  ");

	c += swprintf_s(buffer + c, size - c, L"%s: %.2fms\n", s->name.c_str(), s->time * 1000);
	for (int i = 0; i < s->children.size(); i++)
		PrintSample(buffer, size, c, &s->children[i], tabLevel + 1);
}
void Profiler::PrintLastFrame(wchar_t *buffer, int size) {
	int c = 0;
	PrintSample(buffer, size, c, &frames[(curFrame - 1) % frameCount], 1);
}