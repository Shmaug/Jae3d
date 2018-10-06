#include "Profiler.h"
#include <chrono>

using namespace Profiler;

int frameCount = 256;
std::vector<ProfilerFrame> frames(frameCount);
unsigned long curFrame = 0;

ProfilerSample *currentSample;

const std::chrono::high_resolution_clock timer;
auto start = timer.now();

void Profiler::BeginSample(LPCSTR name) {
	ProfilerSample *s = new ProfilerSample(name, (timer.now() - start).count() * 1e-9);
	if (currentSample) {
		s->parent = currentSample;
		currentSample->children.push_back(*s);
	} else {
		frames[curFrame % frameCount].samples.push_back(*s);
	}
	currentSample = s;
}
void Profiler::EndSample() {
	ProfilerSample *back = &frames[curFrame % frameCount].samples.back();
	currentSample->endTime = (timer.now() - start).count() * 1e-9;
	currentSample = currentSample->parent;
}

void Profiler::FrameStart() {
	frames[curFrame % frameCount].index = curFrame;
	frames[curFrame % frameCount].samples.clear();
	frames[curFrame % frameCount].startTime = (timer.now() - start).count() * 1e-9;
}
void Profiler::FrameEnd() {
	frames[curFrame % frameCount].endTime = (timer.now() - start).count() * 1e-9;
	currentSample = nullptr;
	curFrame++;
}

void Profiler::Print(char *buffer, int size) {
	sprintf_s(buffer, size, "");
}