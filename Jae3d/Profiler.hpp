#pragma once

#include "Util.hpp"

#include <vector>

namespace Profiler {
	class ProfilerSample {
	public:
		std::vector<ProfilerSample*> children;
		ProfilerSample *parent;
		LPCSTR name;
		double startTime;
		double endTime;

		ProfilerSample(LPCSTR name, double startTime) : name(name), startTime(startTime) {
			parent = nullptr;
		}

		~ProfilerSample();
	};

	struct ProfilerFrame {
		std::vector<ProfilerSample*> samples;
		long index;
		double startTime;
		double endTime;
	};

	void BeginSample(LPCSTR name);
	void EndSample();
	void FrameStart();
	void FrameEnd();

	void PrintLastFrame(char *buffer, int size);
};

