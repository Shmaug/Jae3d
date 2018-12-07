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

	JAE_API void BeginSample(LPCSTR name);
	JAE_API void EndSample();
	JAE_API void FrameStart();
	JAE_API void FrameEnd();

	JAE_API void PrintLastFrame(wchar_t *buffer, int size);
};

