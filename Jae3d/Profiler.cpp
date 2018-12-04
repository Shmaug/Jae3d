#include "Profiler.hpp"
#include <chrono>

using namespace Profiler;

int frameCount = 256;
ProfilerFrame frames[256];
unsigned long curFrame = 0;

ProfilerSample *currentSample; // most recent sample created from the most recent BeginSample call

const std::chrono::high_resolution_clock timer;
auto start = timer.now();

Profiler::ProfilerSample::~ProfilerSample() {
	for (auto s : children)
		delete(s);
	children.clear();
}

void Profiler::BeginSample(LPCSTR name) {
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
	for (auto s : frames[i].samples)
		delete(s);
	frames[i].samples.clear();
	frames[i].index = curFrame;
	frames[i].startTime = (timer.now() - start).count() * 1e-9;
}
void Profiler::FrameEnd() {
	frames[curFrame % frameCount].endTime = (timer.now() - start).count() * 1e-9;
	currentSample = nullptr;
	curFrame++;
}

void PrintSample(char *buffer, int size, int &c, ProfilerSample *s, int tabLevel) {
	for (int i = 0; i < tabLevel; i++)
		c += sprintf_s(buffer + c, size - c, "  ");
	c += sprintf_s(buffer + c, size - c, "%s: %.2fms\n", s->name, (s->endTime - s->startTime) * 1000);
	for (auto cs : s->children)
		PrintSample(buffer, size, c, cs, tabLevel + 1);
}

void Profiler::PrintLastFrame(char *buffer, int size) {
	ProfilerFrame *pf = &frames[(curFrame - 1) % frameCount];
	int c = 0;
	c += sprintf_s(buffer, size, "Frame %d: %.2fms\n", curFrame - 1, (pf->endTime - pf->startTime) * 1000);

	for (auto s : pf->samples)
		PrintSample(buffer, size, c, s, 1);
}